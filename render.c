#include "render.h"
#include <stdio.h>

void render_init(RenderContext* ctx) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

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

void render_present(RenderContext* ctx) {
    SDL_RenderPresent(ctx->renderer);
}
