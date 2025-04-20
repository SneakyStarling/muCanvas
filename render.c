#include "render.h"
#include <stdio.h>

void render_init(RenderContext* ctx) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG); // Init SDL_image

    ctx->window_width = 640;
    ctx->window_height = 480;

    ctx->window = SDL_CreateWindow(
        "SDL2 Controller Example",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        ctx->window_width, ctx->window_height,
        SDL_WINDOW_FULLSCREEN
    );

    ctx->renderer = SDL_CreateRenderer(ctx->window, -1, SDL_RENDERER_SOFTWARE);
    ctx->font = TTF_OpenFont("./fonts/DejaVuSans.ttf", 24);

    if (!ctx->font) {
        fprintf(stderr, "Failed to load font: %s\n", TTF_GetError());
    }
}

void render_cleanup(RenderContext* ctx) {
    TTF_CloseFont(ctx->font);
    SDL_DestroyRenderer(ctx->renderer);
    SDL_DestroyWindow(ctx->window);
    TTF_Quit();
    SDL_Quit();
}

void draw_clear(RenderContext* ctx) {
    SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 255);
    SDL_RenderClear(ctx->renderer);
}

void draw_text(RenderContext* ctx, const char* text, int x, int y) {
    if (!ctx || !ctx->renderer || !ctx->font || !text) return;

    SDL_Color color = {255, 255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Solid(ctx->font, text, color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(ctx->renderer, surface);
    SDL_Rect rect = {x, y, surface->w, surface->h};

    SDL_RenderCopy(ctx->renderer, texture, NULL, &rect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void draw_square(RenderContext* ctx, int x, int y, int size) {
    SDL_Rect rect = {x, y, size, size};
    SDL_SetRenderDrawColor(ctx->renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(ctx->renderer, &rect);
}

SDL_Texture* load_image(RenderContext* ctx, const char* path) {
    SDL_Texture* texture = IMG_LoadTexture(ctx->renderer, path);
    if (!texture) {
        fprintf(stderr, "Failed to load image %s: %s\n", path, IMG_GetError());
    }
    return texture;
}

void draw_image(RenderContext* ctx, SDL_Texture* texture, int x, int y) {
    if (!texture) return;
    int w, h;
    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
    SDL_Rect dest = {x, y, w, h};
    SDL_RenderCopy(ctx->renderer, texture, NULL, &dest);
}

void render_present(RenderContext* ctx) {
    SDL_RenderPresent(ctx->renderer);
}
