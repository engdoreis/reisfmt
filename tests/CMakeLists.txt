set(TEST_NAME ${NAME}_test)

add_executable(${TEST_NAME}  main.cc )
target_include_directories(${TEST_NAME} PRIVATE ../lib)

