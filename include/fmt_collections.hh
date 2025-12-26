
#pragma once
#include <concepts>
#include <array>
#include <span>
#include "writeable.hh"

namespace reisfmt {
template <Writeable T, typename U>
struct Formatter<T, std::span<U>> {
  static inline void print(Fmt<T> &fmt, std::span<U> arr) {
    fmt.print("[");
    for (auto b : arr ){
      fmt.print(" {:#x},", b);
    }
    fmt.println("]");
  }
};

template <Writeable T, typename U, size_t N>
struct Formatter<T, std::array<U, N> > {
  static inline void print(Fmt<T> &fmt, std::array<U, N> arr) {
    Formatter<T, std::span<U>>::print(fmt, arr);
  }
};

template <Writeable T, typename U>
struct Formatter<T, std::vector<U> > {
  static inline void print(Fmt<T> &fmt, std::vector<U> arr) {
    Formatter<T, std::span<U>>::print(fmt, arr);
  }
};
}
