
#pragma once
#include <concepts>
#include <array>
#include <algorithm>
#include "cstring"

namespace reisfmt {

template <typename T>
concept Writeable = requires(T t, const char *buf, size_t n) {
  { t.write(buf, n) } -> std::same_as<void>;
};

template <Writeable T, size_t N>
class Fmt {
 private:
  T &device;
  std::array<char, N> buf;

  const char *fmt_ = nullptr;
  size_t fmt_size_ = 0;

  enum class Radix { Bin = 2, Oct = 8, Dec = 10, Hex = 16 };
  enum class Align { Left, Right, Center };

  Radix radix_        = Radix::Dec;
  Align align_        = Align::Right;
  uint32_t width_     = 0;
  char filler_        = ' ';
  bool has_prefix_    = false;
  const char *prefix_ = "0";
  size_t prefix_size_ = 1;

 public:
  Fmt(T &device) : device(device) {};

 private:
  inline char next_fmt(int step = 1) {
    fmt_size_ -= step;
    auto res = *fmt_;
    fmt_ += step;
    return res;
  }

  inline char peek_fmt(int step = 1) { return *fmt_; }
  // Base case to stop the recursion.
  void format() {
    if (fmt_ && *fmt_) {
      device.write(fmt_, fmt_size_);
      fmt_      = nullptr;
      fmt_size_ = 0;
    }
  }

  template <typename U, typename... Args>
  void format(U first, Args... rest) {
    if (fmt_ == nullptr || *fmt_ == 0) {
      return;
    }
    auto start = fmt_;

    // Find format start guard
    while (next_fmt() != '{' && fmt_size_ > 0);
    if (fmt_size_ > 0) {
      device.write(start, fmt_ - start - 1);
      parse_specification();

      size_t len = 0;
      switch (radix_) {
        case Radix::Bin:
          len = to_bit_str(buf, first);
          break;
        case Radix::Hex:
          len = to_hex_str(buf, first);
          break;
        case Radix::Dec:
        default:
          len = to_str(buf, first);
          break;
      };

      if (has_prefix_) {
        device.write(prefix_, prefix_size_);
        width_ -= prefix_size_;
      }
      if (width_ > len) {
        for (int i = width_ - len; i > 0; --i) {
          device.write(&filler_, sizeof(filler_));
        }
      }
      device.write(buf.data(), len);

      // Find format end guard.
      while (next_fmt() != '}');
      format(rest...);
    } else {
      device.write(start, fmt_ - start);
    }
  }

  void parse_specification() {
    reset_specification();
    if (peek_fmt() == ':') {
      next_fmt();
      parse_alternate_mode();
      parse_fill_and_align();
      parse_width();
      parse_type();
    }
  }

  inline void parse_fill_and_align() {
    char align = '>';
    if (fmt_[0] == '<' || fmt_[0] == '>' || fmt_[0] == '^') {
      align   = fmt_[0];
      filler_ = ' ';
      next_fmt();
    } else if (fmt_[1] == '<' || fmt_[1] == '>' || fmt_[1] == '^') {
      align   = fmt_[1];
      filler_ = fmt_[0];
      next_fmt(2);
    } else if (std::isdigit(fmt_[1])) {
      filler_ = fmt_[0];
      next_fmt();
    }

    switch (align) {
      case '<':
        align_ = Align::Left;
        break;
      case '^':
        align_ = Align::Center;
        break;
      case '>':
      default:
        align_ = Align::Right;
        break;
    }
  }

  inline void parse_alternate_mode() {
    if (peek_fmt() == '#') {
      has_prefix_ = true;
      next_fmt();
    }
  }

  inline void parse_width() {
    while (std::isdigit(peek_fmt())) {
      width_ = width_ * 10 + next_fmt() - '0';
    }
  }

  inline void parse_type() {
    switch (peek_fmt()) {
      case 'x':
        radix_ = Radix::Hex;
        next_fmt();
        prefix_      = "0x";
        prefix_size_ = 2;
        break;
      case 'd':
        radix_ = Radix::Dec;
        next_fmt();
        has_prefix_ = false;
        break;
      case 'b':
        radix_ = Radix::Bin;
        next_fmt();
        prefix_      = "0b";
        prefix_size_ = 2;
        break;
      case 'o':
        radix_ = Radix::Oct;
        next_fmt();
        prefix_      = "0";
        prefix_size_ = 1;
        break;
      default:
        break;
    }
  }

