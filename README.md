# TinySmartPointer 🧠

一个轻量级C++智能指针实现库，包含`UniquePointer`、`SharedPointer`和`WeakPointer`的核心功能实现，支持C++20及以上标准。

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

## ✨ 特性概览

- **现代智能指针全家桶**
  - `UniquePointer`：独占所有权，支持自定义删除器
  - `SharedPointer`：引用计数共享所有权
  - `WeakPointer`：打破循环引用的观察指针

- **性能优化**
  - `UniquePointer`空基类优化（EBCO），消除无状态删除器的额外存储开销
  - `MakeShared`单次内存分配优化（控制块与对象内存合并）

- **进阶功能**
  - `MakeUnique`函数支持
  - `enable_shared_from_this`基类支持
  - 原子引用计数操作（基于`std::atomic`）
  - 支持自定义删除器（支持函数指针/仿函数）

## 📦 项目结构
```
.
├── include/
│ ├── SharedPointer.hpp # SharedPointer/WeakPointer及相关模板类
│ └── UniquePointer.hpp # UniquePointer及相关模板类
├── unitest/
│ └── unitest.cpp # Google-Test单元测试用例
├── CMakeLists.txt # 项目构建文件
├── README.md # 文档文件
├── run_test.sh # 编译并运行单元测试脚本
└── LICENSE # MIT许可证
```

## 🛠 构建与测试

**环境要求**：CMake 3.10+, 支持C++20的编译器

```bash
./run_test.sh
```

## 🚧 开发状态
当前为开发中版本，已实现的部分功能：
1. UniquePointer基础功能
2. MakeUnique函数
3. UniquePointer部分测试用例


## 🤝 参与贡献
欢迎通过Issue提交建议或Pull Request参与改进！请确保：
- 代码风格与现有代码保持一致
- 新增功能需附带测试用例
- 通过所有现有单元测试