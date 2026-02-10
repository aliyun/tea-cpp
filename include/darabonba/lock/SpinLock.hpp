
#ifndef SPIN_LOCK_H_
#define SPIN_LOCK_H_
#include <atomic>
#include <thread>

namespace Darabonba {
namespace Lock {
class SpinLock {
public:
  SpinLock() = default;

  SpinLock(const SpinLock &) = delete;

  SpinLock(SpinLock &&other) = delete;

  SpinLock &operator=(const SpinLock &) = delete;

  SpinLock &operator=(SpinLock &&) = delete;

  void lock() {
    // 使用指数退避策略减少 CPU 消耗
    int spinCount = 0;
    while (flag_.test_and_set(std::memory_order_acquire)) {
      if (++spinCount > kMaxSpinCount) {
        std::this_thread::yield();
        spinCount = 0;
      }
    }
  }

  bool try_lock() { return !flag_.test_and_set(std::memory_order_acquire); }

  void unlock() { flag_.clear(std::memory_order_release); }

protected:
  static constexpr int kMaxSpinCount = 16;
  std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
};

} // namespace Lock
} // namespace Darabonba

#endif