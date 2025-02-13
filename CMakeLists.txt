cmake_minimum_required(VERSION 3.5.0)
project(starttime VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(starttime main.cpp)

# Finde die wiringPi Bibliothek
find_library(WIRINGPI_LIBRARIES NAMES wiringPi)
find_path(WIRINGPI_INCLUDE_DIRS NAMES wiringPi.h)

# Falls nicht gefunden, Abbruch mit Fehlermeldung
if (NOT WIRINGPI_LIBRARIES)
    message(FATAL_ERROR "wiringPi library not found!")
endif()

if (NOT WIRINGPI_INCLUDE_DIRS)
    message(WARNING "wiringPi headers not found! Compilation might fail.")
endif()

# Verknüpfen der Bibliothek
target_include_directories(starttime PRIVATE ${WIRINGPI_INCLUDE_DIRS})
target_link_libraries(starttime PRIVATE ${WIRINGPI_LIBRARIES})
