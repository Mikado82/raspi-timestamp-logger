#include <iostream>

#include "application.hpp"
#include "timestamp_writers.hpp"

using namespace std;

int main(int argc, char** argv)
{
    // If a parameter is specified, use it as
    // the INI file; otherwise, use default.
    string ini_file;
    if(argc > 1) ini_file = argv[1];
    else ini_file = "config.ini";

    // Get the singleton instance of our application
    Application &app = Application::get_instance();

    // Used for exception handling
    exception_ptr ex_ptr;

    try
    {
        // Run the application
        app.run(ini_file);
    }    
    catch(...)
    {
        // Capture the current exception
        ex_ptr = current_exception();
    }

    try
    {
        // See if the file writer handles this exception
        file_writer::handle_exception(ex_ptr);
        
        // Return 1 to indicate an error with the file output
        return 1;
    }
    catch(...)
    {
        // If not, capture the exception again
        ex_ptr = current_exception();
    }

    try
    {
        // See if the database writer handles this exception
        db_writer::handle_exception(ex_ptr);

        // Return 2 to indicate an error with the database output
        return 2;
    }
    catch(...)
    {
        // If not, capture the exception again
        ex_ptr = current_exception();
    }

    try
    {
        // Rethrow to address remaining exceptions
        if(ex_ptr) rethrow_exception(ex_ptr);
    }
    // Catch all standard library exceptions
    catch(const exception& e)
    {
        // Display error message
        cerr << "ERROR: " << e.what() << "\n";
        
        // Return 3 to indicate any other error
        return 3;
    }

    // Success
    return 0;
}
