// src/lexishift_gui.c
// Raylib GUI — Matrix Cyberpunk UI for LexiShift
// Single-file GUI that integrates with your trie.c
// Save as src/lexishift_gui.c
// Compile with Raylib's gcc (command shown after code)

#define _CRT_SECURE_NO_WARNINGS
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>

// include your trie header - make sure include path (-Iinclude) is used at compile time
#include "trie.h"

// ---------- Config ----------
static const char *DICT_PATH = "C:\\Users\\madha\\Downloads\\words_alpha (1).txt";
#define WINDOW_W 1100
#define WINDOW_H 700
#define MAX_SUGGESTIONS 10
#define FONT_SIZE 20

// ---------- Matrix drops ----------
typedef struct { int y; int speed; char ch; } Drop;
static Drop *drops = NULL;
static int drop_count = 0;

static void matrix_init(int width) {
    if (drops) free(drops);
    drop_count = width;
    drops = calloc(drop_count, sizeof(Drop));
    srand((unsigned)time(NULL));
    for (int i = 0; i < drop_count; ++i) {
        drops[i].y = GetRandomValue(-600, 600);
        drops[i].speed = GetRandomValue(4, 20);
        drops[i].ch = (char)(33 + GetRandomValue(0, 90));
    }
}
static void matrix_draw_update(void) {
    for (int i = 0; i < drop_count; ++i) {
        drops[i].y += drops[i].speed;
        if (drops[i].y > WINDOW_H + 10) {
            drops[i].y = GetRandomValue(-300, 0);
            drops[i].speed = GetRandomValue(4, 20);
            drops[i].ch = (char)(33 + GetRandomValue(0, 90));
        }
        DrawTextEx(GetFontDefault(), TextFormat("%c", drops[i].ch),
                   (Vector2){ (float)i, (float)drops[i].y }, 14.0f, 0, (Color){0,200,0,120});
    }
}

// ---------- UI helpers ----------
typedef enum { MODE_MENU=0, MODE_WRITE, MODE_ADD, MODE_DELETE } AppMode;

static void draw_button(Rectangle r, const char *text, bool hovered, bool active) {
    Color bg = active ? (Color){0,160,120,220} : hovered ? (Color){0,120,90,200} : (Color){10,10,12,220};
    DrawRectangleRounded(r, 0.12f, 6, bg);
    DrawRectangleRoundedLines(r, 0.12f, 6, (Color){0,200,170,180});

    Vector2 size = MeasureTextEx(GetFontDefault(), text, FONT_SIZE, 1);
    DrawTextEx(GetFontDefault(), text, (Vector2){ r.x + (r.width - size.x)/2, r.y + (r.height - size.y)/2 }, FONT_SIZE, 1, WHITE);
}

static bool point_in_rect(Vector2 p, Rectangle r) {
    return (p.x >= r.x && p.x <= r.x + r.width && p.y >= r.y && p.y <= r.y + r.height);
}

// ---------- Trie reload helper (rebuilds trie from file) ----------
static void reloadTrieFromFile(TrieNode **rootPtr, const char *path) {
    // free old trie? we won't attempt deep-free; instead create new root and let OS reclaim memory on exit.
    *rootPtr = createNode();
    loadDictionary(*rootPtr, path);
}

// ---------- Utility: remove word from dictionary file ----------
static bool removeWordFromFile(const char *path, const char *word) {
    FILE *f = fopen(path, "r");
    if (!f) return false;
    char **lines = NULL;
    size_t count = 0;
    char buf[512];
    while (fgets(buf, sizeof(buf), f)) {
        buf[strcspn(buf, "\r\n")] = 0;
        if (strcmp(buf, word) != 0) {
            lines = realloc(lines, (count+1)*sizeof(char*));
            lines[count] = _strdup(buf);
            count++;
        }
    }
    fclose(f);
    f = fopen(path, "w");
    if (!f) {
        for (size_t i=0;i<count;i++) free(lines[i]);
        free(lines);
        return false;
    }
    for (size_t i=0;i<count;i++) {
        fprintf(f, "%s\n", lines[i]);
        free(lines[i]);
    }
    free(lines);
    fclose(f);
    return true;
}

