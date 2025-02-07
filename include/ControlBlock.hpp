#ifndef RCObject_HPP_
#define RCObject_HPP_
#include <atomic>
#include <cassert>
#include <iostream>
#include <memory>
#include <type_traits>

namespace tinysmartpointer {

template <typename T>
class ControlBlock {
 public:
  // constructor
  template <typename Deleter = std::default_delete<T>>
  ControlBlock(T *p, Deleter deleter)
      : reference_count_{1}, weak_count_{0}, pointer_(static_cast<T *>(p)), deleter(deleter) {}

  // destructor
  ~ControlBlock() {
    if (0 == reference_count_) {
      delete pointer_;
      pointer_ = nullptr;
    }
  }

  void Increase() { ++this->reference_count_; }

  void Decrease() {
    --this->reference_count_;
    if (nullptr != this->pointer_ && 0 == this->reference_count_.load(std::memory_order_release)) {
      deleter(this->pointer_);
    }
  }

  T *Get() { return this->pointer_; }

  T& GetNum() { return *(this->pointer_); }

 private:
  T *pointer_;
  std::atomic<int> reference_count_;
  std::atomic<int> weak_count_;
  Deleter deleter;
};
}  // namespace tiny_smartpointer_
#endif