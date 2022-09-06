
// #define JACK_ASYNCHRONIZE_BROADCAST_EVENT_ALL_WOKE_BUSY_WAIT

#include "jack/asynchronize.hpp"
#include "ex_helpers.hpp"

int main()
{
    jack::broadcast_event event;

    std::cout << sizeof(jack::broadcast_event) << '\n';

    std::thread t0 {[&] {
        log("0 waiting");
        event.wait();
        log("0 woke");
        log("0 waiting again");
        event.wait();
        log("0 woke");
    }};

    std::thread t1 {[&] {
        log("1 waiting");
        event.wait();
        log("1 woke");
        sleep_for(500ms);
        log("1 waiting again");
        event.wait();
        log("1 woke");
    }};

    std::thread t2 {[&] {
        log("2 waiting");
        event.wait();
        log("2 woke");
    }};

    sleep_for(1s);
    log("calling set()");
    event.set();
    sleep_for(1s);
    log("calling set()");
    event.set();

    t0.join();
    t1.join();
    t2.join();
}