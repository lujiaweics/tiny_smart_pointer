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
  using Deleter = std::default_delete<T>;

  // constructor
  ControlBlock(T *p = nullptr, Deleter deleter = std::default_delete<T>())
      : reference_count_{1},
        pointer_(static_cast<T *>(p)),
        deleter(std::move(deleter)) {}

  // destructor
  ~ControlBlock() {
    if (0 == this->reference_count_ && nullptr != this->pointer_) {
      deleter(pointer_);
      pointer_ = nullptr;
    }
  }

  void IncRef() { ++this->reference_count_; }

  void DecRef() {
    --this->reference_count_;
    if (0 == this->reference_count_) {
      delete this;
    }
  }

  int GetNum() { return reference_count_.load(); }

  Deleter* GetDeleter() {
    return &deleter;
  }

 private:
  T *pointer_;
  std::atomic<int> reference_count_;
  // std::atomic<int> weak_count_;
  Deleter deleter;
};
}  // namespace tinysmartpointer
#endif