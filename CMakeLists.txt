cmake_minimum_required(VERSION 3.10)
project(statesDistribution)

# Исполняемые файлы в build/bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(program src/main.c)
