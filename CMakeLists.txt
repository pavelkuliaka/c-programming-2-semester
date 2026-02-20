cmake_minimum_required(VERSION 3.10)
project(TableCSV)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out)

add_executable(program src/main.c src/csv_processor.c)

enable_testing()

add_executable(test_all tests/test_all.c src/csv_processor.c)

set_target_properties(test_all PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/tests)

target_compile_definitions(test_all PRIVATE 
    FIXTURES_DIR="${CMAKE_SOURCE_DIR}/tests/fixtures/")

add_test(NAME AllTests COMMAND test_all 
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests)