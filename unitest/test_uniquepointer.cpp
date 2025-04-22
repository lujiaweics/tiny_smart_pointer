#include <gtest/gtest.h>

#include <iostream>

#include "../include/SharedPointer.hpp"
#include "../include/UniquePointer.hpp"
#include "common/utility.hpp"

using TinySmartPointer::MakeUnique;
using TinySmartPointer::SharedPointer;
using TinySmartPointer::UniquePointer;
using TinySmartPointer::WeakPointer;
template class UniquePointer<double>;

TEST(Constructor, UNIQUE_POINTER_TEST) {
  UniquePointer<double>();
  UniquePointer<double>(new double());
  UniquePointer<double>(new double(), std::default_delete<double>());
  UniquePointer<double>(std::move(UniquePointer<double>(new double())));
  UniquePointer<Base>(std::move(UniquePointer<Derive>(new Derive())));
}

TEST(Assignment, UNIQUE_POINTER_TEST) {
  {
    UniquePointer<double> p1;
    p1 = std::move(p1);
  }

  {
    UniquePointer<double> p1(new double(5));
    UniquePointer<double> p2(new double(5));
    p2 = std::move(p1);
  }
}

TEST(Member_Function, UNIQUE_POINTER_TEST) {
  {
    EXPECT_EQ(*(UniquePointer<int>(new int(5))), 5);
    EXPECT_FALSE(UniquePointer<Base>(new Base())->IsDerived());
  }

  {
    Base *base_ptr = new Base();
    EXPECT_EQ(static_cast<void *>(UniquePointer<Base>(base_ptr)), reinterpret_cast<void *>(base_ptr));
    EXPECT_EQ(static_cast<void *>(UniquePointer<Base>()), reinterpret_cast<void *>(0));
    EXPECT_FALSE(static_cast<bool>(UniquePointer<Base>()));
    EXPECT_TRUE(static_cast<bool>(UniquePointer<Base>(new Base())));
  }

  {
    UniquePointer<Base>().GetDeleter();
    const UniquePointer<Base> p1;
    p1.GetDeleter();
  }

  { UniquePointer<Base>().Reset(); }

  {
    UniquePointer<Base> p1(new Base());
    UniquePointer<Base>(new Base()).Swap(p1);
  }
}