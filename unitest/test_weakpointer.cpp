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
    auto p3(p2);
    WeakPointer<Base> p4(p3);
    WeakPointer<Base>(SharedPointer<Derive>());
  }

  {
    WeakPointer<Base>(WeakPointer<Base>());
    WeakPointer<Base>(std::move(WeakPointer<Base>()));
  }
}

TEST(Assignment, WEAK_POINTER_TEST) {
  {
    WeakPointer<Derive> p1;
    WeakPointer<Derive> p2;
    p2 = p1;
    p2 = std::move(WeakPointer<Derive>());
  }

  {
    WeakPointer<Derive> p1;
    WeakPointer<Base> p2;
    p2 = p1;
    p2 = std::move(p1);
  }

  {
    SharedPointer<Derive> p1;
    WeakPointer<Base> p2;
    p2 = p1;
  }
}

TEST(Member_Function, WEAK_POINTER_TEST) {
  { WeakPointer<double>().Reset(); }

  {
    EXPECT_EQ(WeakPointer<double>().UseCount(), 0);
    SharedPointer<Derive> p1(new Derive());
    WeakPointer<Derive> p2(p1);
    EXPECT_EQ(p2.UseCount(), 1);
  }

  {
    EXPECT_TRUE(WeakPointer<double>().Expire());
    SharedPointer<Derive> p1(new Derive());
    WeakPointer<Derive> p2(p1);
    EXPECT_FALSE(p2.Expire());
    EXPECT_TRUE(static_cast<bool>(p2.Lock()));
    p1.Reset();
    EXPECT_TRUE(p2.Expire());
    EXPECT_FALSE(static_cast<bool>(p2.Lock()));
  }

  {
    SharedPointer<Derive> p1;
    WeakPointer<Derive> p2(p1);
    EXPECT_FALSE(static_cast<bool>(p2.Lock()));
  }

  {
    SharedPointer<Derive> p1(new Derive());
    WeakPointer<Derive> p2(p1);
    p1.Reset();
  }
}