/// @file SharedPointer.hpp
/// @brief 实现引用计数智能指针，弱引用计数智能指针及相关工具类
/// @details 包含以下核心组件：
/// - SharedPointer: 线程安全的共享所有权智能指针
/// - WeakPointer: 观察者指针，解决循环引用问题
/// - ControlBlock: 引用计数管理工具类
/// - Enable_shared_from_this
/// @author Garvey
/// @date 2025-03-17

#ifndef SharedPointer_HPP_
#define SharedPointer_HPP_

#include <atomic>
#include <typeinfo>

#include "UniquePointer.hpp"

namespace TinySmartPointer {

// 前向声明
template <typename T>
class WeakPointer;

template <typename T>
class SharedPointer;

template <typename T>
class Enable_shared_from_this;

/// @brief 默认删除器类模板
/// @tparam T 要删除的对象类型
template <typename T>
class Defalut_Delete {
 public:
  /// @brief 执行删除操作
  /// @param ptr 要删除的指针
  void operator()(T *ptr) { delete ptr; }
};

/// @brief 控制块基类（模板类型擦除）
class ControlBlockBase {
 public:
  /// @brief 构造时初始化引用计数
  ControlBlockBase() : weak_count{0}, shared_count{0} {};

  /// @brief 虚析构函数保证正确释放派生类资源
  virtual ~ControlBlockBase() = default;

  /// @brief 获取当前共享引用计数
  /// @return 当前共享引用数量
  long UseCount() const { return static_cast<long>(this->shared_count.load(std::memory_order_acquire)); }

  /// @brief 获取弱引用计数
  /// @return 当前弱引用数量
  long GetWeakCount() const { return static_cast<long>(this->weak_count.load(std::memory_order_acquire)); }

  /// @brief 增加强引用计数
  void IncRef() { this->shared_count.fetch_add(1, std::memory_order_relaxed); }

  /// @brief 减少强引用计数
  /// @note 当引用归零时触发资源释放：
  /// - 如果弱引用也为零，删除控制块
  /// - 否则保持控制块存活供WeakPointer使用
  void DecRef() {
    auto now_cnt = this->shared_count.fetch_add(-1, std::memory_order_acq_rel) - 1;
    if (0 == now_cnt && 0 == this->GetWeakCount()) {
      this->Dispose();
      delete this;
    } else if (0 == now_cnt) {
      this->Dispose();
    }
  }

  /// @brief 增加弱引用计数
  void IncWeakRef() { this->weak_count.fetch_add(1, std::memory_order_relaxed); }

  /// @brief 减少弱引用计数
  /// @note 当弱引用归零且共享引用为零时删除控制块
  void DecWeakRef() {
    if (1 == this->weak_count.fetch_add(-1, std::memory_order_acq_rel) && 0 == this->UseCount()) {
      delete this;
    }
  }

  /// @brief 获取删除器（接口）
  /// @param type 要查询的删除器类型信息
  /// @return 匹配的删除器指针，未找到返回nullptr
  virtual void *GetDeleter(const std::type_info &) = 0;

  /// @brief 资源释放接口
  virtual void Dispose() {}

 private:
  std::atomic<int> weak_count;    ///< 弱引用计数器
  std::atomic<int> shared_count;  ///< 共享引用计数器
};

/// @brief 控制块实现类模板
/// @tparam T 管理的对象类型
/// @tparam Deleter 删除器类型（默认使用Defalut_Delete）
template <typename T, typename Deleter = Defalut_Delete<T>>
class ControlBlockImpl : public ControlBlockBase {
 public:
  /// @brief 构造函数（使用默认删除器）
  /// @param p 要管理的原始指针
  ControlBlockImpl(T *p) : ptr(p), del(Defalut_Delete<T>()) {}

  // @brief 构造函数（自定义删除器）
  /// @param p 要管理的原始指针
  /// @param d 删除器对象（右值引用）
  ControlBlockImpl(T *p, Deleter &&d) : ptr(p), del(std::forward<Deleter>(d)) {}

