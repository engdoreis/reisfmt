
#pragma once
#include <concepts>
#include <array>
#include <algorithm>
#include <limits>
#include "cstring"
#include "stdint.h"
#include "stddef.h"

#include "to_string.hh"

namespace reisfmt {

template <typename T>
concept Writeable = requires(T t, const char *buf, size_t n) {
  { t.write(buf, n) } -> std::same_as<void>;
};

template <Writeable T>
class Fmt {
 private:
  T &device;
  std::array<char, sizeof(uint64_t) * 8> buf;

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
  inline std::optional<char> next_fmt(int step = 1) {
    if (fmt_size_ == 0) {
      return std::nullopt;
    }
    fmt_size_ -= step;
    auto res = *fmt_;
    fmt_ += step;
    return std::optional{res};
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
    auto find = [&](char c) {
      std::optional<char> opt;
      do {
        opt = next_fmt();
      } while (opt.has_value() && opt.value() != c);
    };
    auto start = fmt_;

    // Find format start guard
    find('{');
    device.write(start, fmt_ - start - int(fmt_size_ > 0));
    if (fmt_size_ > 0) {
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
      if ((align_ == Align::Center || align_ == Align::Right) && width_ > len) {
        int diff = width_ - len;
        diff     = align_ == Align::Center ? diff / 2 : diff;
        width_ -= diff;
        while (diff-- > 0) {
          device.write(&filler_, sizeof(filler_));
        }
      }
      device.write(buf.data(), len);

      // align_ == Align::Left || Align::Center
      if (width_ > len) {
        while (width_-- > len) {
          device.write(&filler_, sizeof(filler_));
        }
      }
      find('}');
      format(rest...);
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
    char c;
    while (std::isdigit((c = peek_fmt()))) {
      width_ = width_ * 10 + c - '0';
      next_fmt();
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
  void println(const char *fmt, Args... args) {
    print(fmt, args...);
    device.write("\n", 1);
  }

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
};
};  // namespace reisfmt
