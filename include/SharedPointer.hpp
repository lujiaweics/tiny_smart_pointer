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
  SharedPointer() : control_block_pointer_(nullptr), data_pointer_(nullptr) {}

  explicit SharedPointer(T* p)
      : control_block_pointer_(nullptr), data_pointer_(p) {
    if (nullptr != p) {
      control_block_pointer_ = new ControlBlock<T>(p);
    }
  }

  SharedPointer(T* p, Deleter user_defined_deleter)
      : data_pointer_(p), control_block_pointer_(nullptr) {
    if (nullptr != p) {
      control_block_pointer_ =
          new ControlBlock<T>(p, user_defined_deleter);
    }
  }

  explicit SharedPointer(const SharedPointer<T>& p) {
    this->control_block_pointer_ = p.control_block_pointer_;
    this->data_pointer_ = p.data_pointer_;
    this->IncRef();
  }

  // copy construct from an existed shared_ptr but not pointer to origin address
  template <typename Ty>
  SharedPointer(const SharedPointer<Ty>& p, T* ptr) {
    this->control_block_pointer_ = p.control_block_pointer_;  // TODO: earse control block type
    this->data_pointer_ = ptr;
    this->IncRef();
  }

  // destructor
  ~SharedPointer() {
    if (this->control_block_pointer_) {
      this->control_block_pointer_->DecRef();
    }
  }

  T& operator*() const {
    return *(this->data_pointer_);
  }

  T* operator->() const {
    return &(this->operator*());
  }

  int UseCount() const {
    if (nullptr == data_pointer_) {
      return 0;
    }
    return this->control_block_pointer_->GetNum();
  }

  bool Unique() const {
    if (nullptr == this->data_pointer_) {
      return false;
    }

    if (1 == this->UseCount()) {
      return true;
    } else {
      return false;
    }
  }

  T* Get() const { return this->data_pointer_; }

  template <typename Ty>
  friend class SharedPointer;

 private:
  T* data_pointer_;                         // pointer to data
  ControlBlock<T>* control_block_pointer_;  // pointer to control block
  Deleter deleter;

  void IncRef() { this->control_block_pointer_->IncRef(); }
};

// TODO: (garvey) initial by make_shared

}  // namespace tinysmartpointer
#endif