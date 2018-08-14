#include "lock.hpp"

#include <cstring>
#include <iostream>
#include <pthread.h>
#include <type_traits>

namespace wrapper {

static_assert(std::is_standard_layout<Mutex>::value);
static_assert(std::is_standard_layout<Condition>::value);

Mutex::Mutex(void) : mutex(std::make_unique<pthread_mutex_t>()) {
  pthread_mutexattr_t attr;
  int err;
  if ((err = pthread_mutexattr_init(&attr)) != 0)
    throw std::system_error(err, std::generic_category(), "pthread_mutex_attr_init failed");
  if ((err = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED)) != 0)
    throw std::system_error(err, std::generic_category(),
        "pthread_mutexattr_setpshared failed");
  if ((err = pthread_mutex_init(mutex.get(), &attr)) != 0)
    throw std::system_error(err, std::generic_category(), "pthread_mutex_init failed");
  if ((err = pthread_mutexattr_destroy(&attr)) != 0)
    throw std::system_error(err, std::generic_category(), "pthread_mutexattr_destroy failed");
}

Mutex::~Mutex(void) {
  if (mutex && pthread_mutex_destroy(mutex.get()) != 0)
    std::cerr << "WARNING: Failed to destroy pthread mutex" << std::endl;
}

Mutex::Mutex(Mutex&& o) : mutex(std::move(o.mutex)) {}

void Mutex::lock(void) {
  if (!mutex) throw std::runtime_error("mutex deleted");
  int err = pthread_mutex_lock(mutex.get());
  if (err) throw std::system_error(err, std::generic_category(), "pthread_mutex_lock failed");
  if (!mutex) throw std::runtime_error("mutex deleted");
}

void Mutex::unlock(void) {
  int err = pthread_mutex_unlock(mutex.get());
  if (err) throw std::system_error(err, std::generic_category(), "pthread_mutex_unlock failed");
}

Condition::Condition(void) : condition(std::make_unique<pthread_cond_t>()) {
  pthread_condattr_t attr;
  int err;
  if ((err = pthread_condattr_init(&attr)) != 0)
    throw std::system_error(err, std::generic_category(), "pthread_condattr_init failed");
  if ((err = pthread_condattr_setpshared(&attr, PTHREAD_PROCESS_SHARED)) != 0)
    throw std::system_error(err, std::generic_category(), "pthread_condattr_setpshared failed");
  if ((err = pthread_cond_init(condition.get(), &attr)) != 0)
    throw std::system_error(err, std::generic_category(), "pthread_cond_init failed");
  if ((err = pthread_condattr_destroy(&attr)) != 0)
    throw std::system_error(err, std::generic_category(), "pthread_condattr_destroy failed");
}

Condition::~Condition(void) {
  if (condition && pthread_cond_destroy(condition.get()) != 0)
    std::cerr << "WARNING: Failed to destroy pthread cond" << std::endl;
}

Condition::Condition(Condition&& o) : condition(std::move(o.condition)) {}

bool Condition::wait(Mutex& mutex, int timeout_ms) {
  int err;
  if (timeout_ms == -1) {
    err = pthread_cond_wait(condition.get(), mutex.mutex.get());
  } else {
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_nsec += timeout_ms * 1000000;
    err = pthread_cond_timedwait(condition.get(), mutex.mutex.get(), &ts);
  }
  if (err == ETIMEDOUT) return false;
  if (err) throw std::system_error(err, std::generic_category(), "pthread_cond_wait failed");
  if (!condition) throw std::runtime_error("condition deleted");
  return true;
}

void Condition::broadcast(void) {
  int err = pthread_cond_broadcast(condition.get());
  if (err) throw std::system_error(err, std::generic_category(), "pthread_cond_broadcast failed");
}
} // namespace wrapper
