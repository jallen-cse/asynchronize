
#include <chrono>
#include <mutex>
#include <atomic>
#include <shared_mutex>
#include <condition_variable>
#include <thread>

namespace jack
{

class event
{
    using mtx_lg = std::lock_guard<std::mutex>;
    using mtx_ul = std::unique_lock<std::mutex>;
  public:
    virtual void wait() = 0;
    virtual void wait_for() = 0;
    virtual void wait_until() = 0;
    virtual void set() = 0;
};

/**
 * @brief A thread synchronization object that garauntees one
 * thread is woken per set() call.  This means that if no
 * threads are waiting when set() is called, the next wait will
 * return immediately.
 * 
 * @note Calling set() multiple times between waiters will have 
 * no additional effect.
 */
class sticky_event
{
    using mtx_lg = std::lock_guard<std::mutex>;
    using mtx_ul = std::unique_lock<std::mutex>;
  public:
    
    /**
     * @brief Block the calling thread until another thread
     * has called set.  If multiple threads are waiting, 
     * it is unspecified which will be woken.
     */
    void wait()
    {
        mtx_ul lock {m_mtx};
        if (!m_is_set)
        {
            m_cv.wait(lock, [this] {
                return m_is_set;
            });
        }
        m_is_set = false;
    }

    /**
     * @brief Block the calling thread until another thread 
     * has called set() OR until the duration expires.  If 
     * multiple threads are waiting, it is unspecified 
     * which will be woken.
     * 
     * @param duration maximum duration thread should wait
     * @return true if set() was called; false if duration expired
     */
    template <typename rep, typename period>
    inline bool wait_for(std::chrono::duration<rep, period> duration)
    {
        mtx_ul lock {m_mtx};
        bool was_set {true};
        if (!m_is_set)
        {
            was_set = m_cv.wait_for(lock, duration, [this] {
                return m_is_set;
            });
        }
        m_is_set = false;
        return was_set;
    }

    /**
     * @brief Block the calling thread until another thread
     * has called set() or until the timeout is reached.  If 
     * multiple threads are waiting, it is unspecified 
     * which will be woken.
     * 
     * @param timeout maximum time point at which thread should stop waiting
     * @return true if set() was called; false if timeout was reached
     */
    template <typename clock, typename duration>
    inline bool wait_until(std::chrono::time_point<clock, duration> timeout)
    {
        mtx_ul lock {m_mtx};
        bool was_set {true};
        if (!m_is_set)
        {
            was_set = m_cv.wait_until(lock, timeout, [this] {
                return m_is_set;
            });
        }
        m_is_set = false;
        return was_set;
    }

    /**
     * @brief Set the internal flag and wake at most one waiting
     * thread.  If there are no waiters, the next call to wait
     * will return immediately.  If there are multiple waiters,
     * it is unspecified which will be woken.
     */
    void set()
    {
        m_mtx.lock();
        if (!m_is_set)
        {
            m_is_set = true;
            m_mtx.unlock();
            m_cv.notify_one();
        }
        else
        {
            m_mtx.unlock();
        }
    }

    /**
     * @brief Check if the internal flag is set.
     * 
     * @return true if the flag is set, else false
     */
    bool is_set()
    {
        mtx_lg lock {m_mtx};
        return m_is_set;
    }

    /**
     * @brief Reset the internal flag so future wait
     * calls will block. 
     */
    void reset()
    {
        mtx_lg lock {m_mtx};
        m_is_set = false;
    }

  private:
    bool m_is_set {false};
    std::mutex m_mtx;
    std::condition_variable m_cv;
};

/**
 * @brief A Python Event-like synchronization object.  Set()
 * will set the internal flag and wake all waiting threads. 
 * All future wait calls will return immediately until
 * reset() is called.
 */
class toggle_event
{
    using mtx_lg = std::lock_guard<std::mutex>;
    using mtx_ul = std::unique_lock<std::mutex>;
  public:

