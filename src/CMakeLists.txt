cmake_minimum_required(VERSION 3.5.0)
project(timestamp-logger VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(timestamp-logger main.cpp application.cpp program_config.cpp timestamp_writers.cpp boost_mysql.cpp)

# wiringPi (via cloned repo & build DEB package)
find_library(WIRINGPI_LIBRARIES NAMES wiringPi)
find_path(WIRINGPI_INCLUDE_DIRS NAMES wiringPi.h)

target_include_directories(timestamp-logger PRIVATE ${WIRINGPI_INCLUDE_DIRS})
target_link_libraries(timestamp-logger PRIVATE ${WIRINGPI_LIBRARIES})

# Boost (via download & build), Threads, OpenSSL (via libssl-dev)
find_package(Boost REQUIRED COMPONENTS charconv)
find_package(OpenSSL REQUIRED)

target_link_libraries(timestamp-logger PRIVATE Boost::charconv  OpenSSL::Crypto OpenSSL::SSL)

# We need to define BOOST_MYSQL_SEPARATE_COMPILATION in any code using Boost.MySQL in separate-build mode
target_compile_definitions(timestamp-logger PRIVATE BOOST_MYSQL_SEPARATE_COMPILATION)
