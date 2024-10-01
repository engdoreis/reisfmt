set(TEST_NAME ${NAME}_test)

# Fetch GoogleTest library. 
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/release-1.12.1.zip
)
FetchContent_MakeAvailable(googletest)

add_compile_options(-O0 -g3)
add_executable(${TEST_NAME}  main.cc )
target_include_directories(${TEST_NAME} PRIVATE ../include)
target_link_libraries(${TEST_NAME} PRIVATE GTest::gtest_main)

add_test(NAME Test_0 COMMAND ${TEST_NAME})
