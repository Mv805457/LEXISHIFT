#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>
#include <windows.h>

#include "input.h"
#include "trie.h"
#include "ui.h"

/* ------------------------------------------------------------------
   Helper: delete a range [start, end) inside sentence
------------------------------------------------------------------ */
static void delete_range(char *sentence, int *len, int start, int end)
{
    if (end > *len) end = *len;
    if (start >= end) return;

    memmove(sentence + start, sentence + end, (*len - end) + 1);
    *len -= (end - start);
}

/* ------------------------------------------------------------------
   Helper: get last word relative to cursorPos
------------------------------------------------------------------ */
static void get_last_word_range(const char *sentence, int cursorPos, int *outStart, int *outLen)
{
    int n = (int)strlen(sentence);
    if (cursorPos > n) cursorPos = n;

    int i = cursorPos - 1;
    while (i >= 0 && sentence[i] == ' ') i--;

    int end = i;
    while (i >= 0 && isalpha((unsigned char)sentence[i])) i--;

    int start = i + 1;
    int len = (end >= start) ? (end - start + 1) : 0;

    if (outStart) *outStart = start;
    if (outLen) *outLen = len;
}

/* ----------------------------------------------------------
   Map ALT+digit to suggestion index 0..9
---------------------------------------------------------- */
static int alt_digit_to_index(int key)
{
    if (key >= '0' && key <= '9')
        return key - '0';
    return -1;
}

/* ----------------------------------------------------------
   Insert text at index pos
---------------------------------------------------------- */
static void insert_at(char *sentence, int *len, int pos, const char *text)
{
    int slen = (int)strlen(text);

    if (*len + slen >= MAX_SENTENCE_LENGTH - 1)
        return;

    memmove(sentence + pos + slen, sentence + pos, (*len - pos) + 1);
    memcpy(sentence + pos, text, slen);

    *len += slen;
}

