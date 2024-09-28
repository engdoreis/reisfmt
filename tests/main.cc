#include <gtest/gtest.h>
#include "fmt.hh"
#include <iostream>
#include <cstring>
#include <string>

struct IostreamMock {
  std::array<char, 64> buf_{0};
  void write(const char *buf, size_t n) { std::copy(buf, buf + n, buf_.begin()); }
};

TEST(Fmt, simple_string) {
  auto mock = IostreamMock();
  reisfmt::Fmt<IostreamMock, 64> fmt(mock);
  std::string msg("Hello World!!");
  fmt.print(msg.c_str());
  EXPECT_EQ(mock.buf_.data(), msg);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
