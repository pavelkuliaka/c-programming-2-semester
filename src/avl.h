#pragma once

#include <stdio.h>

typedef struct NodeStruct {
    struct NodeStruct* leftChild;
    struct NodeStruct* rightChild;
    char* code;
    char* name;
    int height;
} Node;

int recursiveInsert(Node** node, char* code, char* name);

int recursiveRemove(Node** node, const char* code);

Node* search(Node* node, const char* code);

void freeTree(Node* node);

void saveTree(FILE* file, Node* node, int* counter);
