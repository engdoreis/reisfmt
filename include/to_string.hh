
#pragma once
#include <array>
#include <limits>
#include "stdint.h"
#include "stddef.h"

namespace reisfmt {

template <typename U>
constexpr U decimal_digits(U number) {
  U digits = 0;
  do {
    number /= 10;
    digits++;
  } while (number);
  return digits;
}

template <size_t SIZE, typename U>
  requires std::integral<U>
inline size_t to_str(std::array<char, SIZE> &buf, U num) {
  static_assert(SIZE >= decimal_digits(std::numeric_limits<U>::max()));
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

  return len;
}

template <size_t SIZE>
inline size_t to_str(std::array<char, SIZE> &buf, const char *str) {
  size_t len = 0;
  while (*str != 0 && len < buf.size()) {
    buf[len++] = *str++;
  }
  return len;
}

template <size_t SIZE>
inline size_t to_str(std::array<char, SIZE> &buf, const std::string str) {
  auto len = std::min(buf.size() - 1, str.length());
  std::copy_n(str.begin(), len, buf.begin());
  return len;
}

template <size_t SIZE, typename U>
  requires std::integral<U>
inline size_t to_hex_str(std::array<char, SIZE> &buf, U num) {
  static_assert(SIZE >= sizeof(U) * 2);
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
  return head;
}

template <size_t SIZE>
inline size_t to_hex_str(std::array<char, SIZE> &buf, const char *str) {
  return 0;
}

template <size_t SIZE>
inline size_t to_hex_str(std::array<char, SIZE> &buf, std::string str) {
  return 0;
}

template <size_t SIZE, typename U>
  requires std::integral<U>
inline size_t to_bit_str(std::array<char, SIZE> &buf, U num) {
  static_assert(SIZE >= sizeof(U) * 8);
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
  return head;
}

template <size_t SIZE>
inline size_t to_bit_str(std::array<char, SIZE> &buf, const char *str) {
  return 0;
}

template <size_t SIZE>
inline size_t to_bit_str(std::array<char, SIZE> &buf, std::string str) {
  return 0;
}
};  // namespace reisfmt
