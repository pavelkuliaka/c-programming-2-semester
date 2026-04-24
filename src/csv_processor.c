#include "csv_processor.h"
#include "internal.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Функция для проверки типа данных
int isNumber(const char* string)
{
    if (string == NULL || *string == '\0') {
        return 0;
    }

    int hasDecimal = 0;
    for (const char* pointer = string; *pointer; ++pointer) {
        if (*pointer == '.') {
            if (hasDecimal) {
                return 0;
            }
            hasDecimal = 1;
        } else if (!isdigit(*pointer)) {
            return 0;
        }
    }
    return 1;
}

// Функция для подсчета количества запятых в строке
int countCommas(const char* string)
{
    if (string == NULL || *string == '\0') {
        return 0;
    }

    int count = 0;
    for (const char* pointer = string; *pointer; ++pointer) {
        if (*pointer == ',') {
            ++count;
        }
    }
    return count;
}

// Функция для чтения файла в буффер
static char* readFileContents(const char* path, size_t* outSize)
{
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    if (fileSize < 0) {
        fclose(file);
        return NULL;
    }
    fseek(file, 0, SEEK_SET);

    char* buffer = malloc(fileSize + 1);
    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }

    size_t bytesRead = fread(buffer, 1, fileSize, file);
    if (bytesRead != fileSize) {
        free(buffer);
        fclose(file);
        return NULL;
    }
    buffer[fileSize] = '\0';
    fclose(file);

    *outSize = (size_t)fileSize;
    return buffer;
}

// Функция для подсчета размера таблицы
static void countDimensions(const char* buffer, unsigned int* rows, unsigned int* maxColumns)
{
    *rows = 0;
    *maxColumns = 0;

    char* bufferCopy = malloc(strlen(buffer) + 1);
    if (bufferCopy == NULL) {
        return;
    }
    strcpy(bufferCopy, buffer);

    char* context = NULL;
    char* line = strtok_r(bufferCopy, "\n", &context);

    while (line != NULL) {
        ++(*rows);
        int commas = countCommas(line);
        int columnsInLine = commas + 1;
        if (columnsInLine > *maxColumns) {
            *maxColumns = columnsInLine;
        }
        line = strtok_r(NULL, "\n", &context);
    }

    free(bufferCopy);
}

// Функция для создания таблицы
static char*** allocateTable(unsigned int rows, unsigned int columns)
{
    char*** table = malloc(sizeof(char**) * rows);
    if (table == NULL) {
        return NULL;
    }

    for (unsigned int rowIndex = 0; rowIndex < rows; ++rowIndex) {
        table[rowIndex] = calloc(columns, sizeof(char*));
        if (table[rowIndex] == NULL) {
            for (unsigned int index = 0; index < rowIndex; ++index) {
                free(table[index]);
            }
            free(table);
            return NULL;
        }
    }

    return table;
}

// Функция для освобождения таблицы
static void freeTable(char*** table, unsigned int rows, unsigned int columns)
{
    if (table == NULL) {
        return;
    }

    for (unsigned int rowIndex = 0; rowIndex < rows; ++rowIndex) {
        if (table[rowIndex] != NULL) {
            for (unsigned int columnIndex = 0; columnIndex < columns; ++columnIndex) {
                free(table[rowIndex][columnIndex]);
            }
            free(table[rowIndex]);
        }
    }
    free(table);
}

// Функция для очистки памяти
static void cleanup(char* buffer, char*** table, unsigned int* columnSizes,
    unsigned int rows, unsigned int columns)
{
    free(buffer);
    free(columnSizes);
    if (table)
        freeTable(table, rows, columns);
}

// Функция для разбиения строки по столбцам
static unsigned int parseCSVLine(const char* line, char** outValues,
    unsigned int maxColumns, unsigned int* columnSizes)
{
    unsigned int columnIndex = 0;
    size_t lineLength = strlen(line);

    char* lineCopy = malloc(lineLength + 1);
    if (lineCopy == NULL) {
        return 0;
    }
    strcpy(lineCopy, line);

    char* start = lineCopy;
    char* current = lineCopy;

    for (; *current != '\0'; ++current) {
        if (*current == ',') {
            *current = '\0';

            char* value = start;
            while (isspace(*value)) {
                ++value;
            }

            char* end = value + strlen(value) - 1;
            while (end > value && isspace(*end)) {
                --end;
            }
            *(end + 1) = '\0';

            if (columnIndex < maxColumns) {
                outValues[columnIndex] = strdup(value);
                size_t length = strlen(value);
                if (length > columnSizes[columnIndex]) {
                    columnSizes[columnIndex] = length;
                }
                ++columnIndex;
            }

            start = current + 1;
        }
    }

    if (columnIndex < maxColumns) {
        char* value = start;
        while (isspace(*value)) {
            ++value;
        }

        char* end = value + strlen(value) - 1;
        while (end > value && isspace(*end)) {
            --end;
        }
        *(end + 1) = '\0';

        outValues[columnIndex] = strdup(value);
        size_t length = strlen(value);
        if (length > columnSizes[columnIndex]) {
            columnSizes[columnIndex] = length;
        }
        ++columnIndex;
    }

    while (columnIndex < maxColumns) {
        outValues[columnIndex] = strdup("");
        ++columnIndex;
    }

    free(lineCopy);
    return columnIndex;
}

