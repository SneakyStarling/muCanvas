#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_W 640
#define SCREEN_H 480
#define FONT_PATH "/mnt/mmc/MUOS/application/muCanvas/DejaVuSans.ttf"

void cleanup(TTF_Font *font, SDL_Surface *screen) {
    if (font) TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
}

int main() {
    SDL_Surface *screen = NULL;
    TTF_Font *font = NULL;

    putenv("SDL_VIDEODRIVER=kmsdrm");  // or "fbdev"
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, 32, SDL_SWSURFACE);
    if (!screen) {
        fprintf(stderr, "SDL_SetVideoMode failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    if (TTF_Init() < 0) {
        fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    font = TTF_OpenFont(FONT_PATH, 24);
    if (!font) {
        fprintf(stderr, "TTF_OpenFont failed: %s\n", TTF_GetError());
        cleanup(NULL, screen);
        return 1;
    }

    SDL_Color color = {0, 0, 0};
    SDL_Surface *text = TTF_RenderText_Solid(font, "Hello SDL1.2!", color);
    if (!text) {
        fprintf(stderr, "TTF_RenderText failed\n");
        cleanup(font, screen);
        return 1;
    }

    SDL_Event event;
    Uint32 start = SDL_GetTicks();
    int running = 1;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
        }

        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 255, 255, 255));
        SDL_BlitSurface(text, NULL, screen, NULL);
        SDL_Flip(screen);

        if (SDL_GetTicks() - start > 2000) running = 0;
    }

    SDL_FreeSurface(text);
    cleanup(font, screen);
    return 0;
}
