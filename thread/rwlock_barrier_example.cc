// This program measure the lock performance with various conditions.
//
// This program measure performance of different lock mechanism with
// different thread number and running time in critical section.
//
// The supported lock mechanisms are
// - Read Write Lock
// - Mutex

#include <boost/thread/barrier.hpp>
#include <chrono>
#include <iostream>
#include <shared_mutex>
#include <thread>

std::shared_mutex rw_mutex;
typedef std::unique_lock<std::shared_mutex> WriteLock;
typedef std::shared_lock<std::shared_mutex> ReadLock;

std::mutex mutex;
std::mutex counts_mutex;

void ReadLockFunc(uint32_t hold_time) {
  ReadLock lock(rw_mutex);
  for (uint32_t i = 0; i < hold_time; i++) {
    asm volatile("nop");
  }
}

void WriteLockFunc(uint32_t hold_time) {
  WriteLock lock(rw_mutex);
  for (uint32_t i = 0; i < hold_time; i++) {
    asm volatile("nop");
  }
}

void MutexLockFunc(uint32_t hold_time) {
  std::unique_lock<std::mutex> lock(mutex);
  for (uint32_t i = 0; i < hold_time; i++) {
    asm volatile("nop");
  }
}

class LockPerformanceChecker {
 public:
  LockPerformanceChecker(uint32_t num_threads)
      : num_threads_(num_threads),
        launch_barrier(num_threads + 1),
        finish_barrier(num_threads + 1),
        counts(num_threads) {}

  void Measure(uint32_t hold_time, std::function<void(int)> lock_fn, std::string fn_name) {
    counts.clear();
    counts.resize(num_threads_);
    running = true;
  
    auto func = [hold_time, lock_fn]() { lock_fn(hold_time); };
    std::vector<std::thread> threads;
    for (uint32_t i = 0; i < num_threads_; i++) {
      threads.push_back(std::thread(LockAndCountWorker, this, i, func));
    }

    std::thread manager_th(Manager, this, hold_time, fn_name);

    manager_th.join();
    for (auto& thread : threads) {
      thread.join();
    }
  }

  static void LockAndCountWorker(LockPerformanceChecker* checker, uint64_t id,
                                 std::function<void()> lock_fn) {
    checker->launch_barrier.wait();
    uint64_t count = 0;

    while (checker->running) {
      lock_fn();
      ++count;
    }
    std::unique_lock<std::mutex> counts_lock(counts_mutex);
    checker->counts[id] = count;
    counts_lock.unlock();

    checker->finish_barrier.wait();
  }

  static void Manager(LockPerformanceChecker* checker, uint32_t hold_time, std::string fn_name) {
    checker->launch_barrier.wait();

    std::this_thread::sleep_for(std::chrono::seconds(3));
    checker->running = false;

    checker->finish_barrier.wait();

    uint64_t sum_count = 0;
    for (const auto& count : checker->counts) {
      sum_count += count;
    }
    std::cout << checker->num_threads_ << "," << fn_name << "-" << hold_time << ","
              << sum_count / checker->counts.size() << std::endl;
  }

 private:
  uint32_t num_threads_;

  boost::barrier launch_barrier;
  boost::barrier finish_barrier;

  std::vector<uint64_t> counts;
  bool running = true;
};

#define OBJECT_NAME(f) #f

int main() {
  std::vector<std::function<void(int)>> lock_fns = {ReadLockFunc, WriteLockFunc,
                                                    MutexLockFunc};
  std::vector<std::string> fn_names = {OBJECT_NAME(ReadLockFunc), OBJECT_NAME(WriteLockFunc),
                                                    OBJECT_NAME(MutexLockFunc)};
  for (uint32_t i = 1; i < 15; i++) {
    LockPerformanceChecker checker(i);
    for (uint32_t j = 1; j < 5; j++) {
      for (size_t k = 0; k < lock_fns.size(); k++) {
        checker.Measure(std::pow(10, j), lock_fns[k], fn_names[k]);
      }
    }
  }
}