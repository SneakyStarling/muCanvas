#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define FONT_PATH "DejaVuSans.ttf"  // Ensure this font exists in your build directory
#define DISPLAY_DURATION 2000       // 2 seconds

// Helper function to clean up resources
void cleanup(SDL_Window *w, SDL_Renderer *r, TTF_Font *f, SDL_Texture *t) {
    if (t) SDL_DestroyTexture(t);
    if (f) TTF_CloseFont(f);
    if (r) SDL_DestroyRenderer(r);
    if (w) SDL_DestroyWindow(w);
    TTF_Quit();
    SDL_Quit();
}

int main() {
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    TTF_Font *font = NULL;
    SDL_Texture *textTexture = NULL;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    // Initialize SDL_ttf
    if (TTF_Init() < 0) {
        fprintf(stderr, "SDL_ttf initialization failed: %s\n", TTF_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    // Create window
    window = SDL_CreateWindow(
        "SDL2 Text Example",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        cleanup(NULL, NULL, NULL, NULL);
        return EXIT_FAILURE;
    }

    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        fprintf(stderr, "Renderer creation failed: %s\n", SDL_GetError());
        cleanup(window, NULL, NULL, NULL);
        return EXIT_FAILURE;
    }

    // Load font
    font = TTF_OpenFont(FONT_PATH, 48);
    if (!font) {
        fprintf(stderr, "Failed to load font '%s': %s\n", FONT_PATH, TTF_GetError());
        fprintf(stderr, "Note: On Ubuntu/Debian, install with: sudo apt-get install ttf-dejavu\n");
        cleanup(window, renderer, NULL, NULL);
        return EXIT_FAILURE;
    }

    // Create text surface
    SDL_Color textColor = {0, 0, 0, 255}; // Black text
    SDL_Surface *textSurface = TTF_RenderText_Blended(font, "Hello, SDL2!", textColor);
    if (!textSurface) {
        fprintf(stderr, "Text surface creation failed: %s\n", TTF_GetError());
        cleanup(window, renderer, font, NULL);
        return EXIT_FAILURE;
    }

    // Create texture from surface
    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);  // Surface no longer needed
    if (!textTexture) {
        fprintf(stderr, "Texture creation failed: %s\n", SDL_GetError());
        cleanup(window, renderer, font, NULL);
        return EXIT_FAILURE;
    }

    // Set up text position
    SDL_Rect textRect = {
        .x = 50,
        .y = 50,
        .w = textSurface->w,
        .h = textSurface->h
    };

    // Main rendering loop
    Uint32 startTime = SDL_GetTicks();
    SDL_Event event;
    int running = 1;

    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        // Render text
        if (SDL_RenderCopy(renderer, textTexture, NULL, &textRect) < 0) {
            fprintf(stderr, "Rendering failed: %s\n", SDL_GetError());
            cleanup(window, renderer, font, textTexture);
            return EXIT_FAILURE;
        }

        // Update screen
        SDL_RenderPresent(renderer);

        // Check display duration
        if (SDL_GetTicks() - startTime > DISPLAY_DURATION) {
            running = 0;
        }
    }

    // Cleanup and exit
    cleanup(window, renderer, font, textTexture);
    return EXIT_SUCCESS;
}
