#include "avl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[])
{
    if (argc != 2) {
        puts("Укажите путь к файлу.");
        return 1;
    }

    char* pathToFile = argv[1];
    FILE* file = fopen(pathToFile, "r");
    if (!file) {
        puts("Файл не найден.");
        return 1;
    }

    Node* root = NULL;
    int counter = 0;
    char buffer[4096];

    while (fgets(buffer, sizeof(buffer), file)) {
        if (strchr(buffer, '\n') == NULL && strlen(buffer) == sizeof(buffer) - 1) {
            puts("Строка слишком длинная.");
            freeTree(root);
            fclose(file);
            return 1;
        }

        char* colon = strchr(buffer, ':');
        if (!colon) {
            puts("Некоррекные входные данные.");
            freeTree(root);
            fclose(file);
            return 1;
        }

        *colon = '\0';
        char* code = buffer;
        char* name = colon + 1;
        name[strcspn(name, "\n")] = '\0';

        if (strlen(code) != 3) {
            puts("Некорректный IATA код.");
            freeTree(root);
            fclose(file);
            return 1;
        }

        if (*name == '\0') {
            puts("Некорректное название аэропорта.");
            freeTree(root);
            fclose(file);
            return 1;
        }

        char* codeCopy = strdup(code);
        char* nameCopy = strdup(name);
        if (!codeCopy || !nameCopy) {
            puts("Ошибка выделения памяти.");
            free(codeCopy);
            free(nameCopy);
            freeTree(root);
            fclose(file);
            return 1;
        }

        if (search(root, code) != NULL) {
            printf("Аэропорт с кодом %s уже есть в базе.\n", code);
            free(codeCopy);
            free(nameCopy);
            continue;
        }

        recursiveInsert(&root, codeCopy, nameCopy);
        ;
        ++counter;
    }

    printf("Загружено %d аэропортов. Система готова к работе.\n", counter);

    char inputLine[4100];
    while (1) {
        printf("> ");
        if (!fgets(inputLine, sizeof(inputLine), stdin)) {
            break;
        }

        size_t length = strlen(inputLine);
        if (length == 0) {
            continue;
        }

        if (inputLine[length - 1] == '\n') {
            inputLine[length - 1] = '\0';
        }

        char command[7] = { 0 };
        char argument[4096] = { 0 };

        char* space = strchr(inputLine, ' ');
        if (space) {
            size_t commandLength = space - inputLine;
            if (commandLength >= sizeof(command)) {
                puts("Команда не найдена. \n");
                continue;
            }
            strncpy(command, inputLine, commandLength);
            command[commandLength] = '\0';
            strncpy(argument, space + 1, sizeof(argument) - 1);
            argument[sizeof(argument) - 1] = '\0';
        } else {
            strncpy(command, inputLine, sizeof(command) - 1);
            command[sizeof(command) - 1] = '\0';
        }

        if (strcmp(command, "find") == 0) {
            if (strlen(argument) != 3) {
                puts("Некорректный IATA код.\n");
                continue;
            }

            Node* node = search(root, argument);
            if (!node) {
                printf("Аэропорт с кодом '%s' не найден в базе.\n\n", argument);
                continue;
            }

            printf("%s → %s\n\n", argument, node->name);
        } else if (strcmp(command, "add") == 0) {
            char* colon = strchr(argument, ':');
            if (!colon) {
                puts("Некорректные входные данные.\n");
                continue;
            }

            *colon = '\0';
            char* code = argument;
            char* name = colon + 1;

            if (strlen(code) != 3) {
                puts("Некорректный IATA код.\n");
                continue;
            }

            if (*name == '\0') {
                puts("Некорректное имя аэропорта.\n");
                continue;
            }

            char* codeCopy = strdup(code);
            char* nameCopy = strdup(name);
            if (!codeCopy || !nameCopy) {
                puts("Ошибка выделения памяти.\n");
                free(codeCopy);
                free(nameCopy);
                continue;
            }

            if (search(root, code) != NULL) {
                printf("Аэропорт с кодом '%s' уже есть в базе.\n\n", code);
                free(codeCopy);
                free(nameCopy);
                continue;
            }
            int status = recursiveInsert(&root, codeCopy, nameCopy);
            if (status == 1) {
                puts("Ошибка при добавлении аэропорта\n");
                continue;
            }
            puts("");
        } else if (strcmp(command, "delete") == 0) {
            if (strlen(argument) != 3) {
                puts("Некорректный IATA код.\n");
                continue;
            }
            if (search(root, argument) == NULL) {
                printf("Аэропорта с кодом '%s' нет в базе.\n\n", argument);
                continue;
            }
            int status = recursiveRemove(&root, argument);
            if (status == 1) {
                puts("Ошибка при удалении аэропорта\n");
                continue;
            }
            puts("");
        } else if (strcmp(command, "save") == 0) {
            FILE* outFile = fopen(pathToFile, "w");
            if (!outFile) {
                puts("Ошибка при открытии файла.\n");
                continue;
            }
            int counter = 0;
            saveTree(outFile, root, &counter);
            printf("База сохранена: %d аэропортов.\n\n", counter);
            fclose(outFile);
        } else if (strcmp(command, "quit") == 0) {
            break;
        } else {
            puts("Неизвестая команда.\n");
        }
    }

    freeTree(root);
    fclose(file);
    return 0;
}
