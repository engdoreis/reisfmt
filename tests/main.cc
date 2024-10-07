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

  inline std::string to_string() {
    std::string res(buf_.begin(), buf_.end());
    buf_.clear();
    return res;
  }
};

class FmtTest : public testing::Test {
 public:
  IostreamMock mock_ = IostreamMock();
  reisfmt::Fmt<IostreamMock> fmt_;

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
  std::string arg65         = std::format("{:*>65}", "world");
  fmt_.print(msg, arg65.c_str());
  std::string arg64 = std::format("{:*>64}", "worl");
  EXPECT_EQ(mock_.to_string(), std::format(msg, arg64));
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
  int a, b;
  for (int i = 0; i < 10; i++) {
    a = -5 * i;
    b = 7 * i;
    fmt_.print(msg, a, b, a * b);
    EXPECT_EQ(mock_.to_string(), std::format(msg, a, b, a * b));
  }
}

TEST_F(FmtTest, num_fill_width) {
  constexpr const char *msg = "{:02x} * {:03} = {:04x}...";
  unsigned int a, b;
  for (int i = 0; i < 10; i++) {
    a = 5 * i;
    b = 5 * i;
    fmt_.print(msg, a, b, a * b);
    EXPECT_EQ(mock_.to_string(), std::format(msg, a, b, a * b));
  }
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

TEST_F(FmtTest, bin_unsigned) {
  constexpr const char *msg = "{:x} * {} = {:b}";
  unsigned int a, b;
  for (int i = 0; i < 10; i++) {
    a = 5 * i;
    b = 7 * i;
    fmt_.print(msg, a, b, a * b);
    EXPECT_EQ(mock_.to_string(), std::format(msg, a, b, a * b));
  }
}

TEST_F(FmtTest, bin_signed) {
  constexpr const char *msg = "{:x} * {} = {:b}";

  int a, b;
  for (int i = 0; i < 10; i++) {
    a = -5 * i;
    b = 7 * i;
    fmt_.print(msg, a, b, a * b);
    EXPECT_EQ(mock_.to_string(), std::format(msg, a, b, a * b));
  }
}

TEST_F(FmtTest, alternate_form) {
  constexpr const char *msg = "{:#x} * {} + {:#d} = {:#b}";
  unsigned int a, b, c;
  for (int i = 0; i < 10; i++) {
    a = 5 * i;
    b = 7 * i;
    c = 11 * i;
    fmt_.print(msg, a, b, c, a * b + c);
    EXPECT_EQ(mock_.to_string(), std::format(msg, a, b, c, a * b + c));
  }
}

TEST_F(FmtTest, alternate_form_and_filler) {
  constexpr const char *msg = "{:#08x} * {} + {:#04d} = {:#08b}";
  unsigned int a, b, c;
  for (int i = 0; i < 10; i++) {
    a = 5 * i;
    b = 7 * i;
    c = 11 * i;
    fmt_.print(msg, a, b, c, a * b + c);
    EXPECT_EQ(mock_.to_string(), std::format(msg, a, b, c, a * b + c));
  }
}

TEST_F(FmtTest, integer_max) {
  constexpr const char *msg = "{:#08x} * {} + {:#04d} = {:#08b}";
  unsigned long int a       = 0xffffffff;
  fmt_.print(msg, a, a, a, a);
  EXPECT_EQ(mock_.to_string(), std::format(msg, a, a, a, a));

  a = std::numeric_limits<uint64_t>::max();
  fmt_.print(msg, a, a, a, a);
  EXPECT_EQ(mock_.to_string(), std::format(msg, a, a, a, a));
}

TEST_F(FmtTest, alingment_right) {
  constexpr const char *msg = "{:*>30x}";
  unsigned int a            = 0xffffffff;
  fmt_.print(msg, a);
  EXPECT_EQ(mock_.to_string(), std::format(msg, a));
}

TEST_F(FmtTest, alingment_left) {
  constexpr const char *msg = "{:*<30x}";
  unsigned int a            = 0xffffffff;
  fmt_.print(msg, a);
  EXPECT_EQ(mock_.to_string(), std::format(msg, a));
}

TEST_F(FmtTest, alingment_center) {
  constexpr const char *msg = "{:*^30x}";
  unsigned int a            = 0xffffffff;
  fmt_.print(msg, a);
  EXPECT_EQ(mock_.to_string(), std::format(msg, a));
}

TEST_F(FmtTest, missing_format_end_guard) {
  constexpr const char *msg  = "{:#x}, {:#x";
  constexpr const char *msg2 = "{:#x, {:#x}";
  unsigned int a             = 0xffffffff;
  fmt_.print(msg, a, a);
  EXPECT_EQ(mock_.to_string(), "0xffffffff, 0xffffffff");

  fmt_.print(msg2, a, a);
  EXPECT_EQ(mock_.to_string(), "0xffffffff");
}

TEST_F(FmtTest, println) {
  constexpr const char *msg = "Hello {} {}";
  std::string arg("World");
  fmt_.println(msg, arg.c_str(), 42);
  EXPECT_EQ(mock_.to_string(), std::format(msg, arg, 42) + "\n");
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
