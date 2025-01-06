#include "../include/SharedPointer.hpp"
#include "../include/UniquePointer.hpp"
#include <gtest/gtest.h>

#include <iostream>
using namespace tinysmartpointer;

TEST(Shared_Pointer, SMART_POINTER_TEST)
{
  SharedPointer<int> p(new int(5));
  EXPECT_EQ(*p, 5);
  SharedPointer<int> q(p);
  EXPECT_EQ(*q, 5);
  *p = 6;
  EXPECT_EQ(*q, 6);
}

TEST(Unique_Pointer, SMART_POINTER_TEST) {
  UniquePointer<int> unique_pointer(new int(8));
  EXPECT_EQ(*unique_pointer, 8);
  auto unique_pointer2 = std::move(unique_pointer);
  EXPECT_EQ(*unique_pointer2, 8);
  EXPECT_EQ(*(unique_pointer2.Get()), 8);

  class Tmp {
   public:
    int num_;
    Tmp(int num = 0) : num_(num) {}
    void Increase() { ++num_; }
    int Get() { return num_; }
  };
  UniquePointer<Tmp> tmp_pointer(new Tmp(5));
  EXPECT_EQ(tmp_pointer->num_, 5);
  tmp_pointer->Increase();
  EXPECT_EQ(tmp_pointer.Get()->num_, 6);
}

int main(int argc, char **argv) {  
 std::cout << "Running main() from gtest_main.cc\n";  
 testing::InitGoogleTest(&argc, argv);  
 return RUN_ALL_TESTS();  
}