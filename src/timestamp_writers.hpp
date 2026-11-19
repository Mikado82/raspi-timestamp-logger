#ifndef TIMESTAMP_WRITERS_INCLUDED
#define TIMESTAMP_WRITERS_INCLUDED

#include <string>
#include <fstream>
#include <exception>

#include <boost/mysql/connect_params.hpp>
#include <boost/mysql/any_connection.hpp>

#include <boost/asio/io_context.hpp>

class file_writer
{
public:
    // Construction
    file_writer(const std::string &filename);

    // Writing
    void operator()(const std::string &timestamp);

    // Static error handler
    static void handle_exception(std::exception_ptr ex_ptr);

private:
    // The output file stream
    std::ofstream file;
};

class db_writer
{
public:
    // Construction
    db_writer(
        const boost::mysql::connect_params &params,
        const std::string_view table_name);

    // Writing
    void operator()(const std::string &timestamp);

    // Static error handler
    static void handle_exception(std::exception_ptr ex_ptr);

private:
    // The execution context, required to run I/O operations
    boost::asio::io_context ctx;

    // The connection to the MySQL server
    boost::mysql::any_connection conn;

    // The name of the database table to insert into
    std::string table_name;
};

#endif
