#include <chrono>
#include <iomanip>
#include <iostream>

using namespace std;

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

int main(int, char**){
    #ifndef NDEBUG
    cout << "DEBUG VERSION\n";
    #endif

    // Get current timestamp and print it
    string timestamp = get_timestamp();
    cout << timestamp << "\n";

    return 0;
}
