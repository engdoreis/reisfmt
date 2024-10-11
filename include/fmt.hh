
#pragma once
#include <concepts>
#include <array>
#include <algorithm>
#include <limits>
#include "stdint.h"
#include "stddef.h"

#include "to_string.hh"
#include "spec.hh"

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
  StrIterator *it_;
  Spec spec;

 public:
  Fmt(T &device) : device(device) {};

 private:
  // Base case to stop the recursion.
  void format() {
    if (it_ && it_->peek()) {
      device.write(it_->head_, it_->size_);
      it_ = nullptr;
    }
  }

  template <typename U, typename... Args>
  void format(U first, Args... rest) {
    if (it_ == nullptr || it_->size_ == 0) {
      return;
    }
    auto start = it_->head_;

    // Find format start guard
    auto end = it_->find('{');
    device.write(start, end - start - int(it_->size_ > 0));
    if (it_->size_ > 0) {
      spec.from_str(*it_);

      size_t len = 0;
      switch (spec.radix_) {
        case Spec::Radix::Bin:
          len = to_bit_str(buf, first);
          break;
        case Spec::Radix::Hex:
          len = to_hex_str(buf, first);
          break;
        case Spec::Radix::Dec:
        default:
          len = to_str(buf, first);
          break;
      };

      if (spec.has_prefix_) {
        device.write(spec.prefix_, spec.prefix_size_);
        spec.width_ -= spec.prefix_size_;
      }
      if ((spec.align_ == Spec::Align::Center || spec.align_ == Spec::Align::Right) && spec.width_ > len) {
        int diff = spec.width_ - len;
        diff     = spec.align_ == Spec::Align::Center ? diff / 2 : diff;
        spec.width_ -= diff;
        while (diff-- > 0) {
          device.write(&spec.filler_, sizeof(spec.filler_));
        }
      }
      device.write(buf.data(), len);

      // align_ == Spec::Align::Left || Spec::Align::Center
      if (spec.width_ > len) {
        while (spec.width_-- > len) {
          device.write(&spec.filler_, sizeof(spec.filler_));
        }
      }
      it_->find('}');
      format(rest...);
    }
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
      StrIterator it(fmt);
      it_ = &it;
      format(args...);
      it_ = nullptr;
    }
  }
};
};  // namespace reisfmt
