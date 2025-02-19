#ifndef APPLICATION_INCLUDED
#define APPLICATION_INCLUDED

#include <string>
#include <chrono>
#include <functional>
#include <condition_variable>

#include "program_config.hpp"
#include "synced_queue.hpp"

// Singleton pattern
class Application
{
private:
    // Private constructor/destructor to disallow regular construction
    Application() = default;
    ~Application() = default;

public:
    // Get the static instance
    static Application& get_instance()
    {
        static Application instance;
        return instance;
    }

    // Main program
    bool run(std::string ini_file);

private:
    // Holding the configuration
    program_config config;

    // Synced queues to pass timestamps to the writer threads
    synced_queue<std::string> file_writer_queue;
    synced_queue<std::string> db_writer_queue;
    
    // Time of the last trigger
    std::chrono::system_clock::time_point last_trigger;

    // The main writer thread function
    template<typename TimestampWriter>
    static void writer_thread(
        const std::string_view name,
        synced_queue<std::string> &input_queue,
        std::function<TimestampWriter()> create_writer
    );

    // A wrapper around the writer thread function to
    // catch any exception thrown in the thread function
    template<typename TimestampWriter, typename... Args>
    static void writer_thread_wrapper(
        const std::string_view name,
        Args&&... args
    );
    
    // Function for writing timestamps to a file
    static void write_timestamps_to_file(
        std::queue<std::string> &timestamps,
        std::string filename);

    // Function for writing timestamps to a database
    static void write_timestamps_to_db(
        std::queue<std::string> &timestamps,
        boost::mysql::connect_params db_params,
        std::string db_table_name);

    // Get a nicely formatted timestamp
    static std::string get_timestamp();

private:
    // Linux signal handling
    void setup_signal_handlers();
    static void sig_int_term_handler(int signal);

    // Shutdown flag and condition variable
    bool shutdown_flag = false;
    std::mutex shutdown_mtx;
    std::condition_variable shutdown_cv;

private:
    // GPIO port handling
    void setup_gpio();
    static void gpio_button_handler();
};

#endif
