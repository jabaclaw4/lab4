cmake_minimum_required(VERSION 3.28)
project(lab3)

set(CMAKE_CXX_STANDARD 17)

add_executable(lab3 main.cpp
        tests/run_all_tests.cpp
        tests/test_utils.cpp
        tests/test_heap_sorter.cpp
        tests/test_lazy_sequence.cpp
        tests/test_streams.cpp
)
