
#include <mutex>
#include <chrono>
#include <condition_variable>

// #include "tl/optional.hpp"

namespace jack
{

// /**
//  * @brief Wake a single thread per set() call
//  */
// class UnicastEvent;

// /**
//  * @brief Wake all threads until reset() is called
//  */
// class BroadcastEvent;

// /**
//  * @brief Generic event
//  * 
//  * @tparam condition must evaluate to true for set() to wake a thread
//  */
// template <typename condition>
// class Event;

enum Signal
{
    NONE = 0,
    SIG0 = 1,
    SIG1 = 2,
    SIG2 = 4,
    SIG3 = 8,
    SIG4 = 16,
    SIG5 = 32,
    SIG6 = 64,
    SIG7 = 128,
    SIG8 = 256,
    SIG9 = 512,
    SIG10 = 1024,
    SIG11 = 2048,
    SIG12 = 4096,
    SIG13 = 8192,
    SIG14 = 16384,
    SIG15 = 32768,
    ANY = 65535
};

constexpr inline Signal operator|(Signal some, Signal other)
{
    return static_cast<Signal>(
        static_cast<int>(some) | static_cast<int>(other));
}

/**
 * @brief Many-to-one synchronization object. Enable a consumer to
 * wait for different signal types and handle each accordingly.
 */
template <typename SigTp>
class Channel
{
  public:
    explicit Channel(SigTp initial) : m_sig(initial), m_init_state(initial)
    {
    }

    inline SigTp wait(const SigTp& sigs)
    {
        std::unique_lock<std::mutex> lock {m_mtx};
        if (!(m_sig & sigs))
        {
            m_cv.wait(lock, [&] {
                return m_sig & sigs;
            });
        }
        return m_internal_reset();
    }

    template <typename Rep, typename Period>
    inline SigTp wait_for(const SigTp sigs,
            std::chrono::duration<Rep, Period> duration)
    {
        std::unique_lock<std::mutex> lock {m_mtx};
        if (!(m_sig & sigs))
        {
            return m_cv.wait_for(lock, duration, [&] {
                return m_sig & sigs;
            }) ? m_internal_reset() : m_init_state;
        }
        else
        {
            return m_internal_reset();
        }
    }

    template <typename Clock, typename Duration>
    inline SigTp wait_until(const SigTp sigs,
            std::chrono::time_point<Clock, Duration> timeout)
    {
        std::unique_lock<std::mutex> lock {m_mtx};
        if (!(m_sig & sigs))
        {
            return m_cv.wait_until(lock, timeout, [&] {
                return m_sig & sigs;
            }) ? m_internal_reset() : m_init_state;
        }
        else
        {
            return m_internal_reset();
        }
    }

    inline void signal(const SigTp sig)
    {   
        {
            std::lock_guard<std::mutex> lock {m_mtx};
            m_sig = sig;
        }
        m_cv.notify_one();
    }

  private:

    inline SigTp m_internal_reset()
    {
        SigTp holder = m_sig;
        m_sig = m_init_state;
        return holder;
    }

    SigTp m_sig;
    SigTp m_init_state;
    std::mutex m_mtx;
    std::condition_variable m_cv;
};

template <typename SigTp>
class MuxChannel;

}