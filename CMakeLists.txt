cmake_minimum_required(VERSION 3.10)
project(TableCSV)

# Исполняемые файлы в build/bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(program src/main.c src/csv_processor.c)

add_executable(test_all tests/test_all.c src/csv_processor.c)

# test_all тоже в build/bin
set_target_properties(test_all PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Копируем fixtures из tests/fixtures в build/bin/fixtures
add_custom_command(
    TARGET test_all POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures
        ${CMAKE_CURRENT_BINARY_DIR}/bin/fixtures
    COMMENT "Copying fixtures to bin directory"
)

target_compile_definitions(test_all PRIVATE FIXTURES_DIR="${CMAKE_CURRENT_BINARY_DIR}/bin/fixtures/")
