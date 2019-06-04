#include "lock.hpp"

#include <cstring>
#include <iostream>
#include <pthread.h>
#include <type_traits>

#define CHECK_ERRNO(cmd, success)                                                                  \
  {                                                                                                \
    auto ret(cmd);                                                                                 \
    if (ret != success) {                                                                          \
      throw std::system_error(ret, std::generic_category(), #cmd);                                 \
      exit(EXIT_FAILURE);                                                                          \
    }                                                                                              \
  }

namespace wrapper {

Mutex::Mutex(void) {
  pthread_mutexattr_t attr;
  CHECK_ERRNO(pthread_mutexattr_init(&attr), 0);
  CHECK_ERRNO(pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED), 0);
  CHECK_ERRNO(pthread_mutex_init(&m, &attr), 0);
  CHECK_ERRNO(pthread_mutexattr_destroy(&attr), 0);
}

Mutex::~Mutex(void) {
  int ret(pthread_mutex_destroy(&m));
  if (ret)
    fprintf(stderr, "pthread_mutex_destroy failed with error %s\n", strerror(errno));
}

void Mutex::lock(void) {
  CHECK_ERRNO(pthread_mutex_lock(&m), 0);
}

void Mutex::unlock(void) {
  CHECK_ERRNO(pthread_mutex_unlock(&m), 0);
}

Condition::Condition(void) {
  pthread_condattr_t attr;
  CHECK_ERRNO(pthread_condattr_init(&attr), 0);
  CHECK_ERRNO(pthread_condattr_setpshared(&attr, PTHREAD_PROCESS_SHARED), 0);
  CHECK_ERRNO(pthread_cond_init(&c, &attr), 0);
  CHECK_ERRNO(pthread_condattr_destroy(&attr), 0);
}

Condition::~Condition(void) {
  int ret(pthread_cond_destroy(&c));
  if (ret)
    fprintf(stderr, "pthread_cond_destroy failed with error %s\n", strerror(errno));
}

bool Condition::wait(Mutex& mutex, int timeout_ms) {
  int ret;
  if (timeout_ms == -1) {
    ret = pthread_cond_wait(&c, &mutex.m);
  } else {
    timespec timeout;
    clock_gettime(CLOCK_REALTIME, &timeout);

    timeout.tv_sec += timeout_ms / 1000;
    timeout.tv_nsec += (timeout_ms % 1000) * 1000000;
    if (timeout.tv_nsec >= 1000000000) {
      timeout.tv_sec += 1;
      timeout.tv_nsec -= 1000000000;
    }
    ret = pthread_cond_timedwait(&c, &mutex.m, &timeout);
    clock_gettime(CLOCK_REALTIME, &timeout);
  }
  if (ret == ETIMEDOUT) return false;
  if (ret) throw std::system_error(ret, std::generic_category(), "pthread_cond_wait failed");
  return true;
}

void Condition::broadcast(void) {
  CHECK_ERRNO(pthread_cond_broadcast(&c), 0);
}
} // namespace wrapper

#undef CHECK_ERRNO
