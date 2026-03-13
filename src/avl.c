#include "avl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void updateHeight(Node* node)
{
    if (!node) {
        return;
    }

    int leftHeight = node->leftChild ? node->leftChild->height : -1;
    int rightHeight = node->rightChild ? node->rightChild->height : -1;
    node->height = 1 + (leftHeight > rightHeight ? leftHeight : rightHeight);
}

static int calcBalanceFactor(Node* node)
{
    if (!node) {
        return 0;
    }

    int leftHeight = node->leftChild ? node->leftChild->height : -1;
    int rightHeight = node->rightChild ? node->rightChild->height : -1;
    return leftHeight - rightHeight;
}

static Node* rotateRight(Node* node)
{
    if (!node) {
        return NULL;
    }

    Node* leftChild = node->leftChild;
    if (!leftChild) {
        return node;
    }

    node->leftChild = leftChild->rightChild;
    leftChild->rightChild = node;

    updateHeight(node);
    updateHeight(leftChild);

    return leftChild;
}

static Node* rotateLeft(Node* node)
{
    if (!node) {
        return NULL;
    }

    Node* rightChild = node->rightChild;
    if (!rightChild) {
        return node;
    }

    node->rightChild = rightChild->leftChild;
    rightChild->leftChild = node;

    updateHeight(node);
    updateHeight(rightChild);

    return rightChild;
}

static Node* balance(Node* node)
{
    if (!node) {
        return node;
    }

    updateHeight(node);
    int balanceFactor = calcBalanceFactor(node);

    if (balanceFactor > 1) {
        if (calcBalanceFactor(node->leftChild) < 0) {
            node->leftChild = rotateLeft(node->leftChild);
        }
        return rotateRight(node);
    } else if (balanceFactor < -1) {
        if (calcBalanceFactor(node->rightChild) > 0) {
            node->rightChild = rotateRight(node->rightChild);
        }
        return rotateLeft(node);
    }

    return node;
}

int recursiveInsert(Node** node, char* code, char* name)
{
    if (!code || !name) {
        return 1;
    }

    if (!*node) {
        Node* newNode = malloc(sizeof(Node));
        if (!newNode) {
            return 1;
        }

        newNode->leftChild = NULL;
        newNode->rightChild = NULL;
        newNode->height = 0;
        newNode->code = code;
        newNode->name = name;
        *node = newNode;
        return 0;
    }

    int comparison = strcmp(code, (*node)->code);
    if (comparison < 0) {
        int status = recursiveInsert(&(*node)->leftChild, code, name);
        if (status != 0) {
            return status;
        }
    } else if (comparison > 0) {
        int status = recursiveInsert(&(*node)->rightChild, code, name);
        if (status != 0) {
            return status;
        }
    } else {
        free(code);
        free(name);
        return 1;
    }

    *node = balance(*node);
    return 0;
}

static Node* findMin(Node* node)
{
    while (node && node->leftChild) {
        node = node->leftChild;
    }
    return node;
}

int recursiveRemove(Node** node, const char* code)
{
    if (!node || !*node) {
        return 1;
    }

    int comparison = strcmp(code, (*node)->code);
    if (comparison < 0) {
        int status = recursiveRemove(&(*node)->leftChild, code);
        if (status != 0) {
            return status;
        }
    } else if (comparison > 0) {
        int status = recursiveRemove(&(*node)->rightChild, code);
        if (status != 0) {
            return status;
        }
    } else {
        Node* toDelete = *node;
        if (!toDelete->leftChild) {
            *node = toDelete->rightChild;
            free(toDelete->code);
            free(toDelete->name);
            free(toDelete);
        } else if (!toDelete->rightChild) {
            *node = toDelete->leftChild;
            free(toDelete->code);
            free(toDelete->name);
            free(toDelete);
        } else {
            Node* minRight = findMin(toDelete->rightChild);

            char* newCode = strdup(minRight->code);
            char* newName = strdup(minRight->name);
            if (!newCode || !newName) {
                free(newCode);
                free(newName);
                return 1;
            }

            free(toDelete->code);
            free(toDelete->name);
            toDelete->code = newCode;
            toDelete->name = newName;

            int status = recursiveRemove(&toDelete->rightChild, minRight->code);
            if (status != 0) {
                return status;
            }
        }
    }

    if (*node) {
        *node = balance(*node);
    }
    return 0;
}

Node* search(Node* node, const char* code)
{
    if (!node) {
        return NULL;
    }

    int comparison = strcmp(code, node->code);
    if (comparison == 0) {
        return node;
    } else if (comparison < 0) {
        return search(node->leftChild, code);
    } else {
        return search(node->rightChild, code);
    }
}

void freeTree(Node* node)
{
    if (!node) {
        return;
    }

    freeTree(node->leftChild);
    freeTree(node->rightChild);
    free(node->code);
    free(node->name);
    free(node);
}

void saveTree(FILE* file, Node* node, int* counter)
{
    if (!node) {
        return;
    }

    saveTree(file, node->leftChild, counter);
    fprintf(file, "%s:%s\n", node->code, node->name);
    ++(*counter);
    saveTree(file, node->rightChild, counter);
}
