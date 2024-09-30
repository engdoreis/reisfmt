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

  Radix radix_  = Radix::Dec;
  uint32_t fill = 0;

 public:
  Fmt(T &device) : device(device) {};

 private:
  inline char next_fmt(int step = 1) {
    fmt_size_ -= step;
    auto res = *fmt_;
    fmt_ += step;
    return res;
  }
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
    while (fmt_ && *fmt_) {
      if (*fmt_ == '{') {
        parse_specification();

        size_t len = 0;
        switch (radix_) {
          case Radix::Dec:
            len = to_str(buf, first);
            break;
          case Radix::Hex:
            len = to_hex_str(buf, first);
            break;
          default:
            break;
        };

        device.write(buf.data(), len);

        while (next_fmt() != '}');
        format(rest...);
      } else {
        auto c = next_fmt();
        device.write(&c, sizeof(c));
      }
    }
  }

  void parse_specification() {
    reset_specification();
    if (fmt_[1] == ':') {
      next_fmt(2);
      switch (*fmt_) {
        case 'x':
          radix_ = Radix::Hex;
          break;
        case 'd':
          radix_ = Radix::Dec;
          break;
        case 'b':
          radix_ = Radix::Bin;
          break;
        case 'o':
          radix_ = Radix::Oct;
          break;
      }
    }
  }

  void reset_specification() { radix_ = Radix::Dec; }

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

    for (; tail > 0 && num > 0; --tail) {
      buf[tail] = num % 10 + '0';
      num /= 10;
    }

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
    for (size_t i = 0; i < (sizeof(U) * 2); ++i) {
      int mask = ((num >> shift) & 0xf);
      if (mask < 10 && mask > 0) {
        buf[head++] = ((num >> shift) & 0xf) + '0';
      } else if (mask >= 10) {
        buf[head++] = ((num >> shift) & 0xf) + 'a' - 10;
      }
      num <<= 4;
    }
    buf[head] = 0;
    return head;
  }

  template <size_t SIZE>
  static size_t to_hex_str(std::array<char, SIZE> &buf, std::string str) {
    buf[0] = 0;
    return 0;
  }
};
};  // namespace reisfmt
