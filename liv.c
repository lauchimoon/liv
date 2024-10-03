#include <stdio.h>
#include <math.h>
#include "raylib.h"

#define MIN_SIZE_X     800
#define MIN_SIZE_Y     640

#define HUD_BAR_SIZE_Y 24
#define ZOOM_CONSTANT  .25f

typedef struct LivConfig {
    float zoom;
    float rotation;
    bool hide_hud;
    bool antialiasing;
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

    LivConfig cfg = { 0 };
    cfg.zoom = 1.0f;
    cfg.rotation = 0.0;
    cfg.hide_hud = false;
    cfg.antialiasing = true;

    Camera2D camera = { 0 };
    camera.offset = (Vector2){ GetScreenWidth()/2, GetScreenHeight()/2 };
    camera.target = (Vector2){ 0.0f, 0.0f };
    camera.rotation = 0.0f;
    camera.zoom = cfg.zoom;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        camera.offset = (Vector2){ GetScreenWidth()/2, GetScreenHeight()/2 };

        // Toggle HUD
        if (IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_H)) cfg.hide_hud = !cfg.hide_hud;

        // Rotate
        // E - 90 degrees anticlockwise
        // R - 90 degrees clockwise
        if (fabs(cfg.rotation) >= 360) cfg.rotation = 0;
        if (IsKeyPressed(KEY_E)) cfg.rotation -= 90;
        if (IsKeyPressed(KEY_R)) cfg.rotation += 90;

        // Tune zoom
        camera.zoom = cfg.zoom;
        if (IsKeyPressed(KEY_Z) && cfg.zoom > 0.25f) cfg.zoom -= ZOOM_CONSTANT;
        if (IsKeyPressed(KEY_X) && cfg.zoom < 3.0f) cfg.zoom += ZOOM_CONSTANT;

        // Move camera around
        // Only allowed if the image is too zoomed in.
        if (cfg.zoom > 1.0f) {
            if (IsKeyDown(KEY_H)) camera.target.x -= ZOOM_CONSTANT*cfg.zoom*20;
            if (IsKeyDown(KEY_J)) camera.target.y += ZOOM_CONSTANT*cfg.zoom*20;
            if (IsKeyDown(KEY_K)) camera.target.y -= ZOOM_CONSTANT*cfg.zoom*20;
            if (IsKeyDown(KEY_L)) camera.target.x += ZOOM_CONSTANT*cfg.zoom*20;
        } else camera.target = (Vector2){ 0.0f, 0.0f };

        // Antialiasing
        if (cfg.antialiasing) SetTextureFilter(t, TEXTURE_FILTER_BILINEAR);
        else SetTextureFilter(t, TEXTURE_FILTER_POINT);
        if (IsKeyPressed(KEY_A)) cfg.antialiasing = !cfg.antialiasing;

        BeginDrawing();
        ClearBackground(DARKGRAY);
        BeginMode2D(camera);
        DrawTexturePro(t, (Rectangle){ 0, 0, t.width, t.height },
                          (Rectangle){ 0, 0, dst_width, dst_height },
                          (Vector2){ dst_width/2, dst_height/2 },
                          cfg.rotation, WHITE);
        EndMode2D();

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