    /**
     * @brief Block the calling thread until another
     * thread has called set(). 
     */
    void wait()
    {
        mtx_ul lock {m_mtx};
        if (!m_is_set)
        {
            m_cv.wait(lock, [this] {
                return m_is_set;
            });
        }
    }

    /**
     * @brief Block the calling thread until another thread 
     * has called set() OR until the duration expires.
     * 
     * @param duration maximum duration thread should wait
     * @return true if set() was called; false if duration expired
     */
    template <typename rep, typename period>
    inline bool wait_for(std::chrono::duration<rep, period> duration)
    {
        mtx_ul lock {m_mtx};
        bool was_set {true};
        if (!m_is_set)
        {
            was_set = m_cv.wait_for(lock, duration, [this] {
                return m_is_set;
            });
        }
        return was_set;
    }

    /**
     * @brief Block the calling thread until another thread
     * has called set() or until the timeout is reached.
     * 
     * @param timeout maximum time point at which thread should stop waiting
     * @return true if set() was called; false if timeout was reached
     */
    template <typename clock, typename duration>
    inline bool wait_until(std::chrono::time_point<clock, duration> timeout)
    {
        mtx_ul lock {m_mtx};
        bool was_set {true};
        if (!m_is_set)
        {
            was_set = m_cv.wait_until(lock, timeout, [this] {
                return m_is_set;
            });
        }
        return was_set;
    }

    /**
     * @brief Set the internal flag and wake all
     * waiting threads.  All future calls to wait will
     * return immediately until reset() is called.
     */
    void set()
    {
        m_mtx.lock();
        if (!m_is_set)
        {
            m_is_set = true;
            m_mtx.unlock();
            m_cv.notify_all();
        }
        else
        {
            m_mtx.unlock();
        }
    }

    /**
     * @brief Check if the internal flag is set.
     * 
     * @return true if the flag is set, else false
     */
    bool is_set()
    {
        mtx_lg lock {m_mtx};
        return m_is_set;
    }

    /**
     * @brief Reset the internal flag so future wait
     * calls will block. 
     */
    void reset()
    {
        mtx_lg lock {m_mtx};
        m_is_set = false;
    }
    
  private:
    bool m_is_set {false};
    std::mutex m_mtx;
    std::condition_variable m_cv;
};

/**
 * @brief A simple wrapper around a boolean, mutex, and condition variable
 * for concise thread synchronization. Each call to set() will wake at most
 * one thread.  If no threads are waiting when set() is called, nothing will 
 * happen and the next to wait will still block.
 * 
 * @note In the case of multiple waiters, it is possible for set()
 * to wake a thread that began waiting after the call to set().  
 * This is because set() releases the mutex before calling 
 * condition_variable::notify_one, so it is possible (but unlikely)
 * that a waiter will acquire the mutex and check m_set_count within 
 * this period.
 */
class unicast_event
{
    using mtx_lg = std::lock_guard<std::mutex>;
    using mtx_ul = std::unique_lock<std::mutex>;
  public:
    
    /**
     * @brief Block the calling thread until another thread calls set().
     */
    void wait()
    {
        mtx_ul lock {m_mtx};
        ++m_wait_count;
        m_cv.wait(lock, [this] {
            return m_set_count > 0;
        });
        --m_wait_count;
        --m_set_count;
    }

    /**
     * @brief Block the calling thread until another thread 
     * calls set() OR until the duration expires.
     * 
     * @param duration maximum duration thread should wait
     * @return true if set() was called; false if duration expired
     */
    template <typename rep, typename period>
    inline bool wait_for(std::chrono::duration<rep, period> duration)
    {
        mtx_ul lock {m_mtx};
        ++m_wait_count;
        const bool was_set = m_cv.wait_for(lock, duration, [this] {
            return m_set_count > 0;
        });
        --m_wait_count;
        m_set_count -= (uint16_t)was_set;
        return was_set;
    }

