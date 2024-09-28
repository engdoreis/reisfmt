#include "fmt.hh"
#include <iostream>
#include <cstring>

struct IostreamWrapper {
  void write(const char *buf, size_t n) {
    // if (strlen(buf) < n) {
    std::cout << buf;
    // }
  }
};

int main(void) {
  auto wrapper = IostreamWrapper();
  reisfmt::Fmt<IostreamWrapper, 64> fmt(wrapper);
  fmt.print("hello world", 1);
  return 0;
}
