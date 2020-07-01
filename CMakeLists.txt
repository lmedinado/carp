cmake_minimum_required(VERSION 3.0)
project(carp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/permissive- /W4)
endif ()
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wall -Wextra -pedantic)
    add_link_options(-static-libstdc++)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif ()


add_library(carp INTERFACE)
target_include_directories(carp INTERFACE include/carp/)

# tests
enable_testing()

set(catch2_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/external/catch2)
add_library(catch2 INTERFACE)
target_include_directories(catch2 INTERFACE ${catch2_include_dir})

add_library(tests_main OBJECT tests/tests_main.cc)

add_executable(test_carp tests/test_carp.cc $<TARGET_OBJECTS:tests_main>)
target_link_libraries(test_carp carp catch2)

add_test(NAME test_carp COMMAND test_carp)

# examples
add_executable(full_ex examples/full_ex.cc)
target_link_libraries(full_ex carp)

add_executable(terse_ex examples/terse_ex.cc)
target_link_libraries(terse_ex carp)