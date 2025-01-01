#ifndef SharedPointer_HPP_
#define SharedPointer_HPP_
#include "RCObject.hpp"

namespace tinysmartpointer {
template <class T>
class SharedPointer {
 public:
  // constructor
  explicit SharedPointer(T* p = nullptr)
      : reference_counter_(new RCObject<T>(p)) {}

  explicit SharedPointer(const SharedPointer<T>& p) {
    this->reference_counter_ = p.reference_counter_;
    this->reference_counter_->Increase();
  }

  // destructor
  ~SharedPointer() {
    if (0 == this->reference_counter_->Decrease()) {
      delete this->reference_counter_;
      this->reference_counter_ = nullptr;
    }
  }

  T& operator*() { return this->reference_counter_->GetNum(); }

 private:
  RCObject<T>* reference_counter_;
};

}  // namespace tinysmartpointer
#endif