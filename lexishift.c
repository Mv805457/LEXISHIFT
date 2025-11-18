#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#define ALPHABET_SIZE 26
#define MAX_WORD_LENGTH 100
#define MAX_SUGGESTIONS 10

typedef struct TrieNode {
    struct TrieNode* children[ALPHABET_SIZE];
    int isEnd;
} TrieNode;

// Export macro
#ifdef _WIN32
#define API __declspec(dllexport)
#else
#define API
#endif

API TrieNode* createNode() {
    TrieNode* n = calloc(1, sizeof(TrieNode));
    return n;
}

API void insertTrie(TrieNode* root, const char* word) {
    TrieNode* cur = root;
    for (int i = 0; word[i]; i++) {
        int idx = tolower(word[i]) - 'a';
        if (idx < 0 || idx >= ALPHABET_SIZE) return;
        if (!cur->children[idx])
            cur->children[idx] = createNode();
        cur = cur->children[idx];
    }
    cur->isEnd = 1;
}

void collect(TrieNode* node, char* buf, int depth, char out[][100], int* count) {
    if (!node || *count >= MAX_SUGGESTIONS) return;

    if (node->isEnd) {
        buf[depth] = '\0';
        strcpy(out[*count], buf);
        (*count)++;
    }

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i]) {
            buf[depth] = 'a' + i;
            collect(node->children[i], buf, depth + 1, out, count);
        }
    }
}

API int getSuggestions(TrieNode* root, const char* prefix, char out[][100]) {
    TrieNode* cur = root;

    // traverse to prefix node
    for (int i = 0; prefix[i]; i++) {
        int idx = tolower(prefix[i]) - 'a';
        if (idx < 0 || idx >= ALPHABET_SIZE || !cur->children[idx])
            return 0;
        cur = cur->children[idx];
    }

    // collect suffixes
    char buffer[100];
    int count = 0;
    collect(cur, buffer, 0, out, &count);

    // prepend prefix to each result
    for (int i = 0; i < count; i++) {
        char full[100];
        snprintf(full, sizeof(full), "%s%s", prefix, out[i]);  
        strcpy(out[i], full);
    }

    return count;
}
