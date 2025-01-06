#ifndef UNIQUEPOINTER_HPP_
#define UNIQUEPOINTER_HPP_

#include <iostream>

namespace tinysmartpointer {

template <class T>
class UniquePointer {
 public:
  // constructor
  explicit constexpr UniquePointer(T* pointer = nullptr) noexcept {
    pointer_ = static_cast<T*>(pointer);
  }

  // destructor
  ~UniquePointer() {
    if (nullptr != pointer_) {
      delete pointer_;
      pointer_ = nullptr;
    }
  }

  // copy constructor
  UniquePointer(const UniquePointer&) = delete;

  // copy assignment
  UniquePointer& operator=(const UniquePointer&) = delete;

  // move constructor
  UniquePointer(UniquePointer&& unique_pointer) {
    this->pointer_ = unique_pointer.pointer_;
    unique_pointer.pointer_ = nullptr;
  }

  // move assignment
  UniquePointer& operator=(UniquePointer&& rhs) {
    this->pointer_ = rhs.pointer_;
    rhs.pointer_ = nullptr;

    return *this;
  }

  // TODO: (garvey) proxy
  T& operator*() const {
    // assert(nullptr != unique_pointer.pointer_);
    return *(this->pointer_);
  }

  T* operator->() const {
    return &(this->operator*());
  }

  // return dumb pointer
  T *Get() {
    return this->pointer_;
  }

 private:
  T* pointer_;
};

}  // namespace tinysmartpointer

#endif