/// @file UniquePointer.hpp
/// @brief 实现独占所有权智能指针及相关工具类
/// @details 包含以下核心组件：
/// - UniquePointer: 独占所有权智能指针
/// - MakeUnique: 构造UniquePointer的工具函数
/// @author Garvey
/// @date 2025-03-17

#ifndef UNIQUEPOINTER_HPP_
#define UNIQUEPOINTER_HPP_

#include <cassert>
#include <iostream>
#include <memory>
#include <type_traits>

namespace TinySmartPointer {

/// @brief 独占指针类模板
/// @tparam T 管理的对象类型
/// @tparam Deleter 删除器类型，默认为std::default_delete<T>
/// @note 该类不可拷贝，支持移动语义，符合RAII原则
template <typename T, typename Deleter = std::default_delete<T>>
class UniquePointer {
 public:
  /// @brief 删除器类型别名（经过decay处理的类型）
  using DecayedDeleter = typename std::decay_t<Deleter>;

  /// @brief 构造并接管指针所有权（带自定义删除器）
  /// @tparam DeleterType 删除器类型
  /// @param pointer 要管理的原始指针
  /// @param deleter 删除器对象（右值引用）
  template <typename DeleterType>
  constexpr UniquePointer(T* pointer, DeleterType&& deleter)
      : pointer_(static_cast<T*>(pointer)), deleter_(std::forward<DeleterType>(deleter)) {}

  /// @brief 构造并接管指针所有权（使用默认删除器）
  /// @param pointer 要管理的原始指针
  explicit constexpr UniquePointer(T* pointer)
      : pointer_(static_cast<T*>(pointer)), deleter_(std::move(std::default_delete<T>())) {}

  /// @brief 默认构造函数（空指针）
  constexpr UniquePointer() : pointer_(nullptr) ,deleter_(std::move(std::default_delete<T>())){}

  /// @brief 析构函数
  /// @note 自动调用删除器释放资源
  ~UniquePointer() {
    if (nullptr != pointer_) {
      deleter_(pointer_);
      pointer_ = nullptr;
    }
  }

  // 禁用拷贝构造
  UniquePointer(const UniquePointer&) = delete;

  // 禁用拷贝赋值
  UniquePointer& operator=(const UniquePointer&) = delete;

  /// @brief 移动构造函数
  /// @param unique_pointer 源右值对象
  UniquePointer(UniquePointer&& unique_pointer) { this->Reset(unique_pointer.Release()); }

  /// @brief 派生类移动构造函数
  /// @tparam DerivedType 派生类类型
  /// @param unique_pointer 源派生类对象
  /// @note 使用SFINAE确保只有基类-派生类关系才能调用
  template <typename DerivedType, typename = std::enable_if_t<std::is_base_of<T, DerivedType>::value>>
  UniquePointer(UniquePointer<DerivedType>&& unique_pointer)
      : pointer_(unique_pointer.Release()), deleter_(unique_pointer.Get_deleter()) {}

  /// @brief 移动赋值运算符
  /// @tparam DerivedType 派生类类型
  /// @param rhs 右值源对象
  /// @return 当前对象的引用
  template <typename DerivedType, typename = std::enable_if<std::is_base_of<T, DerivedType>::value>>
  UniquePointer& operator=(UniquePointer<DerivedType>&& rhs) {
    if (this == &rhs) {
      return *this;
    }
    if (nullptr != this->pointer_) {
      deleter_(this->pointer_);
    }
    this->pointer_ = rhs.Release();
    this->deleter_ = rhs.Get_deleter();
    return *this;
  }

  /// @brief 解引用运算符
  /// @return 管理对象的引用
  /// @warning 对空指针解引用会导致断言失败
  T& operator*() const {
    assert(nullptr != this->pointer_);
    return *(this->pointer_);
  }

  /// @brief 成员访问运算符
  /// @return 管理对象的指针
  T* operator->() const { return &(this->operator*()); }

  /// @brief void*转换运算符
  /// @return 指针的void*表示
  /// @note 用于布尔上下文判断，空指针返回0的void*表示
  operator void*() const {
    return nullptr != this->pointer_ ? reinterpret_cast<void*>(this->pointer_) : reinterpret_cast<void*>(0);
  }

  /// @brief 显式布尔转换
  /// @return 是否持有有效指针
  explicit operator bool() const { return nullptr != this->Get(); }

  /// @brief 获取原始指针
  /// @return 管理的原始指针（不转移所有权）
  T* Get() const { return this->pointer_; }

  /// @brief 获取删除器（const版本）
  /// @return 当前删除器的const引用
  const DecayedDeleter& Get_deleter() const { return this->deleter_; }

  /// @brief 获取删除器（非const版本）
  /// @return 当前删除器的引用
  DecayedDeleter& Get_deleter() { return this->deleter_; }

  /// @brief 重置管理的指针
  /// @param new_pointer 新指针（默认为nullptr）
  /// @note 会先释放当前指针再接管新指针
  void Reset(T* new_pointer = nullptr) {
    if (this->pointer_ == new_pointer) {
      return;
    }
    std::swap(this->pointer_, new_pointer);
  }

  /// @brief 释放指针所有权
  /// @return 被释放的原始指针
  /// @note 调用后对象不再管理该指针
  T* Release() {
    T* dumb_pointer = this->pointer_;
    this->pointer_ = nullptr;
    return dumb_pointer;
  }

  /// @brief 交换两个UniquePointer
  /// @param other 另一个UniquePointer对象
  void Swap(UniquePointer& other) { std::swap(this->pointer_, other.pointer_); }

 private:
  T* pointer_;                                    ///< 管理的原始指针
  [[no_unique_address]] DecayedDeleter deleter_;  ///< 删除器实例
};

/// @brief 创建UniquePointer的工厂函数
/// @tparam T 要创建的对象类型
/// @tparam Ts 构造函数参数类型包
/// @param params 构造函数参数
/// @return 管理新对象的UniquePointer
template <typename T, typename... Ts>
UniquePointer<T> MakeUnique(Ts&&... params) {
  return UniquePointer<T>(new T(std::forward<Ts>(params)...));
}

}  // namespace TinySmartPointer

#endif