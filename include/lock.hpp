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
      if (pthread_mutex_destroy(&mutex) != 0)
        std::cerr << "WARNING: Failed to destroy pthread mutex" << std::endl;
    }

    void lock(void) {
      int err = pthread_mutex_lock(&mutex);
      if (err)
        throw std::system_error(err, std::generic_category(), "pthread_mutex_lock failed");
    }

    void unlock(void) {
      int err = pthread_mutex_unlock(&mutex);
      if (err)
        throw std::system_error(err, std::generic_category(), "pthread_mutex_unlock failed");
    }
};

class Condition {
  private:
    pthread_cond_t condition;
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
      if (pthread_cond_destroy(&condition) != 0)
        std::cerr << "WARNING: Failed to destroy pthread cond" << std::endl;
    }

    // call with locked mutex. Unlocked during wait, locked after wait
    void wait(Mutex& mutex) {
      int err = pthread_cond_wait(&condition, &mutex.mutex);
      if (err)
        throw std::system_error(err, std::generic_category(), "pthread_cond_wait failed");
    }

    void broadcast(void) {
      int err = pthread_cond_broadcast(&condition);
      if (err)
        throw std::system_error(err, std::generic_category(), "pthread_cond_broadcast failed");
    }
};

#endif
