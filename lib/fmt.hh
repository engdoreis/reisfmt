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
  const char *fmt_str = nullptr;

  enum class Radix { Bin = 2, Oct = 8, Dec = 10, Hex = 16 };

  Radix radix_  = Radix::Dec;
  uint32_t fill = 0;

 public:
  Fmt(T &device) : device(device) {};

 private:
  // Base case to stop the recursion.
  void format() {
    if (fmt_str && *fmt_str) {
      auto len = strlen(fmt_str);
      device.write(fmt_str, len);
      fmt_str += len;
    }
  }

  template <typename U, typename... Args>
  void format(U first, Args... rest) {
    while (fmt_str && *fmt_str) {
      if (*fmt_str == '{') {
        parse_specification();
        switch (radix_) {
          case Radix::Dec:
            to_str(buf, first);
            break;
          case Radix::Hex:
            to_hex_str(buf, first);
            break;
          default:
            break;
        };

        device.write(buf.data(), strlen(buf.data()));

        while (*(fmt_str++) == '}');
        fmt_str++;
        format(rest...);
      } else {
        device.write(fmt_str++, 1);
      }
    }
  }

  void parse_specification() {
    reset_specification();
    if (fmt_str[1] == ':') {
      fmt_str += 2;
      switch (*fmt_str) {
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
    fmt_str = fmt;
    format(args...);
    fmt_str = nullptr;
  }

  template <size_t SIZE, typename U>
    requires std::integral<U>
  static char *to_str(std::array<char, SIZE> &buf, U num) {
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
    return buf.data();
  }

  template <size_t SIZE>
  static char *to_str(std::array<char, SIZE> &buf, const std::string str) {
    auto len = std::min(buf.size() - 1, str.length());
    std::copy_n(str.begin(), len, buf.begin());
    buf[len] = 0;
    return buf.data();
  }

  template <size_t SIZE, typename U>
    requires std::integral<U>
  static inline char *to_hex_str(std::array<char, SIZE> &buf, U num) {
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
    return buf.data();
  }

  template <size_t SIZE>
  static char *to_hex_str(std::array<char, SIZE> &buf, std::string str) {
    buf[0] = 0;
    return buf.data();
  }
};
};  // namespace reisfmt