// Функция для отрисовки таблицы
static void drawTable(FILE* file, char*** table, unsigned int rows,
    unsigned int columns, unsigned int* columnSizes)
{
    fputs("+", file);
    for (unsigned int columnIndex = 0; columnIndex < columns; ++columnIndex) {
        for (unsigned int index = 0; index < columnSizes[columnIndex] + 2; ++index) {
            fputs("=", file);
        }
        if (columnIndex < columns - 1) {
            fputs("+", file);
        }
    }
    fputs("+\n", file);

    for (unsigned int rowIndex = 0; rowIndex < rows; ++rowIndex) {
        fputs("|", file);

        for (unsigned int columnIndex = 0; columnIndex < columns; ++columnIndex) {
            char* value = table[rowIndex][columnIndex] ? table[rowIndex][columnIndex] : "";
            unsigned int width = columnSizes[columnIndex];

            fputs(" ", file);

            if (rowIndex == 0) {
                fprintf(file, "%-*s", width, value);
            } else {
                if (isNumber(value)) {
                    fprintf(file, "%*s", width, value);
                } else {
                    fprintf(file, "%-*s", width, value);
                }
            }

            fputs(" ", file);

            if (columnIndex < columns - 1) {
                fputs("│", file);
            }
        }
        fputs("|\n", file);

        if (rowIndex < rows - 1) {
            if (rowIndex == 0) {
                fputs("+", file);
                for (unsigned int columnIndex = 0; columnIndex < columns; ++columnIndex) {
                    for (unsigned int index = 0; index < columnSizes[columnIndex] + 2; ++index) {
                        fputs("=", file);
                    }
                    if (columnIndex < columns - 1) {
                        fputs("+", file);
                    }
                }
                fputs("+\n", file);
            } else {
                fputs("+", file);
                for (unsigned int columnIndex = 0; columnIndex < columns; ++columnIndex) {
                    for (unsigned int index = 0; index < columnSizes[columnIndex] + 2; ++index) {
                        fputs("-", file);
                    }
                    if (columnIndex < columns - 1) {
                        fputs("+", file);
                    }
                }
                fputs("+\n", file);
            }
        }
    }

    fputs("+", file);
    for (unsigned int columnIndex = 0; columnIndex < columns; ++columnIndex) {
        if (rows == 1) {
            for (unsigned int index = 0; index < columnSizes[columnIndex] + 2; ++index) {
                fputs("=", file);
            }
        } else {
            for (unsigned int index = 0; index < columnSizes[columnIndex] + 2; ++index) {
                fputs("-", file);
            }
        }
        if (columnIndex < columns - 1) {
            fputs("+", file);
        }
    }
    fputs("+\n", file);
}

/*
    Коды возврата
    0 - завершено без ошибок
    1 - ошибка при открытии файла
    2 - ошибка выделения памяти
    3 - ошибка чтения файла

*/
int processCSV(const char* pathToCSV, const char* pathToTextFile)
{
    char* buffer = NULL;
    char*** table = NULL;
    unsigned int* columnsSizes = NULL;
    unsigned int rowsNumber = 0;
    unsigned int maxColumns = 0;

    size_t fileSize;
    buffer = readFileContents(pathToCSV, &fileSize);
    if (buffer == NULL) {
        cleanup(buffer, table, columnsSizes, rowsNumber, maxColumns);
        return 1;
    }

    countDimensions(buffer, &rowsNumber, &maxColumns);

    if (rowsNumber == 0 || maxColumns == 0) {
        FILE* textFile = fopen(pathToTextFile, "w");
        if (textFile == NULL) {
            cleanup(buffer, table, columnsSizes, rowsNumber, maxColumns);
            return 1;
        }
        fclose(textFile);
        cleanup(buffer, table, columnsSizes, rowsNumber, maxColumns);
        return 0;
    }

    table = allocateTable(rowsNumber, maxColumns);
    if (table == NULL) {
        cleanup(buffer, table, columnsSizes, rowsNumber, maxColumns);
        return 2;
    }

    columnsSizes = calloc(maxColumns, sizeof(unsigned int));
    if (columnsSizes == NULL) {
        cleanup(buffer, table, columnsSizes, rowsNumber, maxColumns);
        return 2;
    }

    char* bufferCopy = malloc(fileSize + 1);
    if (bufferCopy == NULL) {
        cleanup(buffer, table, columnsSizes, rowsNumber, maxColumns);
        return 2;
    }
    memcpy(bufferCopy, buffer, fileSize);
    bufferCopy[fileSize] = '\0';

    char* context = NULL;
    char* line = strtok_r(bufferCopy, "\n", &context);
    unsigned int rowIndex = 0;

    while (line != NULL && rowIndex < rowsNumber) {
        parseCSVLine(line, table[rowIndex], maxColumns, columnsSizes);
        line = strtok_r(NULL, "\n", &context);
        ++rowIndex;
    }

    free(bufferCopy);

    FILE* textFile = fopen(pathToTextFile, "w");
    if (textFile == NULL) {
        cleanup(buffer, table, columnsSizes, rowsNumber, maxColumns);
        return 1;
    }

    drawTable(textFile, table, rowsNumber, maxColumns, columnsSizes);
    fclose(textFile);

    cleanup(buffer, table, columnsSizes, rowsNumber, maxColumns);
    return 0;
}
