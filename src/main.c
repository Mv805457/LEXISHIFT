#include <stdio.h>
#include <windows.h>
#include <conio.h>

#include "../include/trie.h"
#include "../include/input.h"
#include "../include/matrix_intro.h"
#include "../include/ui.h"

int main() {
    system("mode con: cols=120 lines=40");

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // MATRIX INTRO FIRST
    matrix_fullscreen_intro();

    TrieNode *root = createNode();
    const char *dictPath = "C:\\Users\\madha\\Downloads\\words_alpha (1).txt";

    printf("Loading dictionary: %s ...\n", dictPath);
    if (!loadDictionary(root, dictPath)) {
        printf("Failed to load dictionary.\n");
        return 1;
    }

    while (1) {
        system("cls");
        printf("------------ LEXISHIFT ------------\n");
        printf("1) Type a sentence\n");
        printf("2) Add a new word\n");
        printf("3) Exit\n\n");
        printf("Choice: ");

        int choice = 0;
        if (scanf("%d", &choice) != 1) { while (getchar()!='\n'); continue; }
        getchar();

        if (choice == 1) {
            typeSentence(root, dictPath);
            printf("\nPress any key to return..."); getch();
        }
        else if (choice == 2) {
            char w[100];
            printf("Enter new word: ");
            if (scanf("%99s", w) == 1) {
                insertTrie(root, w);
                appendWordToFile(dictPath, w);
                printf("Added.\n");
            }
            while (getchar() != '\n');
            printf("Press a key..."); getch();
        }
        else if (choice == 3) break;
        else {
            printf("Invalid.\n"); getch();
        }
    }

    return 0;
}
