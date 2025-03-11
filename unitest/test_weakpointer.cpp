#include <gtest/gtest.h>

#include <iostream>

#include "../include/SharedPointer.hpp"
#include "../include/UniquePointer.hpp"
#include "common/utility.hpp"

using TinySmartPointer::MakeUnique;
using TinySmartPointer::SharedPointer;
using TinySmartPointer::UniquePointer;
using TinySmartPointer::WeakPointer;
template class WeakPointer<double>;

TEST(Constructor, WEAK_POINTER_TEST) {
  {
    WeakPointer<int> p1;
    WeakPointer<int> p2(p1);
  }

  {
    WeakPointer<Derive> p1;
    WeakPointer<Base> p2(p1);
  }

  {
    SharedPointer<Derive> p1(new Derive());
    WeakPointer<Derive> p2(p1);
  }
}