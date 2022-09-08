
#include <map>
#include <thread>
#include <chrono>

#include "jack/asynchronize.hpp"

#include "ex_helpers.hpp"

std::map<jack::signal, const char*> repr {
    {jack::signal::NONE, "NONE"},
    {jack::signal::SIG0, "SIG0"},
    {jack::signal::SIG1, "SIG1"},
    {jack::signal::SIG2, "SIG2"},
    {jack::signal::SIG3, "SIG3"},
    {jack::signal::SIG4, "SIG4"},
    {jack::signal::SIG5, "SIG5"},
    {jack::signal::SIG6, "SIG6"},
    {jack::signal::SIG7, "SIG7"},
    {jack::signal::SIG8, "SIG8"},
    {jack::signal::SIG9, "SIG9"},
    {jack::signal::SIG10, "SIG10"},
    {jack::signal::SIG11, "SIG11"},
    {jack::signal::SIG12, "SIG12"},
    {jack::signal::SIG13, "SIG13"},
    {jack::signal::SIG14, "SIG14"},
    {jack::signal::SIG15, "SIG15"},
    {jack::signal::ANY, "ANY"},
};

int main()
{
    std::cout << "sizeof(jack::channel<jack::signal>) == "
              << sizeof(jack::channel<jack::signal>) << '\n';

    // new channel with ground state NONE (0)
    jack::channel<jack::signal> channel(jack::signal::NONE);

    std::thread waiter {[&] {
        jack::signal sig = channel.wait(jack::signal::ANY);
        log("Got signal ", repr[sig]);

        sig = channel.wait(jack::signal::SIG1 | jack::signal::SIG4);
        log("Got signal ", repr[sig]);

        sig = channel.wait(jack::signal::SIG1 | jack::signal::SIG4);
        log("Got signal ", repr[sig]);
    }};

    sleep_for(500ms);
    log("Sending SIG11");
    channel.signal(jack::signal::SIG11);

    sleep_for(500ms);
    log("Sending SIG4");
    channel.signal(jack::signal::SIG4);
    
    sleep_for(500ms);
    log("Sending SIG3");
    channel.signal(jack::signal::SIG3);

    sleep_for(500ms);
    log("Sending SIG1");
    channel.signal(jack::signal::SIG1);

    waiter.join();
}