/* ----------------------------------------------------------
   Small UI helper: redraw the simple console UI
   This is provided here because the project previously
   used ui_redraw() — we implement it to avoid extra deps.
---------------------------------------------------------- */
static void ui_redraw(const char *sentence, int cursorPos, int selStart, int selEnd,
                      char suggestions[][MAX_WORD_LENGTH], int sugCount, int highlightedTab)
{
    // clear
    system("cls");

    setColor(2); printf("====================================================\n");
    setColor(10); printf("                  WORD PROMPTER – CLEAN MODE\n");
    setColor(2); printf("====================================================\n\n");
    setColor(7);

    // Input line label
    printf("Input: ");

    // Print sentence with selection highlighted (if any)
    int n = (int)strlen(sentence);
    for (int i = 0; i < n; ++i) {
        if (selStart >= 0 && i >= selStart && i < selEnd) {
            setColor(12); putchar(sentence[i]); setColor(7);
        } else {
            putchar(sentence[i]);
        }
    }
    putchar('\n');
    putchar('\n');

    // Suggestions block
    setColor(10); printf("Suggestions (ALT+digit picks, TAB cycles):\n");
    setColor(7);
    for (int i = 0; i < MAX_SUGGESTIONS; ++i) {
        if (i < sugCount) {
            if (i == highlightedTab) {
                setColor(11); printf(" %d) %s\n", i, suggestions[i]); setColor(7);
            } else {
                printf(" %d) %s\n", i, suggestions[i]);
            }
        } else {
            printf(" %d)\n", i);
        }
    }

    printf("\n");
    setColor(8);
    printf("Hints: TAB=cycle suggestions, ALT+digit=pick, Shift+A=start, Shift+B=select last word, Shift+C=start of last word, Shift+D=delete\n");
    setColor(7);

    // Move console cursor to where the user expects typing to happen.
    // "Input: " is 7 characters long.
    COORD pos;
    pos.X = 7 + cursorPos;
    pos.Y = 4; // lines above: 0.. header lines
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

/* ----------------------------------------------------------
   Small UI helper: show final sentence and wait for key
---------------------------------------------------------- */
static void ui_show_final_and_wait(const char *sentence)
{
    system("cls");
    setColor(2); printf("====================================================\n");
    setColor(10); printf("               FINAL SENTENCE (ALT+digit picks)\n");
    setColor(2); printf("====================================================\n\n");
    setColor(7);
    printf("%s\n\n", sentence);
    setColor(7);
    printf("Press any key to continue...");
    getch();
}

/* ----------------------------------------------------------
   MAIN TYPING SYSTEM
---------------------------------------------------------- */
void typeSentence(TrieNode *root, const char *dictPath)
{
    char sentence[MAX_SENTENCE_LENGTH] = "";
    int sentenceLen = 0;
    int cursorPos = 0;

    int selStart = -1, selEnd = -1;

    char suggestions[MAX_SUGGESTIONS][MAX_WORD_LENGTH];
    int sugCount = 0;

    int tabIndex = -1;

    while (1)
    {
        /* ----- compute prefix ----- */
        int lwStart = 0, lwLen = 0;
        get_last_word_range(sentence, cursorPos, &lwStart, &lwLen);

        char prefix[MAX_WORD_LENGTH] = "";

        if (lwLen > 0)
        {
            int pfx = lwLen < MAX_WORD_LENGTH - 1 ? lwLen : MAX_WORD_LENGTH - 1;

            strncpy(prefix, sentence + lwStart, pfx);
            prefix[pfx] = '\0';

            for (int i = 0; i < pfx; i++)
                prefix[i] = (char)tolower((unsigned char)prefix[i]);

            /* <-- FIXED: call the trie function name used in trie.c (getSuggestions) */
            sugCount = getSuggestions(root, prefix, suggestions, MAX_SUGGESTIONS);
        }
        else
        {
            sugCount = 0;
            for (int i = 0; i < MAX_SUGGESTIONS; i++)
                suggestions[i][0] = '\0';
        }


        /* ----- draw UI ----- */
        ui_redraw(sentence, cursorPos, selStart, selEnd, suggestions, sugCount, tabIndex);


        /* ----- read key ----- */
        int ch = getch();


        /* ----- SHIFT HOTKEYS (uppercase) ------ */
        if (ch == 'A')
        {
            cursorPos = 0;
            selStart = selEnd = -1;
            tabIndex = -1;
            continue;
        }

        if (ch == 'C')
        {
            get_last_word_range(sentence, cursorPos, &lwStart, &lwLen);
            cursorPos = (lwLen > 0 ? lwStart : sentenceLen);
            selStart = selEnd = -1;
            tabIndex = -1;
            continue;
        }

        if (ch == 'B')
        {
            get_last_word_range(sentence, cursorPos, &lwStart, &lwLen);

            if (lwLen > 0)
            {
                selStart = lwStart;
                selEnd = lwStart + lwLen;
                cursorPos = selEnd;
            }
            else selStart = selEnd = -1;

            tabIndex = -1;
            continue;
        }

        if (ch == 'D')
        {
            sentence[0] = '\0';
            sentenceLen = 0;
            cursorPos = 0;
            selStart = selEnd = -1;
            tabIndex = -1;
            continue;
        }


        /* ----- ALT + DIGIT (in Windows) ----- */
        if (ch == 0 || ch == 224)
        {
            int ext = getch();

            /* Arrow keys */
            if (ext == 75) {  // left arrow
                if (cursorPos > 0) cursorPos--;
                selStart = selEnd = -1;
            }
            else if (ext == 77) { // right arrow
                if (cursorPos < sentenceLen) cursorPos++;
                selStart = selEnd = -1;
            }
            else if (ext >= '0' && ext <= '9') {
                int idx = alt_digit_to_index(ext);
                if (idx >= 0 && idx < sugCount)
                {
                    get_last_word_range(sentence, cursorPos, &lwStart, &lwLen);
                    if (lwLen > 0)
                        delete_range(sentence, &sentenceLen, lwStart, lwStart + lwLen);
                    cursorPos = lwStart;

                    insert_at(sentence, &sentenceLen, cursorPos, suggestions[idx]);
                    cursorPos += (int)strlen(suggestions[idx]);

                    if (sentenceLen < MAX_SENTENCE_LENGTH - 2) {
                        insert_at(sentence, &sentenceLen, cursorPos, " ");
                        cursorPos++;
                    }
                }
            }

            continue;
        }


        /* ----- TAB autocomplete cycling ----- */
        if (ch == 9)
        {
            if (sugCount > 0)
            {
                tabIndex++;
                if (tabIndex >= sugCount) tabIndex = 0;

                get_last_word_range(sentence, cursorPos, &lwStart, &lwLen);
                if (lwLen > 0)
                    delete_range(sentence, &sentenceLen, lwStart, lwStart + lwLen);
                cursorPos = lwStart;

                insert_at(sentence, &sentenceLen, cursorPos, suggestions[tabIndex]);
                cursorPos += (int)strlen(suggestions[tabIndex]);

                if (sentenceLen < MAX_SENTENCE_LENGTH - 2) {
                    insert_at(sentence, &sentenceLen, cursorPos, " ");
                    cursorPos++;
                }
            }
            continue;
        }


        /* ----- ENTER ends ---- */
        if (ch == 13)
            break;


        /* ----- BACKSPACE ----- */
        if (ch == 8)
        {
            if (selStart >= 0)
            {
                delete_range(sentence, &sentenceLen, selStart, selEnd);
                cursorPos = selStart;
                selStart = selEnd = -1;
            }
            else if (cursorPos > 0)
            {
                delete_range(sentence, &sentenceLen, cursorPos - 1, cursorPos);
                cursorPos--;
            }
            continue;
        }


        /* ----- PRINTABLE CHAR ----- */
        if (isprint((unsigned char)ch))
        {
            if (selStart >= 0)
            {
                delete_range(sentence, &sentenceLen, selStart, selEnd);
                cursorPos = selStart;
                selStart = selEnd = -1;
            }

            char s[2] = { (char)ch, '\0' };
            insert_at(sentence, &sentenceLen, cursorPos, s);
            cursorPos++;
            continue;
        }

        /* otherwise ignore key and loop */
    }

    ui_show_final_and_wait(sentence);
}
