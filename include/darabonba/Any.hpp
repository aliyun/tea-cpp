#ifndef SIMPLE_ANY_HPP
#define SIMPLE_ANY_HPP

#include <stdexcept>
#include <typeinfo>

class SimpleAny {
public:
  SimpleAny() : ptr(nullptr) {}

  template <typename T> SimpleAny(const T &value) : ptr(new Holder<T>(value)) {}

  SimpleAny(const SimpleAny &other)
      : ptr(other.ptr ? other.ptr->clone() : nullptr) {}

  ~SimpleAny() { delete ptr; }

  SimpleAny &operator=(const SimpleAny &other) {
    if (this != &other) {
      delete ptr;
      ptr = other.ptr ? other.ptr->clone() : nullptr;
    }
    return *this;
  }

  template <typename T> T cast() const {
    if (typeid(T) != ptr->typeInfo()) {
      throw std::bad_cast();
    }
    return static_cast<Holder<T> *>(ptr)->held;
  }

  const std::type_info &type() const {
    return ptr ? ptr->typeInfo() : typeid(void);
  }

private:
  struct Base {
    virtual ~Base() {}
    virtual const std::type_info &typeInfo() const = 0;
    virtual Base *clone() const = 0;
  };

  template <typename T> struct Holder : Base {
    Holder(const T &value) : held(value) {}

    virtual const std::type_info &typeInfo() const override {
      return typeid(T);
    }

    virtual Base *clone() const override { return new Holder(held); }

    T held;
  };

  Base *ptr;
};

#endif // SIMPLE_ANY_HPP
