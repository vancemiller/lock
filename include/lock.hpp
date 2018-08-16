#ifndef LOCKS_HPP
#define LOCKS_HPP

#include <pthread.h>

namespace wrapper {
class Mutex {
  private:
    friend class Condition;
    pthread_mutex_t mutex;
    bool moved = false;
  public:
    Mutex(void);
    Mutex(Mutex& o) = delete;
    Mutex(const Mutex& o) = delete;
    Mutex(Mutex&& o);
    ~Mutex(void);
    void lock(void);
    void unlock(void);
};

class Condition {
  private:
    pthread_cond_t condition;
    bool moved = false;
  public:
    Condition(void);
    Condition(Condition& o) = delete;
    Condition(const Condition& o) = delete;
    Condition(Condition&& o);
    ~Condition(void);

    /**
     * Waits for the condition to be signalled by a call to broadcast.
     * Call with locked mutex. Mutex is unlocked while waiting on the condition.
     * When the condition is signalled and the wait returns the mutex is locked.
     *
     * Timeout: -1 to wait forever, otherwise wait returns false if the timeout expires before
     * the condition has been signalled.
     */
    bool wait(Mutex& mutex, int timeout_ms=-1);
    void broadcast(void);
};
} // namespace wrapper

#endif
