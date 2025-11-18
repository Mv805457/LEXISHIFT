// src/launcher.c
// Raylib Matrix Launcher for LexiShift
// This launcher runs:
// 1. Console (lexishift.exe)
// 2. Raylib GUI (lexishift_gui.exe)
// 3. Python GUI (gui.py)
// 4. Exit

#define _CRT_SECURE_NO_WARNINGS
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// --- Matrix drops (same system as your GUI) ---
typedef struct { int y; int speed; char ch; } Drop;
static Drop *drops = NULL;
static int dropCount = 0;

static void MatrixInit(int width, int height) {
    dropCount = width;
    drops = malloc(sizeof(Drop) * dropCount);
    for (int i = 0; i < dropCount; i++) {
        drops[i].y = GetRandomValue(-height, height);
        drops[i].speed = GetRandomValue(5, 18);
        drops[i].ch = (char)(33 + GetRandomValue(0, 90));
    }
}

static void MatrixDrawUpdate(int width, int height) {
    for (int i = 0; i < dropCount; i++) {
        drops[i].y += drops[i].speed;
        if (drops[i].y > height + 20) {
            drops[i].y = GetRandomValue(-200, 0);
            drops[i].speed = GetRandomValue(5, 18);
            drops[i].ch = (char)(33 + GetRandomValue(0, 90));
        }
        DrawText(TextFormat("%c", drops[i].ch), i, drops[i].y, 14, (Color){0, 220, 0, 130});
    }
}

// --- Helper for nice buttons ---
static void DrawButton(Rectangle r, const char *text, bool hover) {
    Color bg = hover ? (Color){0, 120, 90, 220} : (Color){10, 10, 12, 220};
    DrawRectangleRounded(r, 0.15f, 10, bg);
    DrawRectangleRoundedLines(r, 0.15f, 10, (Color){0, 255, 170, 150});

    Vector2 size = MeasureTextEx(GetFontDefault(), text, 24, 1);
    DrawTextEx(GetFontDefault(), text,
        (Vector2){ r.x + (r.width - size.x)/2, r.y + (r.height - size.y)/2 },
        24, 1, WHITE
    );
}

int main(void) {
    InitWindow(800, 600, "LexiShift Launcher");
    SetTargetFPS(60);

    MatrixInit(800, 600);

    Rectangle btnConsole = { 250, 150, 300, 60 };
    Rectangle btnGUI =     { 250, 230, 300, 60 };
    Rectangle btnPy =      { 250, 310, 300, 60 };
    Rectangle btnExit =    { 250, 390, 300, 60 };

    while (!WindowShouldClose()) {
        Vector2 mp = GetMousePosition();
        bool altHeld = (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT));

        // ALT shortcuts
        if (altHeld && IsKeyPressed(KEY_ONE)) {
            system("lexishift.exe");
            CloseWindow();
            return 0;
        }
        if (altHeld && IsKeyPressed(KEY_TWO)) {
            system("lexishift_gui.exe");
            CloseWindow();
            return 0;
        }
        if (altHeld && IsKeyPressed(KEY_THREE)) {
            system("python gui.py");
            CloseWindow();
            return 0;
        }
        if (altHeld && IsKeyPressed(KEY_FOUR)) {
            CloseWindow();
            return 0;
        }

        // Mouse click
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mp, btnConsole)) {
                system("lexishift.exe");
                CloseWindow();
                return 0;
            }
            if (CheckCollisionPointRec(mp, btnGUI)) {
                system("lexishift_gui.exe");
                CloseWindow();
                return 0;
            }
            if (CheckCollisionPointRec(mp, btnPy)) {
                system("python gui.py");
                CloseWindow();
                return 0;
            }
            if (CheckCollisionPointRec(mp, btnExit)) {
                CloseWindow();
                return 0;
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        MatrixDrawUpdate(800, 600);
        DrawText("LexiShift Launcher", 220, 60, 36, (Color){0,255,150,255});

        DrawButton(btnConsole, "1. Console Version (ALT+1)", CheckCollisionPointRec(mp, btnConsole));
        DrawButton(btnGUI,     "2. Matrix GUI (ALT+2)",    CheckCollisionPointRec(mp, btnGUI));
        DrawButton(btnPy,      "3. Python GUI (ALT+3)",    CheckCollisionPointRec(mp, btnPy));
        DrawButton(btnExit,    "4. Exit (ALT+4)",          CheckCollisionPointRec(mp, btnExit));

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
