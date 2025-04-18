#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define FONT_PATH "DejaVuSans.ttf"

void cleanup(TTF_Font *font, SDL_Surface *screen) {
    if (font) TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
}

int main() {
    SDL_Surface *screen = NULL;
    SDL_Surface *textSurface = NULL;
    TTF_Font *font = NULL;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    // Set video mode
    screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_SWSURFACE);
    if (!screen) {
        fprintf(stderr, "Failed to set video mode: %s\n", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    // Initialize SDL_ttf
    if (TTF_Init() < 0) {
        fprintf(stderr, "SDL_ttf initialization failed: %s\n", TTF_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    // Load font
    font = TTF_OpenFont(FONT_PATH, 24);
    if (!font) {
        fprintf(stderr, "Failed to load font: %s\n", TTF_GetError());
        cleanup(NULL, screen);
        return EXIT_FAILURE;
    }

    // Render text
    SDL_Color textColor = {0, 0, 0}; // Black
    textSurface = TTF_RenderText_Solid(font, "Hello SDL1.2!", textColor);
    if (!textSurface) {
        fprintf(stderr, "Failed to render text: %s\n", TTF_GetError());
        cleanup(font, screen);
        return EXIT_FAILURE;
    }

    // Main loop
    SDL_Event event;
    int running = 1;
    Uint32 startTime = SDL_GetTicks();

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        // Clear screen
        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 255, 255, 255));

        // Blit text
        SDL_Rect textRect = {50, 50, 0, 0};
        SDL_BlitSurface(textSurface, NULL, screen, &textRect);

        // Update screen
        SDL_Flip(screen);

        if (SDL_GetTicks() - startTime > 2000) {
            running = 0;
        }
    }

    // Cleanup
    SDL_FreeSurface(textSurface);
    cleanup(font, screen);
    return EXIT_SUCCESS;
}
