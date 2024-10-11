
#pragma once
#include "stdint.h"
#include "stddef.h"

namespace reisfmt {
struct StrIterator {
  const char *head_ = nullptr;
  size_t size_      = 0;

  StrIterator(const char *str) {
    head_ = str;
    for (size_ = 0; str[size_] != 0; size_++);
  }

  StrIterator(const char *start, const char *end) {
    head_ = start;
    size_ = end - start;
  }

  StrIterator(const char *start, size_t size) {
    head_ = start;
    size_ = size;
  }

  inline std::optional<char> next(int step = 1) {
    if (size_ == 0) {
      return std::nullopt;
    }
    size_ -= step;
    auto res = *head_;
    head_ += step;
    return std::optional{res};
  }

  inline char peek(int pos = 0) { return head_[pos]; }

  inline const char *find(char c) {
    std::optional<char> opt;
    do {
      opt = next();
    } while (opt.has_value() && opt.value() != c);
    return head_;
  };
};

struct Spec {
  enum class Radix { Bin = 2, Oct = 8, Dec = 10, Hex = 16 };
  enum class Align { Left, Right, Center };

  Radix radix_        = Radix::Dec;
  Align align_        = Align::Right;
  uint32_t width_     = 0;
  char filler_        = ' ';
  bool has_prefix_    = false;
  const char *prefix_ = "0";
  size_t prefix_size_ = 1;

  void from_str(StrIterator &it) {
    reset();
    if (it.peek() == ':') {
      it.next();
      parse_alternate_mode(it);
      parse_fill_and_align(it);
      parse_width(it);
      parse_type(it);
    }
  }

  inline void parse_fill_and_align(StrIterator &it) {
    char align = '>';
    if (it.peek() == '<' || it.peek() == '>' || it.peek() == '^') {
      align   = it.peek();
      filler_ = ' ';
      it.next();
    } else if (it.peek(1) == '<' || it.peek(1) == '>' || it.peek(1) == '^') {
      align   = it.peek(1);
      filler_ = it.peek();
      it.next(2);
    } else if (std::isdigit(it.peek(1))) {
      filler_ = it.peek();
      it.next();
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

  inline void parse_alternate_mode(StrIterator &it) {
    if (it.peek() == '#') {
      has_prefix_ = true;
      it.next();
    }
  }

  inline void parse_width(StrIterator &it) {
    char c;
    while (std::isdigit((c = it.peek()))) {
      width_ = width_ * 10 + c - '0';
      it.next();
    }
  }

  inline void parse_type(StrIterator &it) {
    switch (it.peek()) {
      case 'x':
        radix_ = Radix::Hex;
        it.next();
        prefix_      = "0x";
        prefix_size_ = 2;
        break;
      case 'd':
        radix_ = Radix::Dec;
        it.next();
        has_prefix_ = false;
        break;
      case 'b':
        radix_ = Radix::Bin;
        it.next();
        prefix_      = "0b";
        prefix_size_ = 2;
        break;
      case 'o':
        radix_ = Radix::Oct;
        it.next();
        prefix_      = "0";
        prefix_size_ = 1;
        break;
      default:
        break;
    }
  }

  inline void reset() {
    radix_       = Radix::Dec;
    align_       = Align::Right;
    width_       = 0;
    filler_      = ' ';
    has_prefix_  = false;
    prefix_size_ = 0;
  }
};
};  // namespace reisfmt
