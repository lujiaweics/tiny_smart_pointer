#ifndef RCObject_HPP_
#define RCObject_HPP_
#include <atomic>

namespace tinysmartpointer {

template <class T>
class RCObject {
 public:
  // constructor
  RCObject(T *p = nullptr)
      : reference_count_(1), pointer_(static_cast<T *>(p)) {}

  // destructor
  ~RCObject() {
    if (0 == reference_count_) {
      delete pointer_;
      pointer_ = nullptr;
    }
  }

  void Increase() { ++this->reference_count_; }

  int Decrease() { return --(this->reference_count_); }

  T *Get() { return this->pointer_; }

  T& GetNum() { return *(this->pointer_); }

 private:
  T *pointer_;
  std::atomic<int> reference_count_;
};
}  // namespace tiny_smartpointer_
#endif