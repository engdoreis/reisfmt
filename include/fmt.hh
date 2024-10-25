
#pragma once
#include <concepts>
#include <type_traits>
#include <array>
#include <string>
#include <stdint.h>
#include <stddef.h>

#include "to_string.hh"
#include "spec.hh"

namespace reisfmt {

template <typename T>
concept Writeable = requires(T t, const char *buf, size_t n) {
  { t.write(buf, n) } -> std::same_as<void>;
};

// Foward declaration.
template <Writeable T>
class Fmt;

// Foward declaration.
template <Writeable T, typename U>
struct Formatter;

template <typename T, typename F>
concept Printable = requires(T t, Fmt<F> &fmt) {
  { t.print(fmt) } -> std::same_as<void>;
};

// This specialization allows types to implement `Printable` in order extend the print function.
template <Writeable T, typename U>
  requires std::is_class_v<U>
struct Formatter<T, U> {
  static void print(Fmt<T> &fmt, U &obj) { obj.print(fmt); }
};

template <Writeable T, typename U>
  requires std::integral<U>
struct Formatter<T, U> {
  static void print(Fmt<T> &fmt, U num) {
    size_t len = 0;
    switch (fmt.spec.radix_) {
      case Spec::Radix::Bin:
        len = to_bit_str(fmt.buf, num);
        break;
      case Spec::Radix::Hex:
        len = to_hex_str(fmt.buf, num);
        break;
      case Spec::Radix::Dec:
      default:
        len = to_str(fmt.buf, num);
        break;
    };

    StrIterator it(fmt.buf.data(), len);
    Formatter<T, StrIterator>::print(fmt, it);
  }
};

template <Writeable T>
struct Formatter<T, StrIterator> {
  static inline void print(Fmt<T> &fmt, StrIterator &text) {
    auto &spec = fmt.spec;
    if (auto opt = spec.prefix_) {  // Is there a formating modifier(#)?
      StrIterator prefix = *opt;
      fmt.device.write(prefix.head_, prefix.size_);
      spec.width_ -= prefix.size_;
    }

    if ((spec.align_ == Spec::Align::Center || spec.align_ == Spec::Align::Right) && spec.width_ > text.size_) {
      int diff = (spec.width_ - text.size_) / (1 + (spec.align_ == Spec::Align::Center));
      spec.width_ -= diff;
      while (diff-- > 0) {
        fmt.device.write(&spec.filler_, sizeof(spec.filler_));
      }
    }

    // Print the formatted type.
    fmt.device.write(text.head_, text.size_);

    // align_ == Spec::Align::Left || Spec::Align::Center
    while (spec.width_-- > text.size_) {
      fmt.device.write(&spec.filler_, sizeof(spec.filler_));
    }
  }
};

template <Writeable T>
struct Formatter<T, const char *> {
  static inline void print(Fmt<T> &fmt, const char *str) {
    StrIterator text(str);
    Formatter<T, StrIterator>::print(fmt, text);
  }
};

template <Writeable T>
struct Formatter<T, std::basic_string<char>> {
  static inline void print(Fmt<T> &fmt, const std::string &str) {
    StrIterator text(str.c_str(), str.length());
    Formatter<T, StrIterator>::print(fmt, text);
  }
};

template <Writeable T>
class Fmt {
 public:
  T &device;
  std::array<char, sizeof(uint64_t) * 8> buf;
  StrIterator *it_;
  Spec spec;

  Fmt(T &device) : device(device) {};

 private:
  // Base case to stop the recursion.
  void format() {
    if (it_ && it_->peek()) {
      device.write(it_->head_, it_->size_);
      it_->next(it_->size_);
    }
  }

  template <typename U, typename... Args>
  void format(U first, Args... rest) {
    if (it_ == nullptr || it_->size_ == 0) {
      return;
    }
    auto start = it_->head_;
    auto end   = it_->find('{');
    // Print the string preceding the format guard.
    device.write(start, end - start - int(it_->size_ > 0));
    if (it_->size_ > 0) {  // Has the format guard been found?
      spec.from_str(*it_);

      // The formatter can be extented for custom types, so the context is saved to allow the custom formatter to
      // recursively call this function.
      auto it = it_;
      Formatter<T, U>::print(*this, first);
      it_ = it;

      it_->find('}');
      format(rest...);
    }
  }

 public:
  template <typename... Args>
  void println(const char *fmt, Args... args) {
    print(fmt, args...);
    device.write("\r\n", 2);
  }

  template <typename... Args>
  void print(const char *fmt, Args... args) {
    if (fmt) {
      StrIterator it(fmt);
      it_ = &it;
      format(args...);
      it_ = nullptr;
    }
  }
};

};  // namespace reisfmt
