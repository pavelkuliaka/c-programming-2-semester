#include "csv_processor.h"
#include <stdio.h>

int main(void)
{
    int exitCode = processCSV("input.csv", "output.txt");
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
    default:
        return exitCode;
    }
    return 1;
}
