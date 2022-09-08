#include <thread>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <iostream>

using std::this_thread::sleep_for;
using namespace std::chrono_literals;

static std::mutex cout_mtx;
static std::chrono::steady_clock::time_point genesis {
        std::chrono::steady_clock::now()};

template <typename last_t>
inline void variadic_strcat(std::stringstream& stream,
        const last_t last)
{
    stream << last;
}

template <typename current_t, typename next_t, typename... remainder_t>
inline void variadic_strcat(std::stringstream& stream, 
        const current_t current, const next_t next, 
        const remainder_t... remainder)
{
    stream << current;
    variadic_strcat(stream, next, remainder...);
}

template <typename... str_args>
inline std::string make_str(const str_args... args)
{
    std::stringstream ss;
    variadic_strcat(ss, args...);
    return ss.str();
}

std::string timepoint_string()
{
    std::stringstream tpss;
    tpss << '[' << std::setfill('0') << std::setw(5) <<
            (std::chrono::steady_clock::now() -
            genesis).count() / 1000000 << ']';
    return tpss.str();
}

template <typename... str_args>
void log(const str_args... va_args)
{
    std::lock_guard<std::mutex> guard {cout_mtx};
    std::cout << timepoint_string() << ' ' << make_str(va_args...) << '\n';
}