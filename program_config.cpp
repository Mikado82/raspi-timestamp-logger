#include "program_config.hpp"

#include <iostream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <boost/algorithm/string.hpp>

#include <wiringPi.h>

using namespace std;

void program_config::read_from_ini(std::string filename)
{
    // Read the INI file
    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini(filename, pt);
    
    // Read pin and edge
    pin = pt.get<int>("GPIO.Pin");
    std::string edge_str = pt.get<string>("GPIO.Edge");
    block_trigger_ms = pt.get<uint64_t>("GPIO.BlockTriggerMillis", block_trigger_ms);
    show_blocked_triggers = pt.get<bool>("GPIO.ShowBlocked", show_blocked_triggers);

    // Convert edge from string to numeric value
    boost::algorithm::to_lower(edge_str);
    if(edge_str == "rising") edge = INT_EDGE_RISING;
    else if(edge_str == "falling") edge = INT_EDGE_FALLING;
    else throw runtime_error("'GPIO.Edge' must be either 'rising' or 'falling'.");

    // Read the output file name; provide default values
    output_file = pt.get<string>("Output.File", output_file);
    write_to_db = pt.get<bool>("Output.Database", write_to_db);
    write_to_file = !output_file.empty();

    // Read the DB parameters
    if(write_to_db)
    {
        // Read hostname and port
        string hostname = pt.get<string>("DB.Hostname");
        unsigned short port = pt.get<unsigned short>("DB.Port", 0);
        
        if(port > 0)
             db_params.server_address.emplace_host_and_port(hostname, port);
        else db_params.server_address.emplace_host_and_port(hostname);
        
        // Read username and password
        db_params.username = pt.get<string>("DB.Username");
        db_params.password = pt.get<string>("DB.Password");
        
        // Read database and table name
        db_params.database = pt.get<string>("DB.Database");
        db_table_name = pt.get<string>("DB.Table");
    }
}

void program_config::print()
{
    // GPIO info
    cout << "GPIO config: detecting " << (edge == INT_EDGE_RISING ? "RISING" : "FALLING")
        << " edge on pin " << pin << " (WiringPi pin scheme).\n";
    cout << "Blocking triggers within " << block_trigger_ms << " ms since last trigger.\n";
    if(show_blocked_triggers) cout << "Showing blocked triggers.\n";

    // Output info
    if(write_to_file) cout << "Writing timestamps to file '" << output_file << "'.\n";
    if(write_to_db) cout << "Writing timestamps to this database:\n";

    // Database info
    if(write_to_db)
    {
        cout << "   Host: " << db_params.server_address.hostname() << "\n";
        cout << "   Port: " << db_params.server_address.port() << "\n";
        cout << "   Name: " << db_params.database << "\n";
        cout << "   User: " << db_params.username << "\n";
        cout << "   Table: " << db_table_name << "\n";
    }
}
