#ifndef UTILITY_HPP_
#define UTILITY_HPP_

#include "../../include/SharedPointer.hpp"

class Base {
 public:
  Base(int num = 0) : num_(num) {}
  void Increase() { ++num_; }
  void Derease() { --num_; }
  int GetNumber() { return num_; }
  void SetNumber(int new_number) { this->num_ = new_number; }
  virtual bool IsDerived() { return false; }

  virtual ~Base() {}

 private:
  int num_;
};

class Derive : public Base {
 public:
  bool IsDerived() override { return true; }

  Derive(int num = 0) { SetNumber(num); }
};

class SharedFromThis : public TinySmartPointer::Enable_shared_from_this<SharedFromThis> {};

template <typename T>
class Deleter {
 public:
  void operator()(T* pointer) { delete pointer; }
};

#endif