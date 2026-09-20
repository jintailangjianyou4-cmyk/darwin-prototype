cmake_minimum_required(VERSION 3.16)
project(darwin_prototype C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(darwin_runtime STATIC
    src/portable_runtime.c
)
target_include_directories(darwin_runtime PUBLIC include)

add_executable(darwinctl src/darwinctl.c)
target_link_libraries(darwinctl PRIVATE darwin_runtime)
target_include_directories(darwinctl PRIVATE include)

if(WIN32)
    target_compile_definitions(darwin_runtime PUBLIC DARWIN_HOST_WINDOWS=1)
elseif(ANDROID)
    target_compile_definitions(darwin_runtime PUBLIC DARWIN_HOST_ANDROID=1)
elseif(APPLE)
    target_compile_definitions(darwin_runtime PUBLIC DARWIN_HOST_APPLE=1)
elseif(BSD)
    target_compile_definitions(darwin_runtime PUBLIC DARWIN_HOST_BSD=1)
elseif(UNIX)
    target_compile_definitions(darwin_runtime PUBLIC DARWIN_HOST_UNIX=1)
endif()

enable_testing()
add_test(NAME identity COMMAND darwinctl uname)
set_tests_properties(identity PROPERTIES PASS_REGULAR_EXPRESSION "Darwin 27\\.0\\.0")
add_test(NAME product COMMAND darwinctl sw_vers)
set_tests_properties(product PROPERTIES PASS_REGULAR_EXPRESSION "ProductVersion: 27\\.0\\.0")
