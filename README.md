# reisfmt
This is a tiny implementation, header only of the c++23 std::print for embedded systems.
It only depend on some headers of the standard lib.

## How to download using CMake
```Cmake
<!-- CMakelist.txt -->
cmake_minimum_required(VERSION 3.13)
include(FetchContent)

FetchContent_Declare(REISFMT
  GIT_REPOSITORY    https://github.com/engdoreis/reisfmt
  GIT_TAG           <git hash>
)
FetchContent_Populate(REISFMT)

set(NAME my_app)
add_executable(${NAME} main.cc)
target_include_directories(${NAME} PRIVATE "${reisfmt_SOURCE_DIR}/include")
```

## How to instantiate in your project
```cpp
// log.hh
#include "fmt.h"
#include "uart.h"

struct LogUart{
  Uart* uart;
  // Implement your serialization here (UART, USB...)
  void write(const char* buf, size_t n) {
    while (n--) {
      uart->putc(*buf++);
    }
  }
};

using Log = reisfmt::Fmt<LogUart>;

//main.cc
#include "log.h"
//...
int main() {
// ...
  LogUart log_uart{uart0};
  Log log(log_uart);
  log.print("Hello {}", "World");
  return 0;
}
```
## How to extend the print for a custom type
Custom types can be printed in two different ways.
The first option and recommended in most cases is to implement the `concept Printable` for the type.
```cpp
struct Memory {
  size_t addr;
  size_t size;

  // Printable concept
  template <typename T>
  inline void print(reisfmt::Fmt<T> &fmt) {
    fmt.print("Memory: addr: {:#x}, size: {}", addr, size);
  }
};
```
The second option is more verbose and may be needed in some cases where the type is defined by an external library. Here we specialize the struct `Formatter` for the desired type.
```cpp
struct Memory {
  size_t addr;
  size_t size;
};

namespace reisfmt {
template <typename T>
struct Formatter<T, Memory> {
  static inline void print(Fmt<T> &fmt, Memory &obj) {
    fmt.print("Memory: addr: {:#x}, size: {}", obj.addr, obj.size);
  }
};
}
```
Now you can print the `struct Memory` using the `fmt` library.
```cpp
  fmt_.println("{}", Memory{0x1000'0000, 1024 * 256});
```

## Formating specification
This library follows the libc++ format specification defined [here](https://en.cppreference.com/w/cpp/utility/format/spec).

The following features above from the specifications are implemented:

|Spec|Implmented|
|-|-|
|fill-and-align|yes|
|sign|no|
|#|yes|
|width|yes|
|precision|no|
|locale|no|
|type| See below|

|Type|Implmemented|
|-|-|
|d|yes|
|b|yes|
|x|yes|
|o|yes|
|f|no|
|a|no|
|e|no|
|g|no|
|p|no|
|?|no|


