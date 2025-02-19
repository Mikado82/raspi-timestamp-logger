#include "application.hpp"
#include "timestamp_writers.hpp"

#include <signal.h>
#include <cstdlib>

#include <iomanip>
#include <iostream>
#include <chrono>
#include <exception>

#include <wiringPi.h>

using namespace std;

void Application::setup_signal_handlers()
{
    struct sigaction sig_action;

    sig_action.sa_handler = Application::sig_int_term_handler;
    sigemptyset(&sig_action.sa_mask);
    sig_action.sa_flags = 0;

    sigaction(SIGINT, &sig_action, NULL);
    sigaction(SIGTERM, &sig_action, NULL);
}

void Application::sig_int_term_handler(int signal)
{
    // Get singleton instance
    Application &app = Application::get_instance();

    // Display info    
    cout << "\nCaught signal " << signal << "\n";

    // Lock the mutex
    unique_lock guard(app.shutdown_mtx);

    // Initiate a graceful shutdown
    if(app.shutdown_flag == false)
    {
        // Dissplay message
        cout << "Graceful shutdown initiated\n";

        // Set shutdown flag and signal the run() method
        app.shutdown_flag = true;
        app.shutdown_cv.notify_all();
    }
    else
    {
        // If the flag is already set, make a hard exit
        cout << "Exiting ungracefully\n";
        exit(signal);
    }
}

void Application::setup_gpio()
{
    // Initialize wiringPi library
    wiringPiSetup();

    // Setup pin to input, pull up/down mode to off
    pinMode(config.pin, INPUT);
    pullUpDnControl(config.pin, PUD_OFF);

    // Bind to interrupt to our handler function
	wiringPiISR(config.pin, config.edge, &Application::gpio_button_handler);
}

void Application::gpio_button_handler()
{
    // Get singleton instance
    Application &app = Application::get_instance();
    
    // Calculate elapsed time
    auto now = chrono::high_resolution_clock::now();
    auto elapsed = now - app.last_trigger;

    // Check if the minimum pause elapsed since last trigger
    if(elapsed > chrono::milliseconds(app.config.block_trigger_ms))
    {
        // Get current timestamp
        string timestamp = get_timestamp();
        
        // Output notification and timestamp
        cout << "\nTRIGGERED: " << timestamp << "\n";
        
        // Push the timestamp to the file writer queue
        if(app.config.write_to_file)
            app.file_writer_queue.push(timestamp);
        
        // Push the timestamp to the database writer queue
        if(app.config.write_to_db)
            app.db_writer_queue.push(timestamp);

        // Update last trigger time
        app.last_trigger = now;
    }
    // Show blocked triggers if requested
    else if(app.config.show_blocked_triggers)
    {
        // Output info that trigger was blocked
        auto elapsed_ms = chrono::duration_cast<chrono::milliseconds>(elapsed).count();
        cout << "\nTrigger BLOCKED (" << elapsed_ms << " ms since last trigger)\n";
    }
}

string Application::get_timestamp()
{
    // Based on:
    // https://gist.github.com/dtoma/cc3d5b2dd4c03a25886adededcc1e3b9

    auto now = chrono::high_resolution_clock::now();
    auto time = chrono::system_clock::to_time_t(now);
    auto tm = *localtime(&time);

    auto epoch = now.time_since_epoch();
    auto us = chrono::duration_cast<chrono::microseconds>(epoch).count() % 1000000;

    stringstream sstr;
    sstr << put_time(&tm, "%F %T.") << us << put_time(&tm, " %Z");

    return sstr.str();
}

template<typename TimestampWriter>
void Application::writer_thread(
    const string_view name,
    synced_queue<string> &input_queue,
    function<TimestampWriter()> create_writer)
{   
    // Local queue of timestamps retrieved from
    // the input queue, but not yet written
    queue<string> unwritten_timestamps;

    bool is_retrying = false;
    bool done = false;

    do
    {
        // If we are not retrying, wait for the handler to push a
        // timestamp to the input queue or for the signal that we are done
        if(is_retrying == false && input_queue.wait_for_items())
        {
            // Set flag
            done = true;
        }

        // Pop all timestamps from the interrupt queue and
        // add them to the local queue of unwritten timestamps
        string timestamp;
        while(input_queue.pop(timestamp))
            unwritten_timestamps.push(timestamp);

        // If we are re-trying:
        if(is_retrying)
        {
            // Display info
            cout << "Re-trying to write to the " << name << " in 10 sec... ";
            cout << "(unwritten timestamps: " << unwritten_timestamps.size() << ")\n";
            
            // Wait five seconds
            this_thread::sleep_for(chrono::seconds(10));

            // Check if we are done
            if(input_queue.is_done())
            {
                // Set flag
                done = true;
            }
        }
    
        // Timestamp writing
        exception_ptr ex_ptr;
        try
        {
            // Create a writer object
            TimestampWriter writer = create_writer();

            // Get each timestamp from the unwritten queue
            while(!unwritten_timestamps.empty())
            {
                // Get the front timestamp
                string timestamp = unwritten_timestamps.front();

                // Write the timestamp
                writer(timestamp);

                // Only now remove it from the queue
                unwritten_timestamps.pop();
            }
        }
        catch(...)
        {
            // Capture the current exception
            ex_ptr = current_exception(); 
        }

        // If there was an exception, try to handle it
        // via call the writer's exception handler function
        if(ex_ptr) TimestampWriter::handle_exception(ex_ptr);
        
        // Check if everything was written and
        // set the retrying flag accordingly
        is_retrying = !unwritten_timestamps.empty();
    }
    // Repeat until we are done
    while(done == false);

    // Report any unwritten timestamps
    if(!unwritten_timestamps.empty())
    {
        cout << "\n---------------------------------------------------\n";
        cout << "Timestamps NOT written to the " << name << ":\n";
        
        while(!unwritten_timestamps.empty())
        {
            string timestamp = unwritten_timestamps.front();
            cout << timestamp << "\n";
            unwritten_timestamps.pop();
        }
        
        cout << "---------------------------------------------------\n\n";
    }
}

