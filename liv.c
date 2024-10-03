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

typedef struct LivState {
    const char *path;
    int dst_width, dst_height;
    Image i;
    Texture t;
    Font font;
    Camera2D camera;
    LivConfig cfg;
} LivState;

void init(LivState *state, const char *path);
void deinit(LivState *state);

void handle_rotation(LivState *state);
void handle_zoom(LivState *state);
void handle_panning(LivState *state);
void set_antialiasing(LivState *state);

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

    LivState state;
    const char *path = argv[1];
    init(&state, path);

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        state.camera.offset = (Vector2){ GetScreenWidth()/2, GetScreenHeight()/2 };

        // Toggle HUD
        if (IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_H)) state.cfg.hide_hud = !state.cfg.hide_hud;

        // Rotate
        // E - 90 degrees anticlockwise
        // R - 90 degrees clockwise
        handle_rotation(&state);

        // Tune zoom
        handle_zoom(&state);

        // Move camera around
        // Only allowed if the image is too zoomed in.
        handle_panning(&state);

        // Antialiasing
        set_antialiasing(&state);

        BeginDrawing();
        ClearBackground(DARKGRAY);
        BeginMode2D(state.camera);
        DrawTexturePro(state.t, (Rectangle){ 0, 0, state.t.width, state.t.height },
                          (Rectangle){ 0, 0, state.dst_width, state.dst_height },
                          (Vector2){ state.dst_width/2, state.dst_height/2 },
                          state.cfg.rotation, WHITE);
        EndMode2D();

        // Draw information HUD
        DrawRectangle(0, GetScreenHeight() - HUD_BAR_SIZE_Y, GetScreenWidth(), HUD_BAR_SIZE_Y, state.cfg.hide_hud? BLANK : BLACK);
        const char *info_text = TextFormat("%s | %dx%d | [%.0f%%]", path, state.i.width, state.i.height, state.cfg.zoom*100.0f);
        DrawTextEx(state.font, info_text, (Vector2){ 5.0f, GetScreenHeight() - HUD_BAR_SIZE_Y }, HUD_BAR_SIZE_Y - 8, 2.0f, state.cfg.hide_hud? BLANK : WHITE);

        EndDrawing();
    }

    deinit(&state);
    return 0;
}

void init(LivState *state, const char *path)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    state->i = LoadImage(path);
    state->dst_width = (state->i.width <= MIN_SIZE_X)? state->i.width : state->i.width/2;
    state->dst_height = (state->i.height <= MIN_SIZE_Y)? state->i.height : state->i.height/2;

    InitWindow(state->dst_width, state->dst_height, "liv");
    SetWindowMinSize(state->dst_width, state->dst_height);
    SetWindowSize((state->dst_width >= GetMonitorWidth(0))? GetMonitorWidth(0) : state->dst_width,
                  (state->dst_height >= GetMonitorHeight(0))? GetMonitorHeight(0) : state->dst_height);
    SetExitKey(KEY_Q);
    state->t = LoadTextureFromImage(state->i);
    state->font = LoadFontEx("/usr/share/fonts/liberation/LiberationSans-Regular.ttf", HUD_BAR_SIZE_Y - 8, NULL, 95);

    LivConfig cfg = { 0 };
    cfg.zoom = 1.0f;
    cfg.rotation = 0.0;
    cfg.hide_hud = false;
    cfg.antialiasing = true;
    state->cfg = cfg;

    Camera2D camera = { 0 };
    camera.offset = (Vector2){ GetScreenWidth()/2, GetScreenHeight()/2 };
    camera.target = (Vector2){ 0.0f, 0.0f };
    camera.rotation = 0.0f;
    camera.zoom = cfg.zoom;
    state->camera = camera;
}

void deinit(LivState *state)
{
    UnloadImage(state->i);
    UnloadTexture(state->t);
    UnloadFont(state->font);
    CloseWindow();
}

void handle_rotation(LivState *state)
{
    if (fabs(state->cfg.rotation) >= 360) state->cfg.rotation = 0;
    if (IsKeyPressed(KEY_E)) state->cfg.rotation -= 90;
    if (IsKeyPressed(KEY_R)) state->cfg.rotation += 90;
}

void handle_zoom(LivState *state)
{
    state->camera.zoom = state->cfg.zoom;
    if (IsKeyPressed(KEY_Z) && state->cfg.zoom > 0.25f) state->cfg.zoom -= ZOOM_CONSTANT;
    if (IsKeyPressed(KEY_X) && state->cfg.zoom < 3.0f) state->cfg.zoom += ZOOM_CONSTANT;
}

void handle_panning(LivState *state)
{
    if (state->cfg.zoom > 1.0f) {
        if (IsKeyDown(KEY_H)) state->camera.target.x -= ZOOM_CONSTANT*state->cfg.zoom*20;
        if (IsKeyDown(KEY_J)) state->camera.target.y += ZOOM_CONSTANT*state->cfg.zoom*20;
        if (IsKeyDown(KEY_K)) state->camera.target.y -= ZOOM_CONSTANT*state->cfg.zoom*20;
        if (IsKeyDown(KEY_L)) state->camera.target.x += ZOOM_CONSTANT*state->cfg.zoom*20;
    } else state->camera.target = (Vector2){ 0.0f, 0.0f };
}

void set_antialiasing(LivState *state)
{
    if (state->cfg.antialiasing) SetTextureFilter(state->t, TEXTURE_FILTER_BILINEAR);
    else SetTextureFilter(state->t, TEXTURE_FILTER_POINT);
    if (IsKeyPressed(KEY_A)) state->cfg.antialiasing = !state->cfg.antialiasing;
}

