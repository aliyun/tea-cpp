//
// Created by page on 2025/5/10.
//

#ifndef DARABONBA_CORE_GENERATOR_H
#define DARABONBA_CORE_GENERATOR_H

namespace Darabonba {
  template<typename T>
  class Generator {
  public:
    using Container = std::vector<T>;

    Generator(Container container) : container_(container), index_(0) {}

    class Iterator {
    public:
      using iterator_category = std::forward_iterator_tag;
      using difference_type   = std::ptrdiff_t;
      using value_type        = T;
      using pointer           = T*;
      using reference         = T&;

      Iterator(pointer ptr) : m_ptr(ptr) {}

      reference operator*() const { return *m_ptr; }
      pointer operator->() { return m_ptr; }

      // 前置递增
      Iterator& operator++() {
        m_ptr++;
        return *this;
      }

      friend bool operator==(const Iterator& a, const Iterator& b) { return a.m_ptr == b.m_ptr; }
      friend bool operator!=(const Iterator& a, const Iterator& b) { return a.m_ptr != b.m_ptr; }

    private:
      pointer m_ptr;
    };

    Iterator begin() const { return Iterator(&container_[0]); }
    Iterator end() const { return Iterator(&container_[container_.size()]); }

  private:
    Container container_;
    size_t index_;
  };

  template<typename T>
  class FutureGenerator {
  public:
    using FutureContainer = std::vector<std::future<T>>;

    FutureGenerator(FutureContainer futures) : futures_(std::move(futures)) {}

    class Iterator {
    public:
      using iterator_category = std::forward_iterator_tag;
      using difference_type   = std::ptrdiff_t;
      using value_type        = T;
      using pointer           = T*;
      using reference         = T&;

      Iterator(typename FutureContainer::iterator iter) : iter_(iter) {}

      T operator*() {
        return iter_->get(); // 获取 future 结果
      }

      Iterator& operator++() {
        ++iter_;
        return *this;
      }

      friend bool operator==(const Iterator& a, const Iterator& b) {
        return a.iter_ == b.iter_;
      }

      friend bool operator!=(const Iterator& a, const Iterator& b) {
        return a.iter_ != b.iter_;
      }

    private:
      typename FutureContainer::iterator iter_;
    };

    Iterator begin() { return Iterator(futures_.begin()); }
    Iterator end() { return Iterator(futures_.end()); }

  private:
    FutureContainer futures_;
  };
} // end Darabonba

#endif // DARABONBA_CORE_GENERATOR_H
