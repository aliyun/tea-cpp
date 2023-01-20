
#ifndef SPIN_LOCK_H_
#define SPIN_LOCK_H_
#include <atomic>

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
    while (flag_.test_and_set(std::memory_order_acquire))
      ;
  }

  bool try_lock() { return !flag_.test_and_set(std::memory_order_acquire); }

  void unlock() { flag_.clear(std::memory_order_release); }

protected:
  std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
};

} // namespace Lock
} // namespace Darabonba

#endif