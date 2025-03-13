#ifndef UNIQUEPOINTER_HPP_
#define UNIQUEPOINTER_HPP_

#include <cassert>
#include <iostream>
#include <memory>
#include <type_traits>

namespace TinySmartPointer {

template <typename T, typename Deleter = std::default_delete<T>>
class UniquePointer {
 public:
  using DecayedDeleter = typename std::decay_t<Deleter>;

  // constructor
  template <typename DeleterType>
  constexpr UniquePointer(T* pointer, DeleterType&& deleter)
      : pointer_(static_cast<T*>(pointer)), deleter_(std::forward<DeleterType>(deleter)) {}

  explicit constexpr UniquePointer(T* pointer)
      : deleter_(std::move(std::default_delete<T>())), pointer_(static_cast<T*>(pointer)) {}

  constexpr UniquePointer() : deleter_(std::move(std::default_delete<T>())), pointer_(nullptr) {}

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
    this->Reset(unique_pointer.Release());
  }

  template <typename DerivedType, typename = std::enable_if_t<std::is_base_of<T, DerivedType>::value>>
  UniquePointer(UniquePointer<DerivedType>&& unique_pointer)
      : pointer_(unique_pointer.Release()), deleter_(unique_pointer.Get_deleter()) {}

  // move assignment
  template <typename DerivedType, typename = std::enable_if<std::is_base_of<T, DerivedType>::value>>
  UniquePointer& operator=(UniquePointer<DerivedType>&& rhs) {
    if (this == &rhs) {
      return *this;
    }
    if (nullptr != this->pointer_) {
      deleter_(this->pointer_);
    }
    this->pointer_ = rhs.Release();
    this->deleter_ = rhs.Get_deleter();
    return *this;
  }

  T& operator*() const {
    assert(nullptr != this->pointer_);  // dangerous operator
    return *(this->pointer_);
  }

  T* operator->() const { return &(this->operator*()); }

  operator void*() const {
    if (nullptr == this->pointer_) {
      return reinterpret_cast<void*>(0);
    } else {
      return reinterpret_cast<void*>(this->pointer_);
    }
  }

  explicit operator bool() const { return nullptr != this->Get(); }

  // return dumb pointer
  T* Get() const { return this->pointer_; }

  const DecayedDeleter& Get_deleter() const { return this->deleter_; }

  DecayedDeleter& Get_deleter() { return this->deleter_; }

  void Reset(T* new_pointer = nullptr) {
    if (this->pointer_ == new_pointer) {
      return;
    }
    // swap
    std::swap(this->pointer_, new_pointer);
  }

  T* Release() {
    T* dumb_pointer = this->pointer_;
    this->pointer_ = nullptr;
    return dumb_pointer;
  }

  void Swap(UniquePointer& other) { std::swap(this->pointer_, other.pointer_); }

 private:
  T* pointer_;
  [[no_unique_address]] DecayedDeleter deleter_;
};

template <typename T, typename... Ts>
UniquePointer<T> MakeUnique(Ts&&... params) {
  return UniquePointer<T>(new T(std::forward<Ts>(params)...));
}

}  // namespace TinySmartPointer

#endif