    /**
     * @brief Block the calling thread until another thread
     * calls set() or until the timeout is reached.
     * 
     * @param timeout maximum time point at which thread should stop waiting
     * @return true if set() was called; false if timeout was reached
     */
    template <typename clock, typename duration>
    inline bool wait_until(std::chrono::time_point<clock, duration> timeout)
    {
        mtx_ul lock {m_mtx};
        ++m_wait_count;
        const bool was_set = m_cv.wait_until(lock, timeout, [this] {
            return m_set_count > 0;
        });
        --m_wait_count;
        m_set_count -= (uint16_t)was_set;
        return was_set;
    }

    /**
     * @brief Wake a single waiting thread, if any.
     */
    void set()
    {
        m_mtx.lock();
        if (m_wait_count > m_set_count)
        {
            ++m_set_count;
            m_mtx.unlock();
            m_cv.notify_one();
        }
        else
        {
            m_mtx.unlock();
        }
    }

  private:

    /// @brief Number of threads currently waiting.
    uint16_t m_wait_count {0};
    
    /// @brief Number of pending wakes. set() can be called mutiple times
    /// before a waiter is notified, so this ensures none of those calls
    /// are redundant.
    uint16_t m_set_count {0};

    /// @brief Guards access to m_set_count and m_wait_count.
    std::mutex m_mtx;
    
    /// @brief Notifies waiting threads on set() calls.
    std::condition_variable m_cv;
};

/**
 * @brief Functionally similar to jack::unicast_event except for
 * two differences. Firstly, set() calls wake all currently waiting threads. 
 * Secondly, between a call to set() and the last waking thread releasing
 * the mutex, new waiters are temporarily blocked from the waiting pool.
 * This is to prevent already-woken waiters from returning to the
 * waiting pool and spuriously waking again.
 */
class broadcast_event
{
  public:
    
    /**
     * @brief Block the calling thread until another
     * thread calls set().
     */
    inline void wait()
    {
        auto wait_lock = m_enter_waiting_pool();
        m_cv.wait(wait_lock, [this] {
            return m_is_set;
        });
        
        // unlock pool if we are the last waiter
        if (!(m_is_set = (--m_wait_count) > 0))
        {
            m_pool_mtx.unlock();
        }
    }

    /**
     * @brief Block the calling thread until another thread 
     * calls set() OR until the duration expires.
     * 
     * @param duration maximum duration thread should wait
     * @return true if set() was called; false if duration expired
     */
    template <typename rep, typename period>
    inline bool wait_for(std::chrono::duration<rep, period> duration)
    {
        auto wait_lock = m_enter_waiting_pool();
        const bool was_set = m_cv.wait_for(wait_lock, duration, [this] {
            return m_is_set;
        });

        // if set() was called, pool is locked and
        // we might be responsible for unlocking
        if (was_set)
        {
            // unlock pool if we are the last waiter
            if (!(m_is_set = (--m_wait_count) > 0))
            {
                m_pool_mtx.unlock();
            }
        }
        else
        {            
            --m_wait_count;
        }

        return was_set;
    }

    /**
     * @brief Block the calling thread until another thread
     * calls set() or until the timeout is reached.
     * 
     * @param timeout maximum time point at which thread should stop waiting
     * @return true if set() was called; false if timeout was reached
     */
    template <typename clock, typename duration>
    inline bool wait_until(std::chrono::time_point<clock, duration> timeout)
    {
        auto wait_lock = m_enter_waiting_pool();
        const bool was_set = m_cv.wait_until(wait_lock, timeout, [this] {
            return m_is_set;
        });

        // if set() was called, pool is locked and
        // we might be responsible for unlocking
        if (was_set)
        {
            // unlock pool if we are the last waiter
            if (!(m_is_set = (--m_wait_count) > 0))
            {
                m_pool_mtx.unlock();
            }
        }
        else
        {            
            --m_wait_count;
        }

        return was_set;
    }

