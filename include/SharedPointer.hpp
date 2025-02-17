#ifndef SharedPointer_HPP_
#define SharedPointer_HPP_

namespace tinysmartpointer {

template<typename _T>
class WeakPointer;

class ControlBlockBase {
 public:
  ControlBlockBase() : weak_count(0), shared_count(0) {};

  virtual ~ControlBlockBase() = 0;

  long UseCount() const {
    return static_cast<int>(this->shared_count);
  }
 private:
  std::atomic<int> weak_count;
  std::atomic<int> shared_count;
};

template<typename _T>
class ControlBlockImpl : public ControlBlockBase{
 public:

 private:
  T_ *ptr;
  [[no_unique_address]] Deleter _Del;
};

template <typename _T>
class SharedPointer {
 public:
  // constructor
  SharedPointer() : ptr(nullptr), control_block(nullptr) {}

  template<typename _Ty>
  SharedPointer(_Ty *pointer) {
    if (nullptr == pointer) {
      *this(SharedPinter());
    } else {
      // enable_shared_from_this
    }
  }
  
  SharedPointer(const WeakPointer<_T> &weak_pointer) {
    // TODO
  }

  // copy constructor
  template <typename _Ty,
            typename = std::enable_if_t<std::is_base_of<_T, _Ty>::value> ||
                       std::is_same_v<_T, _Ty>>
  SharedPointer(const SharedPointer &pointer) {
    this->ptr = pointer.ptr;
    this->control_block = pointer.control_block;
    this->control_block->IncRef();
  }

  // copy assignment
  operator=(const SharedPointer& pointer) {
    if (pointer.Expire()) {

    }
  }

  long UseCount() const {
    if (nullptr != this->control_block) {
      return this->control_block->UseCount();
    }
  }

 private:
  _T *ptr;
  ControlBlockBase *control_block;
};

template<typename _T>
class WeakPointer {
 public:


  bool Expire() const {
    // TODO
  }

 private:
  _T *ptr;
  ControlBlockBase *cb;
};

}  // namespace tinysmartpointer
#endif