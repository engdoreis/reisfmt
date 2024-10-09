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


