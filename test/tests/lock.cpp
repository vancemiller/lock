#include "gtest/gtest.h"
#include "lock.hpp"

#include <mutex>
#include <thread>

namespace wrapper {
// Black box testing of the Mutex api doesn't allow for very interesting tests

TEST(Mutex, LockUnlock) {
  Mutex m;
  EXPECT_NO_THROW(m.lock());
  EXPECT_NO_THROW(m.unlock());
  EXPECT_TRUE(true);// we didn't deadlock
}

TEST(Mutex, LockUnlockGuard) {
  Mutex m;
  {
    std::lock_guard<Mutex> lock(m);
  }
  EXPECT_TRUE(true);// we didn't deadlock
}

TEST(Mutex, LockAndLock) {
  Mutex m;
  for (size_t i = 0; i < 6666; i++) {
    std::lock_guard<Mutex> lock(m);
  }
  EXPECT_TRUE(true);// we didn't deadlock
}

static void recurseLock(int depth) {
  if (depth > 0) {
    Mutex m;
    std::lock_guard<Mutex> lock(m);
    recurseLock(--depth);
  }
}

TEST(Mutex, RecursiveLock) {
  recurseLock(6666);
  EXPECT_TRUE(true);// we didn't deadlock
}

TEST(Mutex, LockBlock) {
  const uint64_t thread_time = 500000;
  Mutex m;
  std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
  std::thread t1([&m] (void) -> void {
      m.lock();
      usleep(thread_time);
      m.unlock();
      });
  std::thread t2([&m] (void) -> void {
      m.lock();
      usleep(thread_time);
      m.unlock();
      });
  t1.join();
  t2.join();
  std::chrono::steady_clock::time_point stop = std::chrono::steady_clock::now();

  uint64_t elapsed = std::chrono::nanoseconds(stop - start).count() / 1000;
  EXPECT_GT(elapsed, 2 * thread_time);// one thread blocked the other
}

TEST(Mutex, Move) {
  Mutex m;
  Mutex m2(std::move(m));
  EXPECT_NO_THROW(m2.lock());
  EXPECT_NO_THROW(m2.unlock());
}

TEST(Condition, Broadcast) {
  Condition c_cond;
  EXPECT_NO_THROW(c_cond.broadcast());
}

TEST(Condition, WaitBroadcast) {
  bool c = false;
  Condition c_cond;
  Mutex m;
  std::thread t1([&m, &c_cond, &c] (void) -> void {
      std::lock_guard<Mutex> lock(m);
      while (!c)
        c_cond.wait(m);
  });
  std::thread t2([&m, &c_cond, &c] (void) -> void {
      m.lock();
      c = true;
      m.unlock();
      c_cond.broadcast();
  });
  t1.join();
  t2.join();
  EXPECT_TRUE(true); // we didn't deadlock
}

TEST(Condition, WaitBroadcastFirst) {
  bool c = false;
  Condition c_cond;
  Mutex m;
  std::thread t2([&m, &c_cond, &c] (void) -> void {
      m.lock();
      c = true;
      m.unlock();
      c_cond.broadcast();
  });
  std::thread t1([&m, &c_cond, &c] (void) -> void {
      usleep(500000);
      std::lock_guard<Mutex> lock(m);
      while (!c)
        c_cond.wait(m);
  });
  t1.join();
  t2.join();
  EXPECT_TRUE(true); // we didn't deadlock
}

TEST(Condition, WaitFirstThenBroadcast) {
  bool c = false;
  Condition c_cond;
  Mutex m;
  std::thread t1([&m, &c_cond, &c] (void) -> void {
      std::lock_guard<Mutex> lock(m);
      while (!c)
        c_cond.wait(m);
  });
  std::thread t2([&m, &c_cond, &c] (void) -> void {
      usleep(500000);
      m.lock();
      c = true;
      m.unlock();
      c_cond.broadcast();
  });
  t1.join();
  t2.join();
  EXPECT_TRUE(true); // we didn't deadlock
}

TEST(Condition, LoopWaitBroadcast) {
  const uint32_t iterations = 6666;
  uint32_t t1_count, t2_count;
  uint32_t read = 0;
  bool write;
  Mutex m;
  Condition read_cond;
  Condition write_cond;
  std::thread t1([&t1_count, &m, &read, &read_cond, &write, &write_cond] (void) -> void {
      t1_count = 0;
      for (uint32_t i = 0; i < iterations; i++) {
        std::lock_guard<Mutex> lock(m);
        while (read <= t1_count)
          read_cond.wait(m);
        t1_count = read;
        write = true;
        write_cond.broadcast();

      }
  });
  std::thread t2([&t2_count, &m, &read, &read_cond, &write, &write_cond] (void) -> void {
      t2_count = 0;
      for (uint32_t i = 0; i < iterations; i++) {
        std::lock_guard<Mutex> lock(m);
        read = ++t2_count;
        read_cond.broadcast();
        write = false;
        while (!write)
          write_cond.wait(m);
      }
  });
  t1.join();
  t2.join();
  EXPECT_EQ(t1_count, iterations);
  EXPECT_EQ(t2_count, iterations);
}

TEST(Condition, Timeout) {
  Condition neverSignalled;
  Mutex m;
  std::lock_guard<Mutex> lock(m);
  neverSignalled.wait(m, 100);
}

TEST(Condition, Move) {
  Condition r;
  Condition r2(std::move(r));
  Mutex m;
  std::lock_guard<Mutex> lock(m);
  EXPECT_NO_THROW(r2.broadcast());
}
} // namespace wrapper
