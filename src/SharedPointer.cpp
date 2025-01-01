#include "../include/SharedPointer.hpp"

#include <iostream>
#include <memory>
using namespace tinysmartpointer;

// namespace tinysmartpointer {
int main() {
  SharedPointer<int> p(new int(5));
  std::cout << *p << std::endl;
  SharedPointer<int> q(p);
  *p = 6;
  std::cout << *q << std::endl;
  {
    SharedPointer<int> m(p);
  }

  return 0;
}
// }  // namespace tinysmartpointer