/* render.h */
#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    int window_width;
    int window_height;
} RenderContext;

void render_init(RenderContext* ctx);
void render_cleanup(RenderContext* ctx);
void draw_clear(RenderContext* ctx);
void draw_text(RenderContext* ctx, const char* text, int x, int y);
void draw_square(RenderContext* ctx, int x, int y, int size);
SDL_Texture* load_image(RenderContext* ctx, const char* path);
void draw_image(RenderContext* ctx, SDL_Texture* texture, int x, int y);
void render_present(RenderContext* ctx);

#endif