  /// @brief 获取删除器（实现）
  /// @param type 目标类型信息
  /// @return 若与传入的类型匹配，则返回删除器指针，否则返回nullptr
  Deleter *GetDeleter(const std::type_info &type) { return type == typeid(Deleter) ? &del : 0; }

  /// @brief 执行资源释放
  /// @note 调用存储的删除器释放资源
  void Dispose() override { del(ptr); }

 private:
  T *ptr;                             ///< 管理的原始指针
  [[no_unique_address]] Deleter del;  ///< 删除器实例（空基类优化）
};

/// @brief 共享指针类模板
/// @tparam T 管理的对象类型
/// @details 实现引用计数智能指针，支持：
/// - 线程安全引用计数
/// - 自定义删除器
/// - 与WeakPointer交互
/// - 支持enable_shared_from_this
template <typename T>
class SharedPointer final {
 public:
  using element_type = T;  ///< 元素类型别名

  template <typename>
  friend class SharedPointer;

  template <typename>
  friend class WeakPointer;

  template <typename, typename>
  friend class UniquePointer;

  /// @brief 默认构造函数（空指针）
  constexpr SharedPointer() : ptr(nullptr), control_block(nullptr) {}

  /// @brief 空指针构造
  /// @param nullptr_t 空指针标记
  constexpr SharedPointer(std::nullptr_t) : ptr(nullptr), control_block(nullptr) {}

  /// @brief 从原始指针构造（支持多态）
  /// @tparam Y 指向的数据类型
  /// @param ptr 托管的指针
  /// @note 启用条件：
  /// - Y可转换为T
  /// - 不继承enable_shared_from_this
  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y, T> &&
                                                    !std::is_base_of_v<Enable_shared_from_this<Y>, Y>>>
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

  /// @brief 从继承自Enable_shared_from_this的裸指针构造（支持多态）
  /// @tparam Y 指向的数据类型（需继承Enable_shared_from_this）
  /// @param ptr 托管的裸指针
  template <
      typename Y,
      typename = std::enable_if_t<std::is_convertible_v<Y, T> && std::is_base_of_v<Enable_shared_from_this<Y>, Y>>,
      int = 0>
  explicit SharedPointer(Y *ptr) {
    if (nullptr == ptr) {
      this->ptr = ptr;
      this->control_block = nullptr;
    } else {
      // enable_shared_from_this
      this->ptr = ptr;
      this->control_block = new ControlBlockImpl<T>(ptr);
      (static_cast<Enable_shared_from_this<T> *>(ptr))->Assign(this->ptr, this->control_block);
      this->control_block->IncRef();
    }
  }

  /// @brief 构造带自定义删除器的共享指针（支持多态）
  /// @tparam Y 指向的数据类型
  /// @tparam Deleter 删除器类型
  /// @param ptr 托管的指针
  /// @param d 自定义删除器对象
  /// @note 启用条件：
  /// - Y可转换为T
  /// - 不继承enable_shared_from_this
  template <
      typename Y, typename Deleter,
      typename = std::enable_if_t<std::is_convertible_v<Y, T> && !std::is_base_of_v<Enable_shared_from_this<Y>, Y>>>
  SharedPointer(Y *ptr, Deleter &&d) {
    this->ptr = ptr;
    this->control_block = new ControlBlockImpl<T, Deleter>(ptr, std::forward<Deleter>(d));
    this->control_block->IncRef();
  }

