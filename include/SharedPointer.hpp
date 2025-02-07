#ifndef SharedPointer_HPP_
#define SharedPointer_HPP_
#include "ControlBlock.hpp"

namespace tinysmartpointer {

// TODO: (garvey) to inherit from enable_shared_from_this(weak_ptr)
template <typename T>
class SharedPointer {
 public:
 using Deleter = std::default_delete<T>;
  // constructor
  // TODO: (garvey) to set user define deleter
  explicit SharedPointer(T* p = nullptr)
      : control_block_pointer_(new control_block_pointer_<T>(p)),
        data_pointer_(p) {}

  explicit SharedPointer(const SharedPointer<T>& p) {
    this->control_block_pointer_ = p.control_block_pointer_;
    this->IncRef();
  }

  // TODO: (garvey) initial by make_shared

  // destructor
  ~SharedPointer() {
    this->control_block_pointer_->Decrease();
  }

  T& operator*() { return this->reference_counter_->GetNum(); }

 private:
  T* data_pointer_;  // pointer to data
  ControlBlock<T>* control_block_pointer_;  // pointer to control block
  Deleter deleter;

  void IncRef() {
    this->control_block_pointer_->IncRef();
  }
};

}  // namespace tinysmartpointer
#endif