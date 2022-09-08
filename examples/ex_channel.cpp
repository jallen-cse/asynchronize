
#include <map>
#include <thread>
#include <chrono>

#include "jack/asynchronize.hpp"

using std::thread;
using std::this_thread::sleep_for;
using namespace std::chrono_literals;
using namespace jack;

std::map<signal, const char*> repr {
    {signal::NONE, "NONE"},
    {signal::SIG0, "SIG0"},
    {signal::SIG1, "SIG1"},
    {signal::SIG2, "SIG2"},
    {signal::SIG3, "SIG3"},
    {signal::SIG4, "SIG4"},
    {signal::SIG5, "SIG5"},
    {signal::SIG6, "SIG6"},
    {signal::SIG7, "SIG7"},
    {signal::SIG8, "SIG8"},
    {signal::SIG9, "SIG9"},
    {signal::SIG10, "SIG10"},
    {signal::SIG11, "SIG11"},
    {signal::SIG12, "SIG12"},
    {signal::SIG13, "SIG13"},
    {signal::SIG14, "SIG14"},
    {signal::SIG15, "SIG15"},
    {signal::ANY, "ANY"},
};

int main()
{
    // new channel with ground state NONE (0)
    channel<signal> channel(signal::NONE);

    std::thread waiter {[&] {
        signal sig = channel.wait(signal::ANY);
        printf("Got signal %s\n", repr[sig]);

        sig = channel.wait(signal::SIG1 | signal::SIG4);
        printf("Got signal %s\n", repr[sig]);

        sig = channel.wait(signal::SIG1 | signal::SIG4);
        printf("Got signal %s\n", repr[sig]);
    }};

    sleep_for(500ms);
    printf("Sending SIG11\n");
    channel.signal(signal::SIG11);

    sleep_for(500ms);
    printf("Sending SIG4\n");
    channel.signal(signal::SIG4);
    
    sleep_for(500ms);
    printf("Sending SIG3\n");
    channel.signal(signal::SIG3);

    sleep_for(500ms);
    printf("Sending SIG1\n");
    channel.signal(signal::SIG1);

    waiter.join();
}