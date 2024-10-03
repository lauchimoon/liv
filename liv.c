#include <stdio.h>
#include <math.h>
#include "raylib.h"

#define MIN_SIZE_X     800
#define MIN_SIZE_Y     640

#define HUD_BAR_SIZE_Y 24

typedef struct LivConfig {
    float zoom;
    float rotation;
    bool hide_hud;
} LivConfig;

int main(int argc, char **argv)
{
#ifndef LIV_DEBUG
    SetTraceLogLevel(LOG_NONE);
#endif
    if (argc < 2) {
        printf("usage: %s <image>\n", argv[0]);
        printf("    where <image> format is png, bmp, tga, jpg, qoi\n");
        return 1;
    }

    const char *path = argv[1];

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    Image i = LoadImage(path);
    int dst_width = (i.width <= MIN_SIZE_X)? i.width : i.width/2;
    int dst_height = (i.height <= MIN_SIZE_Y)? i.height : i.height/2;

    InitWindow(dst_width, dst_height, "liv");
    SetWindowMinSize(dst_width, dst_height);
    SetWindowSize((dst_width >= GetMonitorWidth(0))? GetMonitorWidth(0) : dst_width,
                  (dst_height >= GetMonitorHeight(0))? GetMonitorHeight(0) : dst_height);
    SetExitKey(KEY_Q);
    Texture t = LoadTextureFromImage(i);
    Font font = LoadFontEx("/usr/share/fonts/liberation/LiberationSans-Regular.ttf", HUD_BAR_SIZE_Y - 8, NULL, 95);

    LivConfig cfg;
    cfg.zoom = 1.0f;
    cfg.rotation = 0.0f;
    cfg.hide_hud = false;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Toggle HUD
        if (IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_H)) cfg.hide_hud = !cfg.hide_hud;

        // Rotate
        // E - 90 degrees anticlockwise
        // R - 90 degrees clockwise
        if (fabs(cfg.rotation) >= 360) cfg.rotation = 0;
        if (IsKeyPressed(KEY_E)) cfg.rotation -= 90;
        if (IsKeyPressed(KEY_R)) cfg.rotation += 90;

        BeginDrawing();
        ClearBackground(DARKGRAY);
        DrawTexturePro(t, (Rectangle){ 0, 0, t.width, t.height },
                          (Rectangle){ GetScreenWidth()/2, GetScreenHeight()/2, dst_width, dst_height },
                          (Vector2){ dst_width/2, dst_height/2 },
                          cfg.rotation, WHITE);

        // Draw information HUD
        DrawRectangle(0, GetScreenHeight() - HUD_BAR_SIZE_Y, GetScreenWidth(), HUD_BAR_SIZE_Y, cfg.hide_hud? BLANK : BLACK);
        const char *info_text = TextFormat("%s | %dx%d | [%.0f%%]", path, i.width, i.height, cfg.zoom*100.0f);
        DrawTextEx(font, info_text, (Vector2){ 5.0f, GetScreenHeight() - HUD_BAR_SIZE_Y }, HUD_BAR_SIZE_Y - 8, 2.0f, cfg.hide_hud? BLANK : WHITE);

        EndDrawing();
    }

    UnloadImage(i);
    UnloadTexture(t);
    UnloadFont(font);
    CloseWindow();
    return 0;
}
