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

TEST_F(FmtTest, null_fmt) {
  fmt_.print(nullptr);
  EXPECT_EQ(mock_.to_string(), "");
}

TEST_F(FmtTest, long_arg_str) {
  constexpr const char *msg = "Hello {} !!";
  std::string arg64         = std::format("{:*^64}", "world");
  fmt_.print(msg, arg64);
  std::string arg63 = std::format("{:*^63}", "world");
  EXPECT_EQ(mock_.to_string(), std::format(msg, arg63));
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
  constexpr const char *msg = "{} * {}...";
  fmt_.print(msg, 10, 20, 10 * 20);
  EXPECT_EQ(mock_.to_string(), std::format(msg, 10, 20, 10 * 20));
}

TEST_F(FmtTest, int_arg_execess2) {
  constexpr const char *msg = "{} * {}";
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

TEST_F(FmtTest, num_fill_width) {
  constexpr const char *msg = "{:02x} * {:03} = {:04x}...";
  fmt_.print(msg, 10, 20, 10 * 20);
  EXPECT_EQ(mock_.to_string(), std::format(msg, 10, 20, 10 * 20));
}

TEST_F(FmtTest, string_fill_width) {
  constexpr const char *msg = "{:*>8},  {:.>9}";
  fmt_.print(msg, "hello", "world");
  EXPECT_EQ(mock_.to_string(), std::format(msg, "hello", "world"));
}

TEST_F(FmtTest, string_fill_two_digits) {
  constexpr const char *msg = "{:*>10},  {:.>29}";
  fmt_.print(msg, "hello", "world");
  EXPECT_EQ(mock_.to_string(), std::format(msg, "hello", "world"));
}

TEST_F(FmtTest, string_fill_shorter_than_output) {
  constexpr const char *msg = "{:*>1},  {:.>2}";
  fmt_.print(msg, "hello", "world");
  EXPECT_EQ(mock_.to_string(), std::format(msg, "hello", "world"));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
