#ifndef UNIQUEPOINTER_HPP_
#define UNIQUEPOINTER_HPP_

#include <iostream>
#include <memory>

namespace tinysmartpointer {

template <typename T, typename Deleter = std::default_delete<T>>
class UniquePointer {
 public:
  using DecayedDeleter = typename std::decay_t<Deleter>;

  // constructor
  template <typename DeleterType>
  constexpr UniquePointer(T* pointer, DeleterType&& deleter) {
    deleter_ = std::forward<DeleterType>(deleter);
    pointer_ = static_cast<T*>(pointer);
  }

  explicit UniquePointer(T* pointer)
      : deleter_(std::move(std::default_delete<T>())),
        pointer_(static_cast<T*>(pointer)) {}

  UniquePointer()
      : deleter_(std::move(std::default_delete<T>())), pointer_(nullptr) {}

  // destructor
  ~UniquePointer() {
    if (nullptr != pointer_) {
      deleter_(pointer_);
      pointer_ = nullptr;
    }
  }

  // copy constructor
  UniquePointer(const UniquePointer&) = delete;

  // copy assignment
  UniquePointer& operator=(const UniquePointer&) = delete;

  // move constructor
  UniquePointer(UniquePointer&& unique_pointer) {
    this->pointer_ = std::move(unique_pointer.pointer_);
    unique_pointer.pointer_ = nullptr;
    this->deleter_ = std::move(unique_pointer.deleter_);
    unique_pointer.deleter_ = std::move(std::default_delete<T>());
  }

  // move assignment
  UniquePointer& operator=(UniquePointer&& rhs) {
    if (this == &rhs) {
      return;
    }
    if (nullptr != this->pointer_) {
      deleter_(this->pointer_);
    }
    this->pointer_ = std::move(rhs.pointer_);
    rhs.pointer_ = nullptr;
    this->pointer = std::move(rhs.deleter_);
    rhs.deleter_ = std::move(std::default_delete<T>());
    return *this;
  }

  // TODO: (garvey) proxy
  T& operator*() const {
    // assert(nullptr != unique_pointer.pointer_);
    return *(this->pointer_);
  }

  T* operator->() const { return &(this->operator*()); }

  // return dumb pointer
  T* Get() const { return this->pointer_; }

  void Reset(const T* new_pointer = nullptr) {
    if (this->pointer_ == new_pointer) {
      return;
    }
    // swap
    UniquePointer tmp_unique_pointer(new_pointer);
    std::move(*this, tmp_unique_pointer);
  }

  // TODO
  // operator SharedPointer<T>() {}

 private:
  T* pointer_;
  [[no_unique_address]] DecayedDeleter deleter_;
};

}  // namespace tinysmartpointer

#endif