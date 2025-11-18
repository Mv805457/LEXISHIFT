#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "trie.h"

TrieNode* createNode() {
    TrieNode *n = (TrieNode*)malloc(sizeof(TrieNode));
    n->isEndOfWord = 0;
    for (int i = 0; i < ALPHABET_SIZE; i++)
        n->children[i] = NULL;
    return n;
}

void insertTrie(TrieNode *root, const char *word) {
    TrieNode *cur = root;
    for (int i = 0; word[i]; i++) {
        int idx = tolower(word[i]) - 'a';
        if (idx < 0 || idx >= ALPHABET_SIZE) return;
        if (!cur->children[idx])
            cur->children[idx] = createNode();
        cur = cur->children[idx];
    }
    cur->isEndOfWord = 1;
}

void collectWords(TrieNode *node, char *buf, int depth,
                  char suggestions[][MAX_WORD_LENGTH],
                  int *count, int maxCount) {

    if (!node || *count >= maxCount) return;

    if (node->isEndOfWord) {
        buf[depth] = '\0';
        strcpy(suggestions[*count], buf);
        (*count)++;
        if (*count >= maxCount) return;
    }

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i]) {
            buf[depth] = 'a' + i;
            collectWords(node->children[i], buf, depth + 1, suggestions, count, maxCount);
        }
    }
}

int getSuggestions(TrieNode *root, const char *prefix,char suggestions[][MAX_WORD_LENGTH], int maxCount) {

    TrieNode *cur = root;
    for (int i = 0; prefix[i]; i++) {
        int idx = tolower(prefix[i]) - 'a';
        if (idx < 0 || idx >= ALPHABET_SIZE || !cur->children[idx])
            return 0;
        cur = cur->children[idx];
    }

    char buf[MAX_WORD_LENGTH];
    int count = 0;
    collectWords(cur, buf, 0, suggestions, &count, maxCount);

    for (int i = 0; i < count; i++) {
        char full[MAX_WORD_LENGTH];
        snprintf(full, sizeof(full), "%s%s", prefix, suggestions[i]);
        strcpy(suggestions[i], full);
    }

    return count;
}

int loadDictionary(TrieNode *root, const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) return 0;
    char w[MAX_WORD_LENGTH];

    while (fgets(w, sizeof(w), fp)) {
        w[strcspn(w, "\r\n")] = 0;
        if (strlen(w) > 0) insertTrie(root, w);
    }
    fclose(fp);
    return 1;
}

void appendWordToFile(const char *file, const char *w) {
    FILE *fp = fopen(file, "a");
    if (!fp) return;
    fprintf(fp, "\n%s", w);
    fclose(fp);
}
