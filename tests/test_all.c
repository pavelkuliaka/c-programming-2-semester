#include "../src/csv_processor.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int compareFiles(const char* pathToFile1, const char* pathToFile2)
{
    FILE* file1 = fopen(pathToFile1, "r");
    FILE* file2 = fopen(pathToFile2, "r");

    if (!file1 || !file2) {
        puts("File opening error");
        if (file1 != NULL) {
            fclose(file1);
        }
        if (file2 != NULL) {
            fclose(file2);
        }
        return 0;
    }

    int result = 1;
    int char1, char2;

    while (1) {
        char1 = fgetc(file1);
        char2 = fgetc(file2);

        // Оба файла закончились одновременно
        if (char1 == EOF && char2 == EOF) {
            // Файлы идентичны
            break;
        }
        // Первый файл закончился
        if (char1 == EOF && char2 != EOF) {
            // Проверяем, не идут ли во втором только \n
            while (char2 == '\n') {
                char2 = fgetc(file2);
            }
            if (char2 == EOF) {
                // Только \n в конце - считаем одинаковыми
                break;
            } else {
                result = 0;
                break;
            }
        }
        // Второй файл закончился
        if (char1 != EOF && char2 == EOF) {
            // Проверяем, не идут ли в первом только \n
            while (char1 == '\n') {
                char1 = fgetc(file1);
            }
            if (char1 == EOF) {
                // Только \n в конце - считаем одинаковыми
                break;
            } else {
                result = 0;
                break;
            }
        }
        // Символы разные
        if (char1 != char2) {
            result = 0;
            break;
        }
        // Символы одинаковые - продолжаем цикл
    }

    fclose(file1);
    fclose(file2);
    return result;
}

// Тесты для вспомогательных функций
void testIsNumber(void)
{
    puts("Testing isNumber function...");
    assert(isNumber("123") == 1);
    assert(isNumber("abc") == 0);
    assert(isNumber("") == 0);
    assert(isNumber(NULL) == 0);
    assert(isNumber("123.45") == 1);
    assert(isNumber("12.34.56") == 0);
    puts("Tests of isNumber function passed!\n");
}

void testCountCommas(void)
{
    puts("Testing countCommas function...");
    assert(countCommas("") == 0);
    assert(countCommas("a,b,c") == 2);
    assert(countCommas("a,,c") == 2);
    assert(countCommas(",,,") == 3);
    puts("Tests of countCommas function passed!\n");
}

// Тесты для главной функции
void testProcessCSV(void)
{
    puts("Testing processCSV function...\n");

    struct TestCase {
        const char* name;
        const char* input;
        const char* expected;
    } tests[] = {
        { "basic", "fixtures/basic.csv", "fixtures/basic.txt" },
        { "empty cells", "fixtures/empty_cells.csv", "fixtures/empty_cells.txt" },
        { "empty file", "fixtures/empty.csv", "fixtures/empty.txt" },
        { "header only", "fixtures/header_only.csv", "fixtures/header_only.txt" },
        { "varying columns", "fixtures/varying_columns.csv",
            "fixtures/varying_columns.txt" },
        { "whitespace handling", "fixtures/whitespace.csv",
            "fixtures/whitespace.txt" },
        { NULL, NULL, NULL }
    };

    int failed = 0;
    int skipped = 0;

    // Запускаем тесты по очереди
    for (int index = 0; tests[index].name != NULL; ++index) {
        printf("Testing the %s case...\n", tests[index].name);

        int exitCode = processCSV(tests[index].input, "out.txt");
        if (exitCode == 0) {
            int result = compareFiles("out.txt", tests[index].expected);
            if (result == 1) {
                puts("Test passed!\n");
            } else {
                puts("Test failed!");
                ++failed;
            }
        } else {
            switch (exitCode) {
            case 1:
                puts("File opening error");
                break;
            case 2:
                puts("Memory allocation error");
                break;
            case 3:
                puts("File reading error");
                break;
            default: break;
            }
            puts("Test skipped!\n");
            ++skipped;
        }
    }

    remove("out.txt");

    // Если есть упавшие тесты, завершаем с ошибкой
    if (failed > 0) {
        puts("Some tests failed!");
        assert(0);
    } else if (skipped > 0) {
        puts("Some tests were skipped due to missing files");
    }
}

int main(void)
{
    puts("Running all tests...");
    puts("====================\n");

    testIsNumber();
    testCountCommas();
    testProcessCSV();

    puts("====================");
    puts("All tests passed!");

    return 0;
}
