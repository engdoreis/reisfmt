#include <gtest/gtest.h>
#include "fmt.hh"
#include <iostream>
#include <cstring>
#include <string>
#include <format>

struct IostreamMock {
  std::vector<char> buf_;
  void write(const char *buf, size_t n) {
    while (n--) {
      buf_.push_back(*buf++);
    }
  }

  std::string to_string() { return std::string(buf_.begin(), buf_.end()); }
};

class FmtTest : public testing::Test {
 public:
  IostreamMock mock_ = IostreamMock();
  reisfmt::Fmt<IostreamMock, 64> fmt_;

  FmtTest() : fmt_(mock_) {}
};

TEST_F(FmtTest, simple_string) {
  std::string msg("Hello World!!");
  fmt_.print(msg.c_str());
  EXPECT_EQ(mock_.to_string(), msg);
}

TEST_F(FmtTest, string_arg) {
  constexpr const char *msg = "Hello {} !!";
  std::string arg("World");
  fmt_.print(msg, arg);
  EXPECT_EQ(mock_.to_string(), std::format(msg, arg));
}

TEST_F(FmtTest, c_string_arg) {
  constexpr const char *msg = "Hello {} !!";
  const char *arg           = "World";
  fmt_.print(msg, arg);
  EXPECT_EQ(mock_.to_string(), std::format(msg, arg));
}

TEST_F(FmtTest, pos_int) {
  constexpr const char *msg = "{} * {} = {}...";
  fmt_.print(msg, 10, 20, 10 * 20);
  EXPECT_EQ(mock_.to_string(), std::format(msg, 10, 20, 10 * 20));
}

TEST_F(FmtTest, neg_int) {
  constexpr const char *msg = "{} * {} = {}...";
  fmt_.print(msg, -10, 20, -10 * 20);
  EXPECT_EQ(mock_.to_string(), std::format(msg, -10, 20, -10 * 20));
}

TEST_F(FmtTest, int_arg_execess) {
  constexpr const char *msg = "{} * {} ";
  fmt_.print(msg, 10, 20, 10 * 20);
  EXPECT_EQ(mock_.to_string(), std::format(msg, 10, 20, 10 * 20));
}

TEST_F(FmtTest, int_arg_missing) {
  constexpr const char *msg = "{} * {} = {}";
  fmt_.print(msg, 10, 20);
  EXPECT_EQ(mock_.to_string(), "10 * 20 = {}");
}

TEST_F(FmtTest, hex_unsigned) {
  constexpr const char *msg = "{:x} * {} = {:x}...";
  fmt_.print(msg, 10, 20, 10 * 20);
  EXPECT_EQ(mock_.to_string(), std::format(msg, 10, 20, 10 * 20));
}

TEST_F(FmtTest, hex_signed) {
  constexpr const char *msg = "{:x} * {} = {:x}...";
  fmt_.print(msg, -10, 20, -10 * 20);
  EXPECT_EQ(mock_.to_string(), std::format(msg, -10, 20, -10 * 20));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
