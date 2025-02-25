#ifndef SharedPointer_HPP_
#define SharedPointer_HPP_

#include <atomic>
#include <typeinfo>

#include "UniquePointer.hpp"

namespace tinysmartpointer {

template <typename T>
class WeakPointer;

template <typename T>
class Defalut_Delete {
 public:
  void operator()(T *ptr) { delete ptr; }
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

  virtual ~ControlBlockBase() = default;

  long UseCount() const { return static_cast<long>(this->shared_count.load(std::memory_order_acquire)); }

  void IncRef() { this->shared_count.fetch_add(1, std::memory_order_release); }

  void DecRef() {
    if (1 == this->shared_count.fetch_add(-1, std::memory_order_release)) {
      delete this;
    }
  }

  virtual void *Get_deleter(const std::type_info &) = 0;

 private:
  std::atomic<int> weak_count;
  std::atomic<int> shared_count;
};

template <typename T, typename Deleter = Defalut_Delete<T>>
class ControlBlockImpl : public ControlBlockBase {
 public:
  ControlBlockImpl(T *p) : ptr(p), _Del(Defalut_Delete<T>()) {}
  ControlBlockImpl(T *p, Deleter d) : ptr(p), _Del(std::move(d)) {}

  void *Get_deleter(const std::type_info &type) { return type == typeid(Deleter) ? &_Del : 0; }

 private:
  T *ptr;
  [[no_unique_address]] Deleter _Del;
};

template <typename T>
class SharedPointer final {
 public:
  using element_type = T;

  template <typename>
  friend class SharedPointer;

  template <typename, typename>
  friend class UniquePointer;

  // constructor
  constexpr SharedPointer() : ptr(nullptr), control_block(nullptr) {}

  constexpr SharedPointer(std::nullptr_t) : ptr(nullptr), control_block(nullptr) {}

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y, T> &&
                                                    !std::is_base_of_v<Y, Enable_share_from_this<T>>>>
  explicit SharedPointer(Y *ptr) {
    if (nullptr != ptr) {
      this->ptr = ptr;
      this->control_block = new ControlBlockImpl<T>(ptr);
      this->control_block->IncRef();
    } else {
      this->ptr = nullptr;
      this->control_block = nullptr;
    }
  }

  template <typename Y,
            typename = std::enable_if_t<std::is_convertible_v<Y, T> && std::is_base_of_v<Y, Enable_share_from_this<T>>>,
            int = 0>
  explicit SharedPointer(Y *ptr) {
    if (nullptr == ptr) {
      this->ptr = ptr;
      this->control_block = nullptr;
    } else {
      // enable_shared_from_this
    }
  }

  template <typename Y, typename Deleter,
            typename = std::enable_if_t<std::is_convertible_v<Y, T> && !std::is_base_of_v<Y, Enable_share_from_this>>>
  SharedPointer(Y *ptr, Deleter d) {
    if (nullptr != ptr) {
      this->ptr = ptr;
      this->control_block = new ControlBlockImpl<T, Deleter>(ptr, d);
      this->control_block->IncRef();
    } else {
      this->ptr = nullptr;
      this->control_block = nullptr;
    }
  }

  template <typename Y, typename Deleter,
            typename = std::enable_if_t<std::is_convertible_v<Y, T> && !std::is_base_of_v<Y, Enable_share_from_this>>,
            int = 0>
  SharedPointer(Y *pointer, Deleter d) {
    if (nullptr == pointer) {
      this->ptr = ptr;
      this->control_block = nullptr;
    } else {
      // enable_shared_from_this
    }
  }

  template <typename Y>
  SharedPointer(const SharedPointer<Y> &r, T *ptr) {
    this->ptr = ptr;
    this->control_block = r.control_block;
    this->control_block->IncRef();
  }

  template <typename Y>
  SharedPointer(const SharedPointer<Y> &&r, T *ptr) {
    this->ptr = ptr;
    this->control_block = r.control_block;
    r.control_block = nullptr;
    r.ptr = nullptr;
  }

  SharedPointer(const SharedPointer &r) {
    this->ptr = r.ptr;
    this->control_block = r.control_block;
    this->control_block->IncRef();
  }

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y, T>>>
  SharedPointer(const SharedPointer<Y> &r) {
    this->ptr = r.ptr;
    this->control_block = r.control_block;
    this->control_block->IncRef();
  }

  SharedPointer(const SharedPointer &&r) {
    this->ptr = r.ptr;
    this->control_block = r.control_block;
    r.control_block = nullptr;
    r.ptr = nullptr;
  }

  template <typename Y>
  SharedPointer(const SharedPointer<Y> &&r) {
    this->ptr = r.ptr;
    this->control_block = r.control_block;
    r.control_block = nullptr;
    r.ptr = nullptr;
  }

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y, T>>>
  SharedPointer(const WeakPointer<Y> &weak_pointer) {
    weak_pointer.lock();  // may throw bad_weak_ptr()
    // TODO
  }

