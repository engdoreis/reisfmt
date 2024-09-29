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
        to_str(buf, first);
        device.write(buf.data(), strlen(buf.data()));

        while (*(fmt_str++) == '}');
        fmt_str++;
        format(rest...);
      } else {
        device.write(fmt_str++, 1);
      }
    }
  }

 public:
  template <typename... Args>
  void print(const char *fmt, Args... args) {
    fmt_str = fmt;
    format(args...);
    fmt_str = nullptr;
  }

  template <size_t SIZE, typename U>
    requires std::integral<U>
  static inline char *to_str(std::array<char, SIZE> &buf, U num) {
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
};
};  // namespace reisfmt