    /**
     * @brief Wake all currently waiting threads.  Can block
     * until previous waiting pool is finished waking.
     */
    inline void set()
    {
        m_pool_mtx.lock();
        m_wait_mtx.lock();
        if (m_is_set = m_wait_count > 0)
        {
            m_wait_mtx.unlock();
            m_cv.notify_all();
        }
        else
        {
            m_wait_mtx.unlock();
            m_pool_mtx.unlock();
        }
    }

  private:

    /**
     * @brief Block until new waiters are allowed then obtain 
     * exclusive access to and increment m_wait_count.
     * @return std::unique_lock on m_wait_mtx, 
     * guarding m_wait_count and m_is_set
     */
    std::unique_lock<std::mutex> m_enter_waiting_pool()
    {
        m_pool_mtx.lock();
        std::unique_lock<std::mutex> wait_lock {m_wait_mtx};
        ++m_wait_count;
        m_pool_mtx.unlock();
        return wait_lock;
    }

    /// @brief True if threads are in the process of waking.
    bool m_is_set {false};

    /// @brief Number of threads currently waiting.
    uint16_t m_wait_count {0};
    
    /// @brief Guards access to m_is_set and m_wait_count.
    std::mutex m_wait_mtx;
    
    /// @brief Guards access to the waiting pool (locked when m_is_set == true).
    std::mutex m_pool_mtx;
    
    /// @brief Notifies waiting threads on set() calls.
    std::condition_variable m_cv;
};


/**
 * @brief Functionally similar to jack::unicast_event except for ... TODO
 */
class multicast_event
{
  public:

    /**
     * @brief Block the calling thread until another thread calls set().
     */
    void wait()
    {
        std::unique_lock<std::mutex> wait_lock {m_mtx};
        if (m_set_count == 0)
        {
            ++m_wait_count;
            m_cv.wait(wait_lock, [this] {
                return m_set_count > 0;
            });
            --m_wait_count;
        }
        --m_set_count;
    }

    /**
     * @brief Block the calling thread until another thread 
     * calls set() OR until the duration expires.
     * 
     * @param duration maximum duration thread should wait
     * @return true if set() was called; false if duration expired
     */
    template <typename rep, typename period>
    inline bool wait_for(std::chrono::duration<rep, period> duration)
    {
        std::unique_lock<std::mutex> wait_lock {m_mtx};
        bool was_set {true};
        if (m_set_count == 0)
        {
            ++m_wait_count;
            bool was_set = m_cv.wait_for(wait_lock, duration, [this] {
                return m_set_count > 0;
            });
            --m_wait_count;

        }
        m_set_count -= (uint16_t)was_set;
        return was_set;
    }

    /**
     * @brief Block the calling thread until another thread
     * calls set() or until the timeout is reached.
     * 
     * @param timeout maximum time point at which thread should stop waiting
     * @return true if set() was called; false if timeout was reached
     */
    template <typename clock, typename duration>
    inline bool wait_until(std::chrono::time_point<clock, duration> timeout)
    {
        std::unique_lock<std::mutex> wait_lock {m_mtx};
        bool was_set {true};
        if (m_set_count == 0)
        {
            ++m_wait_count;
            was_set = m_cv.wait_until(wait_lock, timeout, [this] {
                return m_set_count > 0;
            });
            --m_wait_count;
        }
        m_set_count -= (uint16_t)was_set;
        return was_set;
    }

    /**
     * @brief Wake N waiting threads, where N is the number of
     * threads currently in the waiting pool.  It is possible that
     * new waiters will enter the pool between this call and the Nth
     * thread waking which can lead to a different group waking than that
     * which consituted the pool at the time of this call. *If this
     * behavior is undesireable, use jack::broadcast_event*
     */
    void set()
    {
        m_mtx.lock();
        if (m_wait_count > 0)
        {
            // we need to wake m_wait_count waiters
            // increment because set() could be called before
            // all the previous wakes occur
            m_set_count += m_wait_count;
            m_mtx.unlock();
            m_cv.notify_all();
        }
        else
        {
            m_mtx.unlock();
        }
    }

  private:
    
    /// @brief Number of threads currently waiting.
    uint16_t m_wait_count {0};
    