  /// @brief 从继承自Enable_shared_from_this的裸指针和自定义删除器构造（支持多态）
  /// @tparam Y 指向的数据类型（需继承Enable_shared_from_this）
  /// @tparam Deleter 删除器类型
  /// @param ptr 托管的裸指针
  /// @param d 自定义删除器对象
  template <
      typename Y, typename Deleter,
      typename = std::enable_if_t<std::is_convertible_v<Y, T> && std::is_base_of_v<Enable_shared_from_this<Y>, Y>>,
      int = 0>
  SharedPointer(Y *ptr, Deleter &&d) {
    if (nullptr == ptr) {
      this->ptr = nullptr;
      this->control_block = nullptr;
    } else {
      this->ptr = ptr;
      this->control_block = new ControlBlockImpl<T, Deleter>(ptr, std::forward<Deleter>(d));
      (static_cast<Enable_shared_from_this<T> *>(ptr))->Assign(this->ptr, this->control_block);
      this->control_block->IncRef();
    }
  }
  /// @brief 别名使用构造函数
  /// @note 构造 SharedPointer，与r的初始值共享所有权信息，但保有无关且不管理的指针ptr
  template <typename Y>
  SharedPointer(const SharedPointer<Y> &r, T *ptr) {
    this->ptr = ptr;
    this->control_block = r.control_block;
    this->control_block->IncRef();
  }

  /// @brief 别名使用移动构造函数
  /// @note 从r移动构造 Shared_ptr，但保有无关且不管理的指针 ptr
  template <typename Y>
  SharedPointer(SharedPointer<Y> &&r, T *ptr) {
    this->ptr = ptr;
    this->control_block = r.control_block;
    r.control_block = nullptr;
    r.ptr = nullptr;
  }

  /// @brief 拷贝构造函数
  /// @param r 源SharedPointer
  SharedPointer(const SharedPointer &r) {
    this->ptr = r.ptr;
    this->control_block = r.control_block;
    this->control_block->IncRef();
  }

  /// @brief 模板拷贝构造函数（支持多态）
  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y, T>>>
  SharedPointer(const SharedPointer<Y> &r) {
    this->ptr = r.ptr;
    this->control_block = r.control_block;
    if (nullptr != this->control_block) {
      this->control_block->IncRef();
    }
  }

  /// @brief 移动构造函数
  /// @param r 源右值
  SharedPointer(SharedPointer &&r) {
    this->ptr = r.ptr;
    this->control_block = r.control_block;
    r.control_block = nullptr;
    r.ptr = nullptr;
  }

  /// @brief 移动构造函数（支持多态）
  /// @param r 源右值
  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y, T>>>
  SharedPointer(SharedPointer<Y> &&r) {
    this->ptr = static_cast<T *>(r.ptr);
    this->control_block = r.control_block;
    r.control_block = nullptr;
    r.ptr = nullptr;
  }

  /// @brief 从弱指针构造（可能抛出异常）
  /// @throws 当弱指针已失效时抛出异常
  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y, T>>>
  SharedPointer(const WeakPointer<Y> &weak_pointer) {  // may throw bad_weak_ptr()
    SharedPointer shared_pointer = weak_pointer.Lock();
    if (!shared_pointer) {
      throw "bad_weak_ptr";
    }
    *this = std::move(shared_pointer);
  }

  /// @brief 从UniquePointer移动构造
  /// @tparam Y 源指针类型
  /// @tparam Deleter 源删除器类型
  /// @param r 源UniquePointer（右值）
  /// @post 转移所有权后r变为空指针
  template <typename Y, typename Deleter, typename = std::enable_if_t<std::is_convertible_v<Y, T>>>
  SharedPointer(UniquePointer<Y, Deleter> &&r) {
    this->ptr = r.Get();
    this->control_block = new ControlBlockImpl<Y, Deleter>(this->ptr, *(r.GetDeleter()));
    this->control_block->IncRef();
    std::swap(r, UniquePointer<Y, Deleter>());
  }

  /// @brief 析构函数
  ~SharedPointer() {
    this->ptr = nullptr;
    if (nullptr != this->control_block) {
      this->control_block->DecRef();
    }
  }

  /// @brief 拷贝赋值
  SharedPointer &operator=(const SharedPointer &r) {
    if (&r == this) {
      return *this;
    }

    this->ptr = r.ptr;
    if (nullptr != this->control_block) {
      this->control_block->DecRef();
    }
    this->control_block = r.control_block;
    if (nullptr != this->control_block) {
      this->control_block->IncRef();
    }

    return *this;
  }

