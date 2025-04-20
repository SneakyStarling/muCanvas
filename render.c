/* render.c */
#include "render.h"
#include <stdio.h>

void render_init(RenderContext* ctx) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    ctx->window = SDL_CreateWindow("SDL App",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        640, 480, SDL_WINDOW_FULLSCREEN);

    ctx->renderer = SDL_CreateRenderer(ctx->window, -1, SDL_RENDERER_SOFTWARE);
    ctx->font = TTF_OpenFont("./fonts/DejaVuSans.ttf", 24);
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
    // Check for NULL pointers
    if (!ctx || !ctx->renderer || !ctx->font || !text) {
        fprintf(stderr, "draw_text: NULL parameter detected\n");
        return;
    }

    SDL_Color color = {255, 255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Solid(ctx->font, text, color);

    // Check if surface creation succeeded
    if (!surface) {
        fprintf(stderr, "draw_text: Failed to render text '%s': %s\n",
                text, TTF_GetError());
        return;
    }

    // Store width and height before converting to texture
    int width = surface->w;
    int height = surface->h;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(ctx->renderer, surface);
    SDL_FreeSurface(surface);  // Free the surface as soon as we're done with it

    // Check if texture creation succeeded
    if (!texture) {
        fprintf(stderr, "draw_text: Failed to create texture: %s\n",
                SDL_GetError());
        return;
    }

    // Use the stored width and height
    SDL_Rect rect = {x, y, width, height};
    SDL_RenderCopy(ctx->renderer, texture, NULL, &rect);

    SDL_DestroyTexture(texture);
}

void draw_square(RenderContext* ctx, int x, int y, int size) {
    SDL_Rect rect = {x - size/2, y - size/2, size, size};
    SDL_SetRenderDrawColor(ctx->renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(ctx->renderer, &rect);
}

void render_present(RenderContext* ctx) {
    SDL_RenderPresent(ctx->renderer);
}
