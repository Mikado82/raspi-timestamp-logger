#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include <fstream>

#include <wiringPi.h>

using namespace std;

// Global variables
ofstream file;
chrono::system_clock::time_point last_trigger;

string get_timestamp()
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

void buttonPushHandler()
{
    // Calculate elapsed time
    auto now = chrono::high_resolution_clock::now();
    auto elapsed = now - last_trigger;

    // Check if at least one second elapsed since last trigger
    if(elapsed > chrono::milliseconds(1000))
    {
        // Get current timestamp
        string timestamp = get_timestamp();
        
        // Print to console and write to file
        cout << timestamp << "\n";
        file << timestamp << "\n";
        file.flush();

        // Update last trigger time
        last_trigger = now;
    }
    // Only in debug builds
    #ifndef NDEBUG
    else
    {
        // Output info that trigger was blocked
        cout << "BLOCKED!\n";
    }
    #endif
}

int main(int, char**)
{
    // Parameters
    constexpr int PIN = 5;
    constexpr int EDGE = INT_EDGE_RISING; //INT_EDGE_FALLING
    const string filename = "starttimes.txt";

    // Print info if this is a debug build
    #ifndef NDEBUG
    cout << "DEBUG BUILD\n";
    #endif
    
    // Open output file, always append
    file.open(filename, ios::out | ios::app);
    if(!file.is_open())
    {
        cerr << "Could not open output file '" << filename << "'.\nExiting.\n";
        return 1;
    }

    // Initialize wiringPi library
    wiringPiSetup();

    // Setup pin to input, pull up/down mode to off
    pinMode(PIN, INPUT);
    pullUpDnControl(PIN, PUD_OFF);

    // Bind to interrupt to our handler function
	wiringPiISR(PIN, EDGE, &buttonPushHandler);

    // Wait for interrupts
    while(true)
    {
        // Just sleep
        this_thread::sleep_for(chrono::milliseconds(1000));
    }

    return 0;
}
