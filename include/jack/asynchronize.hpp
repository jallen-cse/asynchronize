
#include <mutex>
#include <chrono>
#include <condition_variable>

// #include "tl/optional.hpp"

namespace jack
{

/**
 * @brief Wake a single thread per set() call
 */
class UnicastEvent
{
  public:

    /**
     * @brief Block the calling thread until another thread calls set().
     */
    inline void wait()
    {
        std::unique_lock<std::mutex> lock {m_mtx};
        if (!m_set) {
            m_cv.wait(lock, [&]{
                return m_set;
            });
        }
        m_set = false;
    }

    /**
     * @brief Block the calling thread until another thread 
     * calls set() OR until the duration is expired.
     * 
     * @param duration maximum duration thread should wait
     * @return true if set() was called; false if duration expired
     */
    template <typename rep, typename period>
    inline bool wait_for(std::chrono::duration<rep, period> duration)
    {
        std::unique_lock<std::mutex> lock {m_mtx};
        bool was_set {true};
        if (!m_set) {
            was_set = m_cv.wait_for(lock, duration, [&]{
                return m_set;
            });
        }
        m_set = false;
        return was_set;
    }

    /**
     * @brief Block the calling thread until another thread
     * calls set() OR until the timeout is reached.
     * 
     * @param timeout maximum time point at which thread should stop waiting
     * @return true if set() was called; false if timeout was reached
     */
    template <typename clock, typename duration>
    inline bool wait_until(std::chrono::time_point<clock, duration> timeout)
    {
        std::unique_lock<std::mutex> lock {m_mtx};
        bool was_set {true};
        if (!m_set) {
            was_set = m_cv.wait_until(lock, timeout, [&]{
                return m_set;
            });
        }
        m_set = false;
        return was_set;
    }

    /**
     * @brief Set the internal flag & wake 
     * a waiting thread.
     */
    inline void set()
    {
        m_mtx.lock();
        m_set = true;
        m_mtx.unlock();
        m_cv.notify_one();
    }

    /**
     * @brief Check if the internal flag has been set.
     */
    inline bool is_set()
    {
        std::lock_guard<std::mutex> lock {m_mtx};
        return m_set;
    }



  private:
    bool m_set {false};
    std::mutex m_mtx;
    std::condition_variable m_cv;
};

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
    return static_cast<Signal>(static_cast<int>(some) |
                               static_cast<int>(other));
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