  /// @brief 拷贝赋值（支持多态）
  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y, T>>>
  SharedPointer &operator=(const SharedPointer<Y> &r) {
    if (reinterpret_cast<const void *>(&r) == reinterpret_cast<void *>(this)) {
      return *this;
    }

    this->ptr = r.ptr;
    if (nullptr != this->control_block) {
      this->control_block->DecRef();
    }
    this->control_block = r.control_block;
    if (nullptr != this->control_block) {
      this->control_block->IncRef();
    }

    return *this;
  }

  /// @brief 移动赋值
  SharedPointer &operator=(SharedPointer &&r) {
    if (&r == this) {
      return *this;
    }

    SharedPointer(std::move(r)).Swap(*this);
    return *this;
  }

  /// @brief 移动赋值（支持多态）
  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y, T>>>
  SharedPointer &operator=(SharedPointer<Y> &&r) {
    if (reinterpret_cast<const void *>(&r) == reinterpret_cast<void *>(this)) {
      return *this;
    }

    this->ptr = r.ptr;
    r.ptr = nullptr;
    if (nullptr != this->control_block) {
      this->control_block->DecRef();
    }
    this->control_block = r.control_block;
    r.control_block = nullptr;

    return *this;
  }

  /// @brief 含删除器的移动赋值（支持多态）
  template <typename Y, typename Deleter, typename = std::enable_if_t<std::is_convertible_v<Y, T>>>
  SharedPointer &operator=(UniquePointer<Y, Deleter> &&r) {
    this->ptr = r.Release();
    this->control_block = new ControlBlockImpl<Y, Deleter>(this->ptr, *(r.GetDeleter()));
    this->control_block->IncRef();
  }

  /// @brief 重置指针，并释放指向数据的引用
  void Reset() {
    this->ptr = nullptr;
    if (nullptr != this->control_block) {
      this->control_block->DecRef();
      this->control_block = nullptr;
    }
  }

  /// @brief 接管新指针的所有权，并释放对原指针的引用
  /// @tparam Y 新指针类型
  /// @param ptr 新托管的裸指针
  template <typename Y>
  void Reset(Y *ptr) {
    SharedPointer tmp_sp(ptr);
    this->Swap(tmp_sp);
  }

  /// @brief 接管新指针，指定删除器，并释放对原指针的引用
  /// @tparam Y 新指针类型
  /// @tparam Deleter 删除器类型
  /// @param ptr 新托管的裸指针
  /// @param d 自定义删除器
  template <typename Y, typename Deleter>
  void Reset(Y *ptr, Deleter &&d) {
    SharedPointer tmp_sp(ptr, std::forward<Deleter>(d));
    this->Swap(tmp_sp);
  }

  /// @brief 交换两个共享指针
  void Swap(SharedPointer &r) {
    std::swap(this->ptr, r.ptr);
    std::swap(this->control_block, r.control_block);
  }

  /// @brief 获取管理对象的指针
  /// @return 原始指针
  T *Get() const { return this->ptr; }

  /// @brief 解引用运算符
  /// @warning 对空指针调用导致未定义行为
  T &operator*() const {
    assert(nullptr != this->ptr);
    return *(this->ptr);
  }

  /// @brief 成员访问运算符
  /// @return 管理对象的指针
  T *operator->() const { return &(this->operator*()); }

  /// @brief 获取当前引用计数
  /// @return 共享引用数量
  long UseCount() const {
    if (nullptr != this->control_block) {
      return this->control_block->UseCount();
    } else {
      return 0l;
    }
  }

  /// @brief 检查是否唯一所有者
  bool Unique() const { return (1 == this->UseCount()); }

  /// @brief 向布尔类型的转换
  explicit operator bool() { return nullptr != this->Get(); }