    /// @brief Number of pending wakes.
    uint16_t m_set_count {0};

    /// @brief Guards access to m_set_count and m_wait_count.
    std::mutex m_mtx;
    
    /// @brief Notifies waiting threads on set() calls.
    std::condition_variable m_cv;
    
};

// /**
//  * @brief Generic event
//  * 
//  * @tparam condition must evaluate to true for set() to wake a thread
//  */
// template <typename condition>
// class event
// {   
//     using mtx_ul = std::unique_lock<std::mutex>;
//     using mtx_lg = std::lock_guard<std::mutex>;

//     template <typename predicate>
//     inline void wait(predicate& pred)
//     {
//         mtx_ul lock {m_mtx};
//         m_cv.wait(lock, pred);
//     }

//     inline void set()
//     {
//         m_cv.notify_all();
//     }

//     std::mutex m_mtx;
//     std::condition_variable m_cv;
// }

/**
 * @brief A generic signal type for use with jack::channel.
 * It is recommended that users implement their own signal enum
 * with more autological values to improve code readability.
 */
enum class signal
{
    NONE  = 0b0000000000000000,
    SIG0  = 0b0000000000000001,
    SIG1  = 0b0000000000000010,
    SIG2  = 0b0000000000000100,
    SIG3  = 0b0000000000001000,
    SIG4  = 0b0000000000010000,
    SIG5  = 0b0000000000100000,
    SIG6  = 0b0000000001000000,
    SIG7  = 0b0000000010000000,
    SIG8  = 0b0000000100000000,
    SIG9  = 0b0000001000000000,
    SIG10 = 0b0000010000000000,
    SIG11 = 0b0000100000000000,
    SIG12 = 0b0001000000000000,
    SIG13 = 0b0010000000000000,
    SIG14 = 0b0100000000000000,
    SIG15 = 0b1000000000000000,
    ANY   = 0b1111111111111111,
};

/**
 * @brief Bitwise-OR operator upon two signal values.  Enables
 * channel::wait(SIGA | SIGB) to wait for multiple signal values.
 * @return constexpr signal
 */
constexpr inline signal operator|(signal some, signal other)
{
    return static_cast<signal>(static_cast<int>(some) |
                               static_cast<int>(other));
}

/**
 * @brief Bitwise-AND operator upon two signal values.  The channel
 * wake eligibility check requires this.  If this bitwise-AND operator
 * evaluates to true, the thread is considered eligible to wake.
 * 
 * @return true if the two signals have shared set bits, else false
 */
constexpr inline bool operator&(signal some, signal other)
{
    return static_cast<bool>(static_cast<int>(some) &
                             static_cast<int>(other));
}

/**
 * @brief Many-to-one synchronization object. Enable a consumer to
 * wait for different signal types and handle each accordingly.
 * 
 * A waiter is determined to be wake-eligible if a bitwise-AND between
 * the sent signal and the waited signal returns a non-zero value.
 */
template <typename sig_tp>
class channel
{
  public:
    explicit channel(sig_tp initial) : m_sig(initial), m_init_state(initial)
    {
    }

    inline sig_tp wait(const sig_tp& sigs)
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

    template <typename rep, typename period>
    inline sig_tp wait_for(const sig_tp sigs,
            std::chrono::duration<rep, period> duration)
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

    template <typename clock, typename duration>
    inline sig_tp wait_until(const sig_tp sigs,
            std::chrono::time_point<clock, duration> timeout)
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

    inline void signal(const sig_tp sig)
    {   
        {
            std::lock_guard<std::mutex> lock {m_mtx};
            m_sig = sig;
        }
        m_cv.notify_one();
    }

  private:

    inline sig_tp m_internal_reset()
    {
        sig_tp holder = m_sig;
        m_sig = m_init_state;
        return holder;
    }

    sig_tp m_sig;
    sig_tp m_init_state;
    std::mutex m_mtx;
    std::condition_variable m_cv;
};

// template <typename sig_tp>
// class MuxChannel;

}