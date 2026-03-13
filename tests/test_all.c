#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/avl.h"

static int verifyAVLNode(Node* node, const char** lastCode)
{
    if (!node)
        return -1;

    int leftHeight = verifyAVLNode(node->leftChild, lastCode);

    if (*lastCode && strcmp(*lastCode, node->code) >= 0) {
        printf("Order error: %s >= %s\n", *lastCode, node->code);
        exit(1);
    }
    *lastCode = node->code;

    int rightHeight = verifyAVLNode(node->rightChild, lastCode);

    int expectedHeight = 1 + (leftHeight > rightHeight ? leftHeight : rightHeight);
    if (node->height != expectedHeight) {
        printf("Invalid height for node %s: expected %d, got %d\n",
            node->code, expectedHeight, node->height);
        exit(1);
    }

    int balance = leftHeight - rightHeight;
    if (balance < -1 || balance > 1) {
        printf("Balance violation at node %s: bf = %d\n", node->code, balance);
        exit(1);
    }

    return expectedHeight;
}

static void verifyAVL(Node* root)
{
    const char* last = NULL;
    verifyAVLNode(root, &last);
}

static int compareFiles(const char* fileName1, const char* fileName2)
{
    FILE* file1 = fopen(fileName1, "r");
    FILE* file2 = fopen(fileName2, "r");
    if (!file1 || !file2) {
        perror("fopen");
        if (file1)
            fclose(file1);
        if (file2)
            fclose(file2);
        exit(1);
    }

    char line1[256], line2[256];
    int result = 0;

    while (1) {
        char* row1 = fgets(line1, sizeof(line1), file1);
        char* row2 = fgets(line2, sizeof(line2), file2);

        if (!row1 && !row2) {
            break;
        }
        if (!row1 || !row2) {
            result = -1;
            break;
        }
        if (strcmp(line1, line2) != 0) {
            result = -1;
            break;
        }
    }

    fclose(file1);
    fclose(file2);
    return result;
}

void testInsertAndSearch()
{
    puts("Test 1: insertion and search...");
    Node* root = NULL;

    int status = recursiveInsert(&root, strdup("b"), strdup("B element"));
    assert(status == 0);
    verifyAVL(root);

    status = recursiveInsert(&root, strdup("a"), strdup("A element"));
    assert(status == 0);
    verifyAVL(root);

    status = recursiveInsert(&root, strdup("c"), strdup("C element"));
    assert(status == 0);
    verifyAVL(root);

    status = recursiveInsert(&root, strdup("d"), strdup("D element"));
    assert(status == 0);
    verifyAVL(root);

    Node* found = search(root, "a");
    assert(found && strcmp(found->name, "A element") == 0);
    found = search(root, "c");
    assert(found && strcmp(found->name, "C element") == 0);
    found = search(root, "d");
    assert(found && strcmp(found->name, "D element") == 0);

    found = search(root, "e");
    assert(found == NULL);

    freeTree(root);
    puts("Test 1 passed.\n");
}

void testDuplicateInsert()
{
    puts("Test 2: attempt to insert a duplicate...");
    Node* root = NULL;

    recursiveInsert(&root, strdup("b"), strdup("B element"));
    verifyAVL(root);

    char* duplicateCode = strdup("b");
    char* duplicateName = strdup("B duplicate");
    int status = recursiveInsert(&root, duplicateCode, duplicateName);
    assert(status == 1);
    verifyAVL(root);

    freeTree(root);
    puts("Test 2 passed.\n");
}

void testRemoveExisting()
{
    puts("Test 3: remove an existing element...");
    Node* root = NULL;

    recursiveInsert(&root, strdup("b"), strdup("B element"));
    recursiveInsert(&root, strdup("a"), strdup("A element"));
    recursiveInsert(&root, strdup("c"), strdup("C element"));
    verifyAVL(root);

    int status = recursiveRemove(&root, "b");
    assert(status == 0);
    verifyAVL(root);
    assert(search(root, "b") == NULL);
    assert(search(root, "a") != NULL);
    assert(search(root, "c") != NULL);

    freeTree(root);
    puts("Test 3 passed.\n");
}

void testRemoveNonexistent()
{
    printf("Test 4: remove a non‑existent element...\n");
    Node* root = NULL;

    recursiveInsert(&root, strdup("a"), strdup("A element"));
    recursiveInsert(&root, strdup("c"), strdup("C element"));
    verifyAVL(root);

    int status = recursiveRemove(&root, "x");
    assert(status == 1);
    verifyAVL(root);

    freeTree(root);
    puts("Test 4 passed.\n");
}

void testRemoveNodeWithTwoChildren()
{
    printf("Test 5: remove a node with two children...\n");
    Node* root = NULL;

    recursiveInsert(&root, strdup("b"), strdup("B element"));
    recursiveInsert(&root, strdup("a"), strdup("A element"));
    recursiveInsert(&root, strdup("d"), strdup("D element"));
    recursiveInsert(&root, strdup("c"), strdup("C element"));

    verifyAVL(root);

    int status = recursiveRemove(&root, "b");
    assert(status == 0);
    verifyAVL(root);
    assert(search(root, "b") == NULL);

    assert(search(root, "a") != NULL);
    assert(search(root, "c") != NULL);
    assert(search(root, "d") != NULL);

    freeTree(root);
    puts("Test 5 passed.\n");
}

void testSaveTree()
{
    printf("Test 6: save tree to file...\n");
    Node* root = NULL;

    recursiveInsert(&root, strdup("b"), strdup("B element"));
    recursiveInsert(&root, strdup("a"), strdup("A element"));
    recursiveInsert(&root, strdup("c"), strdup("C element"));
    verifyAVL(root);

    const char* outFile = "test_output.txt";
    const char* expectedFile = "expected.txt";
    FILE* file = fopen(outFile, "w");
    assert(file != NULL);
    int counter = 0;
    saveTree(file, root, &counter);
    fclose(file);

    file = fopen(expectedFile, "w");
    assert(file != NULL);
    fprintf(file, "a:A element\n");
    fprintf(file, "b:B element\n");
    fprintf(file, "c:C element\n");
    fclose(file);

    assert(compareFiles(outFile, expectedFile) == 0);

    remove(outFile);
    remove(expectedFile);
    freeTree(root);
    puts("Test 6 passed.\n");
}

void testFreeTree()
{
    printf("Test 7: free tree...\n");
    Node* root = NULL;
    recursiveInsert(&root, strdup("x"), strdup("X element"));
    recursiveInsert(&root, strdup("y"), strdup("Y element"));
    recursiveInsert(&root, strdup("z"), strdup("Z element"));
    freeTree(root);
    puts("Test 7 passed.\n");
}

int main()
{
    testInsertAndSearch();
    testDuplicateInsert();
    testRemoveExisting();
    testRemoveNonexistent();
    testRemoveNodeWithTwoChildren();
    testSaveTree();
    testFreeTree();

    puts("All tests passed successfully.");
    return 0;
}
