#ifndef SharedPointer_HPP_
#define SharedPointer_HPP_
// #include "ControlBlock.hpp"

namespace tinysmartpointer {

// TODO: (garvey) to inherit from enable_shared_from_this(weak_ptr)
template <class T>
class SharedPointer {
 public:
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
    if (0 == this->reference_counter_->Decrease()) {
      delete this->reference_counter_;
      this->reference_counter_ = nullptr;
    }
  }

  T& operator*() { return this->reference_counter_->GetNum(); }

 private:
  ControlBlock<T>* control_block_pointer_;  // pointer to control block
  /* the reason why data pointer is not set in control block is may be the
   * deleter is also in control block */
  T* data_pointer_;  // pointer to data

  void IncRef() {
    this->control_block_pointer_->IncRef();
  }
};

}  // namespace tinysmartpointer
#endif