  /// @brief 获取特定类型的删除器
  /// @return 若存在匹配的删除器则返回指针，否则返回nullptr
  template <typename Deleter>
  Deleter *GetDeleter() {
    if (nullptr == this->ptr || nullptr == this->control_block) {
      return nullptr;
    } else {
      return static_cast<Deleter *>(this->control_block->GetDeleter(typeid(Deleter)));
    }
  }

 private:
  T *ptr;                           ///< 管理的原始指针
  ControlBlockBase *control_block;  ///< 控制块指针

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y, T>>>
  SharedPointer(const WeakPointer<Y> &weak_pointer, std::nothrow_t) : control_block(weak_pointer.control_block) {
    if (nullptr != this->control_block && 0 != this->control_block->UseCount()) {
      this->ptr = weak_pointer.ptr;
      this->control_block->IncRef();
    } else {
      this->ptr = nullptr;
      this->control_block = nullptr;
    }
  }
};

/// @brief 弱引用计数的智能指针类
/// @tparam T 观察指针的类型
template <typename T>
class WeakPointer final {
 public:
  using element_type = T;

  template <typename>
  friend class SharedPointer;

  template <typename>
  friend class WeakPointer;

  template <typename>
  friend class Enable_shared_from_this;

  constexpr WeakPointer() : ptr(nullptr), control_block(nullptr) {}

  WeakPointer(const WeakPointer &r) {
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

  ~WeakPointer() {
    if (nullptr != this->control_block) {
      this->control_block->DecWeakRef();
    }
  }

  WeakPointer &operator=(const WeakPointer &r) {
    WeakPointer(r).Swap(*this);
    return *this;
  }

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y, T>>>
  WeakPointer &operator=(const WeakPointer<Y> &r) {
    WeakPointer(r).Swap(*this);
    return *this;
  }

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y, T>>>
  WeakPointer &operator=(const SharedPointer<Y> &r) {
    WeakPointer(r).Swap(*this);
    return *this;
  }

  WeakPointer &operator=(WeakPointer &&r) {
    WeakPointer(std::move(r)).Swap(*this);
    return *this;
  }

  template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y, T>>>
  WeakPointer &operator=(WeakPointer<Y> &&r) {
    WeakPointer(std::move(r)).Swap(*this);
    return *this;
  }

  void Swap(WeakPointer &r) {
    T *tmp_ptr = r.ptr;
    r.ptr = this->ptr;
    this->ptr = tmp_ptr;
    ControlBlockBase *tmp_cb_ptr = r.control_block;
    r.control_block = this->control_block;
    this->control_block = tmp_cb_ptr;
  }

  void Reset() { WeakPointer().Swap(*this); }

  long UseCount() const {
    if (nullptr == this->control_block) {
      return 0;
    } else {
      return this->control_block->UseCount();
    }
  }

  bool Expire() const { return 0 == this->UseCount(); }

  /// @brief 尝试升级为共享指针
  /// @return 如果对象仍然存在则返回有效共享指针，否则返回空指针
  SharedPointer<T> Lock() const { return SharedPointer<T>(*this, std::nothrow); }

 private:
  T *ptr;
  ControlBlockBase *control_block;
};

/// @brief 支持从this获取共享指针的工具类
/// @tparam T 指向对象的类型
template <typename T>
class Enable_shared_from_this {
 public:
  template <typename>
  friend class SharedPointer;

  /// @brief 获取当前对象的共享指针
  /// @return 管理当前对象的共享指针，若SharedPointer已失效，则返回空的SharedPointer，否则创建新的SharedPointer指向数据
  SharedPointer<T> shared_from_this() { return SharedPointer<T>(this->weak_ptr); }

 private:
  void Assign(T *ptr, ControlBlockBase *control_block) {
    if (0 == control_block->UseCount()) {
      this->weak_ptr.ptr = ptr;
      this->weak_ptr.control_block = control_block;
    }
  }
  WeakPointer<T> weak_ptr;
};

}  // namespace TinySmartPointer
#endif