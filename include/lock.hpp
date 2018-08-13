#ifndef LOCKS_HPP
#define LOCKS_HPP

#include <cstring>
#include <iostream>
#include <pthread.h>

class Mutex {
  private:
    friend class Condition;
    pthread_mutex_t mutex;
    bool moved = false;
  public:
    Mutex(void) {
      pthread_mutexattr_t attr;
      int err;
      if ((err = pthread_mutexattr_init(&attr)) != 0)
        throw std::system_error(err, std::generic_category(), "pthread_mutex_attr_init failed");
      if ((err = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED)) != 0)
        throw std::system_error(err, std::generic_category(),
            "pthread_mutexattr_setpshared failed");
      if ((err = pthread_mutex_init(&mutex, &attr)) != 0)
        throw std::system_error(err, std::generic_category(), "pthread_mutex_init failed");
      if ((err = pthread_mutexattr_destroy(&attr)) != 0)
        throw std::system_error(err, std::generic_category(), "pthread_mutexattr_destroy failed");
    }

    ~Mutex(void) {
      if (!moved && pthread_mutex_destroy(&mutex) != 0)
        std::cerr << "WARNING: Failed to destroy pthread mutex" << std::endl;
    }

    Mutex(Mutex& o) = delete;
    Mutex(const Mutex& o) = delete;
    Mutex(Mutex&& o) : mutex(o.mutex) { o.moved = true; }

    void lock(void) {
      if (moved) throw std::runtime_error("Mutex is not valid");
      int err = pthread_mutex_lock(&mutex);
      if (err)
        throw std::system_error(err, std::generic_category(), "pthread_mutex_lock failed");
    }

    void unlock(void) {
      if (moved) throw std::runtime_error("Mutex is not valid");
      int err = pthread_mutex_unlock(&mutex);
      if (err)
        throw std::system_error(err, std::generic_category(), "pthread_mutex_unlock failed");
    }
};

class Condition {
  private:
    pthread_cond_t condition;
    bool moved = false;
  public:
    Condition(void) {
      pthread_condattr_t attr;
      int err;
      if ((err = pthread_condattr_init(&attr)) != 0)
        throw std::system_error(err, std::generic_category(), "pthread_condattr_init failed");
      if ((err = pthread_condattr_setpshared(&attr, PTHREAD_PROCESS_SHARED)) != 0)
        throw std::system_error(err, std::generic_category(), "pthread_condattr_setpshared failed");
      if ((err = pthread_cond_init(&condition, &attr)) != 0)
        throw std::system_error(err, std::generic_category(), "pthread_cond_init failed");
      if ((err = pthread_condattr_destroy(&attr)) != 0)
        throw std::system_error(err, std::generic_category(), "pthread_condattr_destroy failed");
    }

    ~Condition(void) {
      if (!moved && pthread_cond_destroy(&condition) != 0)
        std::cerr << "WARNING: Failed to destroy pthread cond" << std::endl;
    }

    Condition(Condition& o) = delete;
    Condition(const Condition& o) = delete;
    Condition(Condition&& o) : condition(o.condition) { o.moved = true; }

    /**
     * Waitis for the condition to be signalled by a call to broadcast.
     * Call with locked mutex. Mutex is unlocked while waiting on the condition.
     * When the condition is signalled and the wait returns the mutex is locked.
     *
     * Timeout: -1 to wait forever, otherwise wait returns false if the timeout expires before
     * the condition has been signalled.
     */
    bool wait(Mutex& mutex, int timeout_ms=-1) {
      if (moved) throw std::runtime_error("Condition is not valid");
      int err;
      if (timeout_ms == -1) {
        err = pthread_cond_wait(&condition, &mutex.mutex);
      } else {
        timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_nsec += timeout_ms * 1000000;
        err = pthread_cond_timedwait(&condition, &mutex.mutex, &ts);
      }
      if (err == ETIMEDOUT) return false;
      if (err) throw std::system_error(err, std::generic_category(), "pthread_cond_wait failed");
      return true;
    }

    void broadcast(void) {
      if (moved) throw std::runtime_error("Condition is not valid");
      int err = pthread_cond_broadcast(&condition);
      if (err)
        throw std::system_error(err, std::generic_category(), "pthread_cond_broadcast failed");
    }
};

#endif