  template <typename Y, typename Deleter, typename = std::enable_if_t<std::is_convertible_v<Y, T>>>
  SharedPointer(const UniquePointer<Y, Deleter> &&r) {
    this->ptr = r.Get();
    this->control_block = new ControlBlockImpl<Y, Deleter>(this->ptr, r.Get_deleter());
    this->control_block->IncRef();
    std::swap(r, UniquePointer<Y, Deleter>());
  }

  ~SharedPointer() {
    this->ptr = nullptr;
    this->control_block->DecRef();
  }

  // copy assignment
  SharedPointer &operator=(const SharedPointer &r) {
    if (&r == this) {
      return *this;
    }

    this->ptr = r.ptr;
    this->control_block->DecRef();
    this->control_block = r.control_block;
    this->control_block->IncRef();
  }

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y, T>>>
  SharedPointer &operator=(const SharedPointer<Y> &r) {
    if (&r == this) {
      return *this;
    }

    this->ptr = r.ptr;
    this->control_block->DecRef();
    this->control_block = r.control_block;
    this->control_block->IncRef();
  }

  SharedPointer &operator=(SharedPointer &&r) {
    if (&r == this) {
      return *this;
    }

    this->ptr = r.ptr;
    r.ptr = nullptr;
    this->control_block = r.control_block;
    r.control_block = nullptr;
  }

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y, T>>>
  SharedPointer &operator=(SharedPointer<Y> &&r) {
    if (&r == this) {
      return *this;
    }

    this->ptr = r.ptr;
    r.ptr = nullptr;
    this->control_block = r.control_block;
    r.control_block = nullptr;
  }

  template <typename Y, typename Deleter, typename = std::enable_if_t<std::is_convertible_v<Y, T>>>
  SharedPointer &operator=(UniquePointer<Y, Deleter> &&r) {
    this->ptr = r.Release();
    this->control_block = new ControlBlockImpl<Y, Deleter>(this->ptr, r.Get_deleter());
    this->control_block->IncRef();
  }

  void Reset() {
    this->ptr = nullptr;
    this->control_block->DecRef();
  }

  template <typename Y>
  void Reset(Y *ptr) {
    this->Swap(SharePointer(ptr));
  }

  template <typename Y, typename Deleter>
  void Reset(Y *ptr, Deleter d) {
    this->swap(SharedPointer(ptr, d));
  }

  void Swap(SharedPointer &r) {
    std::swap(this->ptr, r.ptr);
    std::swap(this->control_block, r.control_block);
  }

  T *Get() const { return this->ptr; }

  T &operator*() const {
    assert(nullptr != this->ptr);  // dangerous operator
    return *(this->ptr);
  }

  T *operator->() const { return &(this->operator*()); }

  long UseCount() const {
    if (nullptr != this->control_block) {
      return this->control_block->UseCount();
    }
  }

  bool Unique() const { return (1 == this->UseCount()); }

  explicit operator bool() { return nullptr != this->Get(); }

  // TODO: non-member function, or if null
  template <typename Deleter>
  Deleter *Get_deleter() {
    if (this->ptr) {
      return nullptr;
    } else {
      return static_cast<Deleter *>(this->control_block->Get_deleter(typeid(Deleter)));
    }
  }

 private:
  T *ptr;
  ControlBlockBase *control_block;
};

template <typename T>
class WeakPointer final {
 public:
  using element_type = T;

  template <typename>
  friend class SharedPointer;

  template <typename>
  friend class WeakPointer;

  constexpr WeakPointer() : ptr(nullptr), cb(nullptr) {}

  WeakPointer(const WeakPointer &r) : {
    if (nullptr != r.control_block) {
      r.control_block->IncWeakRef();
      this->control_block = r.control_block;
      this->ptr = r.ptr;
    } else {
      this->control_block = nullptr;
      this->ptr = nullptr;
    }
  }

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y, T>>>
  WeakPointer(const WeakPointer<Y> &r) {
    if (nullptr != r.control_block) {
      r.control_block->IncWeakRef();
      this->control_block = r.control_block;
      this->ptr = r.ptr;
    } else {
      this->control_block = nullptr;
      this->ptr = nullptr;
    }
  }

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y, T>>>
  WeakPointer(const SharedPointer<Y> &r) {
    if (nullptr != r.control_block) {
      r.control_block->IncWeakRef();
      this->control_block = r.control_block;
      this->ptr = r.ptr;
    } else {
      this->control_block = nullptr;
      this->ptr = nullptr;
    }
  }

  WeakPointer(WeakPointer &&r) {
    this->control_block = r.control_block;
    this->ptr = r.ptr;
    r.ptr = nullptr;
    r.control_block = nullptr;
  }

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y, T>>>
  WeakPointer(WeakPointer<Y> &&r) {
    this->control_block = r.control_block;
    this->ptr = r.ptr;
    r.ptr = nullptr;
    r.control_block = nullptr;
  }

  bool Expire() const {
    // TODO
  }

 private:
  T *ptr;
  ControlBlockBase *control_block;
};

}  // namespace tinysmartpointer
#endif