// ---------- Text helpers for last-word detection ----------
static void get_last_word_range(const char *s, int cursorPos, int *outStart, int *outLen) {
    int n = (int)strlen(s);
    if (cursorPos > n) cursorPos = n;
    int i = cursorPos - 1;
    while (i >= 0 && s[i] == ' ') i--;
    int end = i;
    while (i >= 0 && isalpha((unsigned char)s[i])) i--;
    int start = i + 1;
    int len = (end >= start) ? (end - start + 1) : 0;
    if (outStart) *outStart = start;
    if (outLen) *outLen = len;
}
static void select_first_word(const char *s, int *start, int *len) {
    int n = (int)strlen(s);
    int i = 0;
    while (i < n && s[i] == ' ') i++;
    int st = i;
    while (i < n && isalpha((unsigned char)s[i])) i++;
    int en = i - 1;
    if (en >= st) { *start = st; *len = en - st + 1; } else { *start = 0; *len = 0; }
}
static void select_last_word(const char *s, int *start, int *len) { get_last_word_range(s, (int)strlen(s), start, len); }

// ---------- MAIN ----------
int main(void) {
    InitWindow(WINDOW_W, WINDOW_H, "LexiShift — Matrix UI");
    SetTargetFPS(60);

    matrix_init(WINDOW_W);

    // Load trie
    TrieNode *root = createNode();
    if (!loadDictionary(root, DICT_PATH)) {
        // still continue - user can add words
    }

    AppMode mode = MODE_MENU;
    bool matrixVisible = true;

    // UI state
    Rectangle btnRects[4];
    float btnW = 220, btnH = 56;
    float gap = 20;
    float startX = 60;
    for (int i=0;i<4;i++) {
        btnRects[i] = (Rectangle){ startX + i*(btnW + gap), 40, btnW, btnH };
    }

    // typing buffers
    char paragraph[MAX_SENTENCE_LENGTH];
    paragraph[0] = '\0';
    int cursorPos = 0;
    int selStart = -1, selLen = 0;

    // suggestion storage
    char suggestions[MAX_SUGGESTIONS][MAX_WORD_LENGTH];
    int sugCount = 0;
    int highlighted = -1;

    // timing for backspace repeat handling
    double lastBackspace = 0.0;
    const double backspaceInitialDelay = 0.35; // sec
    const double backspaceRepeat = 0.06; // repeat rate when held

    // fonts/colors
    Color bg = (Color){ 2, 6, 10, 255 };
    Color panel = (Color){12,16,20,220};
    Color neon = (Color){0,255,120,255};

    // main loop
    while (!WindowShouldClose()) {
        // INPUT PROCESSING
        // Toggle matrix with M
        if (IsKeyPressed(KEY_M)) matrixVisible = !matrixVisible;

        // Menu hotkeys: ALT + 0..3 mapped only in MENU mode
        bool altDown = IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT);

        if (mode == MODE_MENU) {
            for (int d=0; d<=3; ++d) {
                int key = (d==0?KEY_ZERO: d==1?KEY_ONE: d==2?KEY_TWO:KEY_THREE);
                if (altDown && IsKeyPressed(key)) {
                    mode = (AppMode)(MODE_MENU + 1 + d); // MODE_WRITE if d==0 etc.
                    // reset typing buffers when entering write/add/delete
                    paragraph[0] = '\0'; cursorPos = 0; selStart = -1; selLen = 0;
                }
            }
        }

        // Mouse clicks on top buttons
        Vector2 mp = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            for (int i=0;i<4;i++) {
                if (point_in_rect(mp, btnRects[i])) {
                    mode = (AppMode)(MODE_MENU + 1 + i);
                    paragraph[0] = '\0'; cursorPos = 0; selStart = -1; selLen = 0;
                }
            }
        }

        // Typing-mode / Add / Delete behavior
        if (mode == MODE_WRITE || mode == MODE_ADD || mode == MODE_DELETE) {
            // Capture typed chars (raylib helper)
            int key = GetCharPressed();
            while (key > 0) {
                if ((key >= 32) && (key <= 126) && (int)strlen(paragraph) < MAX_SENTENCE_LENGTH-2) {
                    // insert at cursorPos
                    int len = strlen(paragraph);
                    memmove(paragraph + cursorPos + 1, paragraph + cursorPos, len - cursorPos + 1);
                    paragraph[cursorPos] = (char)key;
                    cursorPos += 1;
                }
                key = GetCharPressed();
            }

            // Backspace behavior: single on press, repeat when held after initial delay
            bool backPressed = IsKeyDown(KEY_BACKSPACE);
            if (IsKeyPressed(KEY_BACKSPACE)) {
                // delete one char
                if (selStart >= 0 && selLen > 0) {
                    // delete selection
                    int len = strlen(paragraph);
                    memmove(paragraph + selStart, paragraph + selStart + selLen, len - (selStart + selLen) + 1);
                    cursorPos = selStart;
                    selStart = -1; selLen = 0;
                } else if (cursorPos > 0) {
                    int len = strlen(paragraph);
                    memmove(paragraph + cursorPos - 1, paragraph + cursorPos, len - cursorPos + 1);
                    cursorPos--;
                }
                lastBackspace = GetTime();
            } else if (backPressed) {
                double now = GetTime();
                if (now - lastBackspace > backspaceInitialDelay) {
                    // controlled repeat
                    static double lastRepeat = 0;
                    if (now - lastRepeat > backspaceRepeat) {
                        if (selStart >= 0 && selLen > 0) {
                            int len = strlen(paragraph);
                            memmove(paragraph + selStart, paragraph + selStart + selLen, len - (selStart + selLen) + 1);
                            cursorPos = selStart;
                            selStart = -1; selLen = 0;
                        } else if (cursorPos > 0) {
                            int len = strlen(paragraph);
                            memmove(paragraph + cursorPos - 1, paragraph + cursorPos, len - cursorPos + 1);
                            cursorPos--;
                        }
                        lastRepeat = now;
                    }
                }
            }

            // ALT-coded text functions (works while ALT is down)
            if (altDown) {
                if (IsKeyPressed(KEY_A)) { // ALT+A -> go to beginning
                    cursorPos = 0; selStart = -1; selLen = 0;
                }
                if (IsKeyPressed(KEY_B)) { // ALT+B -> go to end
                    cursorPos = (int)strlen(paragraph); selStart = -1; selLen = 0;
                }
                if (IsKeyPressed(KEY_C)) { // ALT+C -> select first word
                    int s,l; select_first_word(paragraph, &s, &l); if (l>0) { selStart = s; selLen = l; cursorPos = s + l; } 
                }
                if (IsKeyPressed(KEY_D)) { // ALT+D -> select last word
                    int s,l; select_last_word(paragraph, &s, &l); if (l>0) { selStart = s; selLen = l; cursorPos = s + l; }
                }
                // ALT + BACKSPACE => delete entire paragraph
                if (IsKeyPressed(KEY_BACKSPACE)) {
                    paragraph[0] = '\0'; cursorPos = 0; selStart = -1; selLen = 0;
                }
            }

            // Enter key behavior:
            if (IsKeyPressed(KEY_ENTER)) {
                if (mode == MODE_ADD) {
                    // add the word in paragraph (trim)
                    char w[MAX_WORD_LENGTH]; strncpy(w, paragraph, MAX_WORD_LENGTH-1); w[MAX_WORD_LENGTH-1]=0;
                    // trim spaces
                    int L = (int)strlen(w);
                    while (L>0 && isspace((unsigned char)w[L-1])) { w[--L]=0; }
                    if (L>0) { insertTrie(root, w); appendWordToFile(DICT_PATH, w); }
                    paragraph[0]=0; cursorPos=0;
                } else if (mode == MODE_DELETE) {
                    // attempt remove word
                    char w[MAX_WORD_LENGTH]; strncpy(w, paragraph, MAX_WORD_LENGTH-1); w[MAX_WORD_LENGTH-1]=0;
                    int L = (int)strlen(w); while (L>0 && isspace((unsigned char)w[L-1])) { w[--L]=0; }
                    if (L>0) {
                        if (removeWordFromFile(DICT_PATH, w)) {
                            reloadTrieFromFile(&root, DICT_PATH);
                        }
                    }
                    paragraph[0]=0; cursorPos=0;
                } else if (mode == MODE_WRITE) {
                    // accept or do nothing (could be save)
                }
            }

            // Suggestion fetching (use last word as prefix)
            int lastStart=0,lastLen=0;
            get_last_word_range(paragraph, cursorPos, &lastStart, &lastLen);
            if (lastLen > 0) {
                char prefix[MAX_WORD_LENGTH]; int cp = lastLen < MAX_WORD_LENGTH-1 ? lastLen : MAX_WORD_LENGTH-1;
                strncpy(prefix, paragraph + lastStart, cp); prefix[cp]=0;
                for (int k=0;k<cp;k++) prefix[k]=tolower((unsigned char)prefix[k]);
                sugCount = getSuggestions(root, prefix, suggestions, MAX_SUGGESTIONS);
            } else {
                sugCount = 0;
                for (int k=0;k<MAX_SUGGESTIONS;k++) suggestions[k][0]=0;
            }

            // ALT + digits select suggestion (context-sensitive)
            if (altDown) {
                for (int d=0; d<=9; ++d) {
                    int key = (d==0?KEY_ZERO: d==1?KEY_ONE: d==2?KEY_TWO: d==3?KEY_THREE:
                               d==4?KEY_FOUR: d==5?KEY_FIVE: d==6?KEY_SIX: d==7?KEY_SEVEN:
                               d==8?KEY_EIGHT: KEY_NINE);
                    if (IsKeyPressed(key) && sugCount > 0 && d < sugCount) {
                        // replace last word with chosen suggestion
                        char newp[MAX_SENTENCE_LENGTH];
                        strncpy(newp, paragraph, lastStart);
                        newp[lastStart] = '\0';
                        strncat(newp, suggestions[d], MAX_SENTENCE_LENGTH - strlen(newp) - 1);
                        strncat(newp, " ", MAX_SENTENCE_LENGTH - strlen(newp) - 1);
                        strncpy(paragraph, newp, MAX_SENTENCE_LENGTH-1);
                        cursorPos = (int)strlen(paragraph);
                    }
                }
            }

            // Mouse click for suggestion
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && sugCount > 0) {
                // compute suggestion box coordinates (we'll draw them in draw phase)
                // handle in drawing section by checking mp
            }
        } // end typing/add/delete block

        // UI DRAW
        BeginDrawing();
        ClearBackground(BLACK);

        // matrix background
        if (matrixVisible) {
            DrawRectangle(0,0, WINDOW_W, WINDOW_H, (Color){0,0,0,200});
            matrix_draw_update();
        } else {
            ClearBackground((Color){8,8,10,255});
        }

        // top buttons
        for (int i=0;i<4;i++) {
            const char *labels[] = {
                "1) Write (ALT+0)","2) Add to DB (ALT+1)","3) Delete from DB (ALT+2)","4) Exit (ALT+3)"
            };
            bool hovered = point_in_rect(mp, btnRects[i]);
            bool active = (mode == MODE_WRITE && i==0) || (mode==MODE_ADD && i==1) || (mode==MODE_DELETE && i==2);
            draw_button(btnRects[i], labels[i], hovered, active);
        }

        // central panel
        Rectangle card = { 60, 130, WINDOW_W - 120, WINDOW_H - 220 };
        DrawRectangleRounded(card, 0.02f, 8, (Color){12,12,14,180});
        DrawRectangleRoundedLines(card, 0.02f, 8, (Color){0,255,150,80});


        // mode header
        const char *modeNames[] = {"MENU","WRITE","ADD","DELETE"};
        DrawTextEx(GetFontDefault(), TextFormat("Mode: %s", modeNames[mode-MODE_MENU]), (Vector2){80,150}, 22, 1, (Color){200,255,200,240});

        // If in menu show hint
        if (mode == MODE_MENU) {
            DrawTextEx(GetFontDefault(), "Matrix is running behind. Click a button or press ALT+0..3 to choose.", (Vector2){80,190}, 18, 1, (Color){170,170,170,200});
        } else {
            // Draw typing area + suggestions
            // Input area
            Rectangle inputBox = { 100, 210, card.width - 160, 42 };
            DrawRectangleRounded(inputBox, 0.06f, 8, (Color){6,6,8,220});
            DrawRectangleRoundedLines(inputBox, 0.06f, 8, (Color){0,255,150,40});


            // show paragraph text
            DrawTextEx(GetFontDefault(), paragraph, (Vector2){ inputBox.x + 10, inputBox.y + 8 }, 20, 1, (Color){220,220,220,255});

            // cursor
            float textW = MeasureTextEx(GetFontDefault(), paragraph, 20, 1).x;
            if (((int)(GetTime()*2)) % 2 == 0) {
                DrawRectangle(inputBox.x + 10 + textW, inputBox.y + 8, 2, 22, (Color){200,255,200,255});
            }

            // suggestion panel box
            Rectangle sugBox = { inputBox.x, inputBox.y + 64, inputBox.width, 34 * MAX_SUGGESTIONS + 12 };
            DrawRectangleRounded(sugBox, 0.06f, 8, (Color){6,6,8,200});
            DrawRectangleRoundedLines(sugBox, 0.06f, 8, (Color){0,255,150,30});


            // draw suggestions
            for (int s = 0; s < sugCount; ++s) {
                Rectangle item = { sugBox.x + 8, sugBox.y + 8 + s*34, sugBox.width - 16, 30 };
                Color itemBg = (s == highlighted) ? (Color){0,120,80,200} : (Color){10,10,12,160};
                DrawRectangleRec(item, itemBg);
                DrawTextEx(GetFontDefault(), suggestions[s], (Vector2){ item.x + 8, item.y + 6 }, 18, 1, (Color){220,220,220,255});
                // clickable
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && point_in_rect(mp, item)) {
                    // replace last word with suggestions[s]
                    int ls, ll; get_last_word_range(paragraph, cursorPos, &ls, &ll);
                    char newp[MAX_SENTENCE_LENGTH]; newp[0]=0;
                    strncpy(newp, paragraph, ls);
                    newp[ls] = '\0';
                    strncat(newp, suggestions[s], MAX_SENTENCE_LENGTH - strlen(newp) - 1);
                    strncat(newp, " ", MAX_SENTENCE_LENGTH - strlen(newp) - 1);
                    strncpy(paragraph, newp, MAX_SENTENCE_LENGTH-1);
                    cursorPos = (int)strlen(paragraph);
                }
            }

            // show controls hint
            DrawTextEx(GetFontDefault(), "ALT+0..9 picks suggestion. ALT+A/B/C/D for special edits. ALT+Backspace clears paragraph.", (Vector2){80, card.y + card.height - 40}, 14, 1, (Color){180,180,180,180});
        }

        EndDrawing();
    } // main loop

    if (drops) free(drops);
    CloseWindow();
    return 0;
}
