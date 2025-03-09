#include <gtest/gtest.h>

#include <iostream>

#include "../include/SharedPointer.hpp"
#include "../include/UniquePointer.hpp"
#include "common/utility.hpp"

using TinySmartPointer::MakeUnique;
using TinySmartPointer::SharedPointer;
using TinySmartPointer::UniquePointer;
template class SharedPointer<double>;

TEST(Constructor, SHARED_POINTER_TEST) {
  SharedPointer<int> p1;
  SharedPointer<int> p2(nullptr);
  SharedPointer<Base> p3(new Derive());
  SharedPointer<SharedFromThis> p4(new SharedFromThis());

  // using Pair = std::pair<int, int>;
  // SharedPointer<std::pair<int, int>> pair_pointer(new Pair(4, 5));
  // SharedPointer<int> pair_to_second(pair_pointer, &(pair_pointer->second));
  // EXPECT_EQ(*pair_to_second, 5);
}