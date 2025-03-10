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
  SharedPointer<int>();
  SharedPointer<int>(nullptr);
  SharedPointer<Base>(new Derive());
  SharedPointer<Base>(static_cast<Derive*>(nullptr));
  SharedPointer<SharedFromThis>(new SharedFromThis());
  SharedPointer<SharedFromThis>(static_cast<SharedFromThis*>(nullptr));
  SharedPointer<Base>(new Derive(), Deleter<Derive>());
  SharedPointer<Base>(static_cast<Derive*>(nullptr), Deleter<Derive>());
  SharedPointer<SharedFromThis>(new SharedFromThis(), Deleter<Derive>());
  SharedPointer<SharedFromThis>(static_cast<SharedFromThis*>(nullptr), Deleter<Derive>());

  {
    using Pair = std::pair<int, int>;
    SharedPointer<std::pair<int, int>> pair_pointer(new Pair(4, 5));
    SharedPointer<std::pair<int, int>> pair_pointer_copy(pair_pointer);
    SharedPointer<int> pair_to_second(pair_pointer, &(pair_pointer->second));
    SharedPointer<int> pair_to_second2(std::move(pair_pointer), &(pair_pointer->second));
  }

  {
    SharedPointer<Derive> p1;
    SharedPointer<Base> p2(p1);
  }

  {
    SharedPointer<Derive> p1(new Derive());
    SharedPointer<Base> p2(p1);
    SharedPointer<Base>(std::move(p1));
    SharedPointer<Base>(std::move(p2));
  }
}

TEST(Copy_Constructor, SHARED_POINTER_TEST) {
  SharedPointer<int> p1;
  SharedPointer<int> p2;
  p1 = p1;
  p1 = p2;
}