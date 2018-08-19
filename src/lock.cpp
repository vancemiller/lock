#include "lock.hpp"

#include <cstring>
#include <iostream>
#include <pthread.h>
#include <type_traits>

namespace wrapper {

Mutex::Mutex(void) {
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

Mutex::~Mutex(void) {
  if (!moved && pthread_mutex_destroy(&mutex) != 0)
    std::cerr << "WARNING: Failed to destroy pthread mutex" << std::endl;
}

Mutex::Mutex(Mutex&& o) : mutex(o.mutex) { o.moved = true; }

void Mutex::lock(void) {
  if (moved) throw std::runtime_error("invalid mutex");
  int err = pthread_mutex_lock(&mutex);
  if (err) throw std::system_error(err, std::generic_category(), "pthread_mutex_lock failed");
  if (moved) throw std::runtime_error("invalid mutex");
}

void Mutex::unlock(void) {
  if (moved) throw std::runtime_error("invalid mutex");
  int err = pthread_mutex_unlock(&mutex);
  if (err) throw std::system_error(err, std::generic_category(), "pthread_mutex_unlock failed");
}

Condition::Condition(void) {
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

Condition::~Condition(void) {
  if (!moved && pthread_cond_destroy(&condition) != 0)
    std::cerr << "WARNING: Failed to destroy pthread cond" << std::endl;
}

Condition::Condition(Condition&& o) : condition(o.condition) { o.moved = true; }

bool Condition::wait(Mutex& mutex, int timeout_ms) {
  if (moved) throw std::runtime_error("invalid condition");
  int err;
  if (timeout_ms == -1) {
    err = pthread_cond_wait(&condition, &mutex.mutex);
  } else {
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    long offset_ms = timeout_ms;
    while (offset_ms > 1000) {
      ts.tv_sec += 1;
      offset_ms -= 1000;
    }
    long offset_ns = offset_ms * 1000000;
    ts.tv_nsec += offset_ns;
    if (ts.tv_nsec >= 1000000000) {
      ts.tv_sec += 1;
      ts.tv_nsec -= 1000000000;
    }
    err = pthread_cond_timedwait(&condition, &mutex.mutex, &ts);
  }
  if (err == ETIMEDOUT) return false;
  if (err) throw std::system_error(err, std::generic_category(), "pthread_cond_wait failed");
  if (moved) throw std::runtime_error("invalid condition");
  return true;
}

void Condition::broadcast(void) {
  if (moved) throw std::runtime_error("invalid condition");
  int err = pthread_cond_broadcast(&condition);
  if (err) throw std::system_error(err, std::generic_category(), "pthread_cond_broadcast failed");
}
} // namespace wrapper
