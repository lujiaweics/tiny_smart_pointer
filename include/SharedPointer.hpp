#ifndef SharedPointer_HPP_
#define SharedPointer_HPP_

#include <atomic>

namespace tinysmartpointer {

template <typename T>
class WeakPointer;

class Defalut_Delete {
 public:
  void operator()(void *ptr) { delete ptr; }
};

template <typename T>
class Enable_share_from_this {
 public:
  // TODO
  void share_from_this() {}

 private:
  WeakPointer<T> weak_ptr;
};

class ControlBlockBase {
 public:
  ControlBlockBase() : weak_count{0}, shared_count{0} {};

  virtual ~ControlBlockBase() = 0;

  long UseCount() const { return static_cast<int>(this->shared_count); }

 private:
  std::atomic<int> weak_count;
  std::atomic<int> shared_count;
};

template <typename T, typename Deleter = Defalut_Delete>
class ControlBlockImpl : public ControlBlockBase {
 public:
 private:
  T *ptr;
  [[no_unique_address]] Deleter _Del;
};

template <typename T>
class SharedPointer {
 public:
  // constructor
  constexpr SharedPointer() : ptr(nullptr), control_block(nullptr) {}

  constexpr SharedPointer(std::nullptr_t) : ptr(nullptr), control_block(nullptr) {}

  template <typename Y, typename = std::is_convertible_v<Y, T> && !std::is_base_of_v<Y, Enable_share_from_this>>
  explicit SharedPointer(Y *ptr) {
    if (nullptr != ptr) {
      this->ptr = ptr;
      this->control_block = new ControlBlockImpl<T>(ptr);
    } else {
      this->ptr = nullptr;
      this->control_block = nullptr;
    }
  }

  template <typename Y>
  SharedPointer(Y *pointer) {
    if (nullptr == pointer) {
      *this(SharedPinter());
    } else {
      // enable_shared_fromThis
    }
  }

  SharedPointer(const WeakPointer<T> &weak_pointer) {
    // TODO
  }

  // copy constructor
  template <typename Y, typename = std::enable_if_t<std::is_base_of<T, Y>::value> || std::is_same_v<T, Y>>
  SharedPointer(const SharedPointer &pointer) {
    this->ptr = pointer.ptr;
    this->control_block = pointer.control_block;
    this->control_block->IncRef();
  }

  // copy assignment
  operator=(const SharedPointer &pointer) {
    if (pointer.Expire()) {
    }
  }

  long UseCount() const {
    if (nullptr != this->control_block) {
      return this->control_block->UseCount();
    }
  }

 private:
  T *ptr;
  ControlBlockBase *control_block;
};

template <typename T>
class WeakPointer {
 public:
  bool Expire() const {
    // TODO
  }

 private:
  T *ptr;
  ControlBlockBase *cb;
};

}  // namespace tinysmartpointer
#endif