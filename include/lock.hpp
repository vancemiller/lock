#ifndef LOCKS_HPP
#define LOCKS_HPP

#include <cstring>
#include <iostream>
#include <pthread.h>

class Mutex {
  private:
    friend class Condition;
    pthread_mutex_t mutex;
  public:
    Mutex(void) {
      pthread_mutexattr_t attr;
      if (pthread_mutexattr_init(&attr) != 0)
        throw std::runtime_error("Failed to initialize pthread mutex attributes");
      if (pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED) != 0)
        throw std::runtime_error("Failed to set pshared pthread mutex attributes");
      if (pthread_mutex_init(&mutex, &attr) != 0)
        throw std::runtime_error("Failed to initialize pthread mutex");
      if (pthread_mutexattr_destroy(&attr) != 0)
        throw std::runtime_error("Failed to destroy pthread mutex attributes");
    }

    ~Mutex(void) {
      if (pthread_mutex_destroy(&mutex) != 0)
        std::cerr << "WARNING: Failed to destroy pthread mutex" << std::endl;
    }

    void lock(void) {
      if (pthread_mutex_lock(&mutex) != 0)
        throw std::runtime_error("Failed to lock pthread mutex");
    }

    void unlock(void) {
      if (pthread_mutex_unlock(&mutex) != 0)
        throw std::runtime_error("Failed to unlock pthread mutex");
    }
};

class Condition {
  private:
    pthread_cond_t condition;
  public:
    Condition(void) {
      pthread_condattr_t attr;
      if (pthread_condattr_init(&attr) != 0)
        throw std::runtime_error("Failed to initialize pthread cond attributes");
      if (pthread_condattr_setpshared(&attr, PTHREAD_PROCESS_SHARED) != 0)
        throw std::runtime_error("Failed to set pshared pthread cond attribute");
      if (pthread_cond_init(&condition, &attr) != 0)
        throw std::runtime_error("Failed to initialize pthread cond");
      if (pthread_condattr_destroy(&attr) != 0)
        throw std::runtime_error("Failed to destroy pthread cond attributes");
    }

    ~Condition(void) {
      if (pthread_cond_destroy(&condition) != 0)
        std::cerr << "WARNING: Failed to destroy pthread cond" << std::endl;
    }

    // call with locked mutex. Unlocked during wait, locked after wait
    void wait(Mutex& mutex) {
      int err;
      if ((err = pthread_cond_wait(&condition, &mutex.mutex)) != 0)
        throw std::runtime_error("Failed to wait pthread cond: " + std::string(std::strerror(err)));
    }

    void broadcast(void) {
      if (pthread_cond_broadcast(&condition) != 0)
        throw std::runtime_error("Failed to broadcast pthread cond");
    }
};


#endif