  inline void reset_specification() {
    radix_       = Radix::Dec;
    align_       = Align::Right;
    width_       = 0;
    filler_      = ' ';
    has_prefix_  = false;
    prefix_size_ = 0;
  }

 public:
  template <typename... Args>
  void print(const char *fmt, Args... args) {
    if (fmt) {
      fmt_ = fmt;
      // Avoid using libc:strlen for system without libc.
      for (fmt_size_ = 0; fmt[fmt_size_] != 0; fmt_size_++);
      format(args...);
      fmt_ = nullptr;
    }
  }

  template <size_t SIZE, typename U>
    requires std::integral<U>
  static size_t to_str(std::array<char, SIZE> &buf, U num) {
    size_t head = 0;
    size_t tail = SIZE - 1;
    if constexpr (std::signed_integral<U>) {
      // This code won't be linked for unsigned U.
      if (num < 0) {
        buf[head++] = '-';
        num         = 0 - num;
      }
    }

    do {
      buf[tail--] = num % 10 + '0';
      num /= 10;
    } while (tail > 0 && num > 0);

    tail++;
    size_t len = SIZE - tail + head;
    for (size_t i = 0; i < len; ++i) {
      buf[head + i] = buf[tail + i];
    }

    buf[len] = 0;
    return len;
  }

  template <size_t SIZE>
  static size_t to_str(std::array<char, SIZE> &buf, const std::string str) {
    auto len = std::min(buf.size() - 1, str.length());
    std::copy_n(str.begin(), len, buf.begin());
    buf[len] = 0;
    return len;
  }

  template <size_t SIZE, typename U>
    requires std::integral<U>
  static inline size_t to_hex_str(std::array<char, SIZE> &buf, U num) {
    assert(SIZE > sizeof(U) * 2 + 1);
    constexpr U shift = (sizeof(U) * 8 - 4);

    size_t head = 0;
    if constexpr (std::signed_integral<U>) {
      // This code won't be linked for unsigned U.
      if (num < 0) {
        buf[head++] = '-';
        num         = 0 - num;
      }
    }

    // Consume the leading zeros.
    size_t i = 0;
    for (; i < (sizeof(U) * 2) - 1; ++i) {
      if (((num >> shift) & 0xf) > 0) {
        break;
      }
      num <<= 4;
    }

    for (; i < (sizeof(U) * 2); ++i) {
      int masked = ((num >> shift) & 0xf);
      if (masked < 10) {
        buf[head++] = masked + '0';
      } else {
        buf[head++] = masked + 'a' - 10;
      }
      num <<= 4;
    }
    buf[head] = 0;
    return head;
  }

  template <size_t SIZE>
  static size_t to_hex_str(std::array<char, SIZE> &buf, std::string str) {
    return 0;
  }

  template <size_t SIZE, typename U>
    requires std::integral<U>
  static inline size_t to_bit_str(std::array<char, SIZE> &buf, U num) {
    assert(SIZE > sizeof(U) * 8 + 1);
    constexpr U shift = (sizeof(U) * 8 - 1);

    size_t head = 0;
    if constexpr (std::signed_integral<U>) {
      // This code won't be linked for unsigned U.
      if (num < 0) {
        buf[head++] = '-';
        num         = 0 - num;
      }
    }

    // Consume the leading zeros.
    size_t i = 0;
    for (; i < (sizeof(U) * 8) - 1; ++i) {
      if (((num >> shift) & 0x1)) {
        break;
      }
      num <<= 1;
    }

    for (; i < (sizeof(U) * 8); ++i) {
      buf[head++] = ((num >> shift) & 0x1) + '0';
      num <<= 1;
    }
    buf[head] = 0;
    return head;
  }

  template <size_t SIZE>
  static size_t to_bit_str(std::array<char, SIZE> &buf, std::string str) {
    return 0;
  }
};
};  // namespace reisfmt
