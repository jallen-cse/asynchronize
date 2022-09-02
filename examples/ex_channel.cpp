
#include <map>
#include <thread>
#include <chrono>

#include "jack/asynchronize.hpp"

using std::thread;
using std::this_thread::sleep_for;
using namespace std::chrono_literals;
using namespace jack;

std::map<Signal, const char*> repr {
    {NONE, "NONE"},
    {SIG0, "SIG0"},
    {SIG1, "SIG1"},
    {SIG2, "SIG2"},
    {SIG3, "SIG3"},
    {SIG4, "SIG4"},
    {SIG5, "SIG5"},
    {SIG6, "SIG6"},
    {SIG7, "SIG7"},
    {SIG8, "SIG8"},
    {SIG9, "SIG9"},
    {SIG10, "SIG10"},
    {SIG11, "SIG11"},
    {SIG12, "SIG12"},
    {SIG13, "SIG13"},
    {SIG14, "SIG14"},
    {SIG15, "SIG15"},
    {ANY, "ANY"},
};

int main()
{

    // new channel with ground state NONE (0)
    Channel<Signal> channel(Signal::NONE);

    std::thread waiter {[&] {
        Signal sig = channel.wait(Signal::ANY);
        printf("Got signal %s\n", repr[sig]);

        sig = channel.wait(Signal::SIG1 | Signal::SIG4);
        printf("Got signal %s\n", repr[sig]);

        sig = channel.wait(Signal::SIG1 | Signal::SIG4);
        printf("Got signal %s\n", repr[sig]);
    }};

    sleep_for(500ms);
    printf("Sending SIG11\n");
    channel.signal(Signal::SIG11);

    sleep_for(500ms);
    printf("Sending SIG4\n");
    channel.signal(Signal::SIG4);
    
    sleep_for(500ms);
    printf("Sending SIG3\n");
    channel.signal(Signal::SIG3);

    sleep_for(500ms);
    printf("Sending SIG1\n");
    channel.signal(Signal::SIG1);

    waiter.join();
}