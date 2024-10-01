#include "raylib.h"

int main()
{
    InitWindow(800, 600, "liv");

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(DARKGRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
