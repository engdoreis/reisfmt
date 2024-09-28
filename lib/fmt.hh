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
  T device;
  std::array<char, N> buf;

 public:
  Fmt(T &device) : device(device){};

  template <typename... Args>
  void print(const char *fmt, Args... args) {
    auto len = strlen(fmt);
    std::copy(fmt, fmt + len, buf.begin());
    device.write(buf.data(), N);
  }
};
};  // namespace reisfmt
