#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void matrix_fullscreen_intro() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD written;

    // Hide cursor
    CONSOLE_CURSOR_INFO cci;
    GetConsoleCursorInfo(h, &cci);
    cci.bVisible = FALSE;
    SetConsoleCursorInfo(h, &cci);

    system("cls");

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(h, &csbi);

    int width  = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    int *drops = (int*)malloc(width * sizeof(int));

    srand((unsigned int)time(NULL));
    for (int i = 0; i < width; i++)
        drops[i] = rand() % height;

    for (int frame = 0; frame < 120; frame++) {
        for (int i = 0; i < width; i++) {
            if (rand() % 20 == 0)
                drops[i] = 0;

            COORD pos = { (SHORT)i, (SHORT)drops[i] };
            SetConsoleCursorPosition(h, pos);
            SetConsoleTextAttribute(h, 10);
            char c = '0' + (rand() % 10);
            WriteConsoleA(h, &c, 1, &written, NULL);

            drops[i]++;

            if (drops[i] > 1) {
                COORD p2 = { (SHORT)i, (SHORT)(drops[i]-1) };
                SetConsoleCursorPosition(h, p2);
                SetConsoleTextAttribute(h, 0);
                char blank = ' ';
                WriteConsoleA(h, &blank, 1, &written, NULL);
            }

            if (drops[i] >= height) drops[i] = 0;
        }
        Sleep(30);
    }

    free(drops);

    system("cls");
    SetConsoleTextAttribute(h, 10);
    printf("\n\n\n\n");
    printf("               W E L C O M E   T O   L E X I S H I F T\n\n");
    Sleep(2000);

    cci.bVisible = TRUE;
    SetConsoleCursorInfo(h, &cci);

    SetConsoleTextAttribute(h, 7);
    system("cls");
}
