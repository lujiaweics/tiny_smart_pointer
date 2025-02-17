#include "../include/SharedPointer.hpp"
#include "../include/UniquePointer.hpp"
#include <gtest/gtest.h>

#include <iostream>
using namespace tinysmartpointer;

class Tmp {
 public:
  int num_;
  Tmp(int num = 0) : num_(num) {}
  void Increase() { ++num_; }
  int Get() { return num_; }

  virtual ~Tmp() {}
};

class Derive : public Tmp {
 public:
  explicit Derive(int num) : Tmp(num) {}
};

void deleter(Tmp* pointer) { delete pointer; }

TEST(Unique_Pointer, SMART_POINTER_TEST) {
  UniquePointer<int> unique_pointer(new int(8));
  EXPECT_EQ(*unique_pointer, 8);
  auto unique_pointer2 = std::move(unique_pointer);
  EXPECT_EQ(*unique_pointer2, 8);
  EXPECT_EQ(*(unique_pointer2.Get()), 8);

  UniquePointer<Tmp> tmp_pointer(new Tmp(5));
  EXPECT_EQ(tmp_pointer->num_, 5);
  tmp_pointer->Increase();
  EXPECT_EQ(tmp_pointer.Get()->num_, 6);
  auto dumb_pointer = new Tmp();
  EXPECT_EQ(sizeof(dumb_pointer), sizeof(tmp_pointer));

  UniquePointer<Tmp, decltype(deleter)> self_defined_pointer(new Tmp(5), deleter);
  EXPECT_EQ(sizeof(self_defined_pointer), sizeof(&deleter) + sizeof(dumb_pointer));

  UniquePointer<Derive> unique_pointer3(new Derive(5));
  UniquePointer<Tmp> unique_pointer4(std::move(unique_pointer3));

  auto unique_pointer5(MakeUnique<Tmp>(10));
  EXPECT_EQ(unique_pointer5->num_, 10);
}

TEST(Shared_Pointer, SMART_POINTER_TEST) {
  SharedPointer<int> p(new int(5));
  EXPECT_EQ(*p, 5);
  {
    SharedPointer<int> q(p);
    EXPECT_EQ(*q, 5);
    EXPECT_EQ(q.UseCount(), 2);
    EXPECT_FALSE(q.Unique());
  }
  EXPECT_EQ(p.UseCount(), 1);
  *p = 6;
  EXPECT_EQ(*p, 6);

  using Pair = std::pair<int, int>;
  SharedPointer<std::pair<int, int>> pair_pointer(new Pair(4, 5));
  SharedPointer<int> pair_to_second(pair_pointer, &(pair_pointer->second));
  EXPECT_EQ(*pair_to_second, 5);
}

int main(int argc, char **argv) {  
 std::cout << "Running main() from gtest_main.cc\n";  
 testing::InitGoogleTest(&argc, argv);  
 return RUN_ALL_TESTS();  
}