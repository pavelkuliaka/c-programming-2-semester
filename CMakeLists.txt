cmake_minimum_required(VERSION 3.10)
project(FlightRegistration)

# Исполняемые файлы в build/bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(program src/main.c src/avl.c)

add_executable(test_all tests/test_all.c src/avl.c)

# test_all тоже в build/bin
set_target_properties(test_all PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
