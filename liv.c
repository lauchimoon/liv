#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "raylib.h"

#define MIN_SIZE_X     800
#define MIN_SIZE_Y     640

#define HUD_BAR_SIZE_Y 24
#define ZOOM_CONSTANT  .25f

#define LIV_VIEW_FILE  0
#define LIV_VIEW_DIR   1

typedef struct LivConfig {
    float zoom;
    float rotation;
    bool hide_hud;
    bool antialiasing;
} LivConfig;

typedef struct LivState {
    char *path;
    int dst_width, dst_height;
    int window_width, window_height;
    int viewing;                // Directory or file?
    Image i;
    Texture t;
    Font font;
    Camera2D camera;
    LivConfig cfg;
} LivState;

bool check_path(const char *path, int viewing);

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
        printf("usage: %s <image/directory>\n", argv[0]);
        printf("    where <image> format is png, bmp, tga, jpg, qoi\n");
        return 1;
    }

    LivState state;
    const char *path = argv[1];
    struct stat s;
    int status = stat(path, &s);
    state.viewing = S_ISDIR(s.st_mode)? LIV_VIEW_DIR : LIV_VIEW_FILE;
    if (status < 0 || !check_path(path, state.viewing)) {
        printf("%s: bad path: this is not a valid input!\n", argv[0]);
        return 1;
    }

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
        if (state.viewing == LIV_VIEW_FILE) {
            DrawTexturePro(state.t, (Rectangle){ 0, 0, state.t.width, state.t.height },
                              (Rectangle){ 0, 0, state.dst_width, state.dst_height },
                              (Vector2){ state.dst_width/2, state.dst_height/2 },
                              state.cfg.rotation, WHITE);
        }
        EndMode2D();

        if (state.viewing == LIV_VIEW_FILE) {
            // Draw information HUD
            DrawRectangle(0, GetScreenHeight() - HUD_BAR_SIZE_Y, GetScreenWidth(), HUD_BAR_SIZE_Y, state.cfg.hide_hud? BLANK : BLACK);
            const char *info_text = TextFormat("%s | %dx%d | [%.0f%%]", path, state.i.width, state.i.height, state.cfg.zoom*100.0f);
            DrawTextEx(state.font, info_text, (Vector2){ 5.0f, GetScreenHeight() - HUD_BAR_SIZE_Y }, HUD_BAR_SIZE_Y - 8, 2.0f, state.cfg.hide_hud? BLANK : WHITE);
        } else if (state.viewing == LIV_VIEW_DIR) {
            DrawTextEx(state.font, TextFormat("Viewing directory: %s", state.path), (Vector2){ 5.0f, 5.0f }, 16, 2.0f, WHITE);
        }

        EndDrawing();
    }

    deinit(&state);
    return 0;
}

bool check_path(const char *path, int viewing)
{
    if (viewing == LIV_VIEW_FILE) {
        return IsFileExtension(path, ".png") || IsFileExtension(path, ".bmp") ||
            IsFileExtension(path, ".tga") || IsFileExtension(path, ".jpg") || IsFileExtension(path, ".qoi");
    } else if (viewing == LIV_VIEW_DIR) {
        return DirectoryExists(path);
    }
}

void init(LivState *state, const char *path)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    state->path = malloc(strlen(path) + 1);
    strcpy(state->path, path);

    if (state->viewing == LIV_VIEW_FILE) {
        state->i = LoadImage(path);
        state->dst_width = (state->i.width <= MIN_SIZE_X)? state->i.width : state->i.width/2;
        state->dst_height = (state->i.height <= MIN_SIZE_Y)? state->i.height : state->i.height/2;
    }
    state->window_width = (state->viewing == LIV_VIEW_FILE)? state->dst_width : MIN_SIZE_X;
    state->window_height = (state->viewing == LIV_VIEW_FILE)? state->dst_height : MIN_SIZE_Y;

    InitWindow(state->window_width, state->window_height, "liv");
    SetWindowMinSize(state->window_width, state->window_height);
    SetWindowSize((state->window_width >= GetMonitorWidth(0))? GetMonitorWidth(0) : state->window_width,
                  (state->window_height >= GetMonitorHeight(0))? GetMonitorHeight(0) : state->window_height);
    SetExitKey(KEY_Q);
    state->font = LoadFontEx("/usr/share/fonts/liberation/LiberationSans-Regular.ttf", HUD_BAR_SIZE_Y - 8, NULL, 95);
    if (state->viewing == LIV_VIEW_FILE) state->t = LoadTextureFromImage(state->i);

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
    if (state->viewing == LIV_VIEW_FILE) {
        UnloadImage(state->i);
        UnloadTexture(state->t);
        UnloadFont(state->font);
    }
    free(state->path);
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

