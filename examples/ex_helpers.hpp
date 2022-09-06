#include <thread>
#include <chrono>
#include <sstream>
#include <iostream>

using std::this_thread::sleep_for;
using namespace std::chrono_literals;

std::string timepoint_string()
{
    std::stringstream tpss;
    tpss.precision(8);
    auto tp = std::chrono::steady_clock::now()
            .time_since_epoch().count();
    tpss << '[' << (double)tp / 1000000000.0 << ']';
    return tpss.str();
}

void log(const char* msg)
{
    static std::mutex cout_mtx;
    std::lock_guard<std::mutex> guard {cout_mtx};
    std::cout << timepoint_string() << ' ' << msg << '\n';
}