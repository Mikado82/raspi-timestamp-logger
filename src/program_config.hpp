#ifndef PROGRAM_CONFIG_INCLUDED
#define PROGRAM_CONFIG_INCLUDED

#include <string>

#include <boost/mysql/connect_params.hpp>

class program_config
{
public:
    program_config() = default;
    ~program_config() = default;

    // Read configuration form an INI file
    void read_from_ini(std::string filename);
    
    // Display configuration
    void print();

public:
    // GPIO setup
    int pin;
    int edge;
    int64_t block_trigger_ms = 1000;
    bool show_blocked_triggers = false;
    
    // Output parameters
    bool write_to_file = false;
    bool write_to_db = false;
    std::string output_file;
    std::string test_string;
    
    // Database connection info
    boost::mysql::connect_params db_params;
    boost::mysql::connect_params db_params.auth_plugin_name = "mysql_native_password";
    std::string db_table_name;
};

#endif