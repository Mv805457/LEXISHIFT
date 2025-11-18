#ifndef TRIE_H
#define TRIE_H

#define ALPHABET_SIZE 26
#define MAX_WORD_LENGTH 100
#define MAX_SUGGESTIONS 10
#define MAX_SENTENCE_LENGTH 1024

typedef struct TrieNode {
    struct TrieNode *children[ALPHABET_SIZE];
    int isEndOfWord;
} TrieNode;

// Trie functions
TrieNode* createNode();
void insertTrie(TrieNode *root, const char *word);
int wordExistsTrie(TrieNode *root, const char *word);
int getSuggestions(TrieNode *root, const char *prefix, char suggestions[][MAX_WORD_LENGTH], int maxCount);

// File operations
int loadDictionary(TrieNode *root, const char *path);
void appendWordToFile(const char *file, const char *w);

#endif