template<typename TimestampWriter, typename... Args>
void Application::writer_thread_wrapper(
    const string_view name, Args&&... args)
{
    // Compile-time check if the writer is a suitable callable
    static_assert(is_invocable_v<TimestampWriter, const string&>,
        "A timestamp writer must be callable with a 'const string&' parameter.");

    // Compile-time check if the writer's handle_exception function has a correct signature
    static_assert(is_same_v<decltype(TimestampWriter::handle_exception), void(exception_ptr)>,
        "A timestamp writer's must be have a static member function 'void handle_exception(std::exception_ptr)'.");

    try
    {
        // Call the actual thread function, forwarding arguments
        Application::writer_thread<TimestampWriter>(name, forward<Args>(args)...);
    }
    // Catch anything that has not been caught inside the task
    catch(const std::exception& err)
    {
        // Display the error and a note that this exception is not recoverable
        cerr << "\n-----------------------------------------------------------------\n";
        cerr << "FATAL error: " << err.what() << "\n\n";
        cerr << "This is an uncaught exception in the " << name << " writer thread.\n";
        cerr << "The program will keep running, but the thread will stop working.\n";
        cerr << "Therefore, no more timestamps are written to the " << name << ".\n";
        cerr << "-----------------------------------------------------------------\n\n";
    }
}

bool Application::run(string ini_file)
{
    // Print info if this is a debug build
    #ifndef NDEBUG
    cout << "DEBUG BUILD\n";
    #endif

    // Set up handlers for the SIGINT and SIGTERM signals
    setup_signal_handlers();
    
    // Read configuration from the INI file and print it
    config.read_from_ini(ini_file);
    config.print();    
    
    // Test opening the output file
    if(config.write_to_file)
    {
        cout << "Testing file output...\n";
        
        // Try creating a file writer opject
        file_writer(config.output_file);
    }

    // Test the connection to the DB server
    if(config.write_to_db)
    {
        cout << "Testing database connection...\n";

        // Try create a database writer opject
        db_writer(config.db_params, config.db_table_name);
    }

    // Setup the GPIO ports
    setup_gpio();

    // Start the file writer thread
    thread file_writer_thread;
    if(config.write_to_file)
    {
        // Lambda for creating a file writer
        auto create_file_writer = [this]() -> file_writer {
            return file_writer(config.output_file); };

        // Create the thread
        file_writer_thread = thread([this, create_file_writer]() {
            writer_thread_wrapper<file_writer>("file", file_writer_queue, create_file_writer);
        });
    }

    // Start the database writer thread
    thread db_writer_thread;
    if(config.write_to_db)
    {
        // Lambda for creating a database writer
        auto create_db_writer = [this]() -> db_writer {
            return db_writer(config.db_params, config.db_table_name); };

        // Create the thread
        db_writer_thread = thread([this, create_db_writer]() {
            writer_thread_wrapper<db_writer>("database", db_writer_queue, create_db_writer);
        });
    }

    // Display info that we are ready and how to quit
    cout << "Listening for GPIO triggers, press Ctrl+C to quit.\n";

    // Block and wait for graceful shutdown signal
    {
        unique_lock guard(shutdown_mtx);
        shutdown_cv.wait(guard, [&]() { return shutdown_flag; });
    }

    // Shutdown file writer thread
    if(config.write_to_file)
    {
        // Notify the handler queue that we are done
        file_writer_queue.done();
    
        // Wait for the file writer thread to join
        cout << "Waiting for file writer thread...\n";
        if(file_writer_thread.joinable())
            file_writer_thread.join();
    }
    
    // Shutdown database writer thread
    if(config.write_to_db)
    {
        // Notify the handler queue that we are done
        db_writer_queue.done();
    
        // Wait for the database writer thread to join
        cout << "Waiting for database writer thread...\n";
        if(db_writer_thread.joinable())
            db_writer_thread.join();
    }

    return true;
}
