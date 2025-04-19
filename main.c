#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

int main() {
    // Initialize SDL with video and gamecontroller subsystems
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    // Initialize TTF
    if (TTF_Init() < 0) {
        fprintf(stderr, "TTF_Init error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    // Create window
    SDL_Window *window = SDL_CreateWindow("SDL2 Text",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        640, 480, SDL_WINDOW_FULLSCREEN);
    if (!window) {
        fprintf(stderr, "Window creation error: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Create renderer (software for Mali GPU compatibility)
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer) {
        fprintf(stderr, "Renderer creation error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Load font
    TTF_Font *font = TTF_OpenFont("DejaVuSans.ttf", 24);
    if (!font) {
        fprintf(stderr, "Font loading error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Open first available gamepad/controller
    SDL_GameController *controller = NULL;
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            controller = SDL_GameControllerOpen(i);
            if (controller) {
                printf("Found gamepad: %s\n", SDL_GameControllerName(controller));
                break;
            }
        }
    }

    // Create text surface and texture
    SDL_Color color = {255, 255, 255, 255};
    SDL_Surface *text_surface = TTF_RenderText_Solid(font,
        "Hello world! Press B button to exit", color);
    if (!text_surface) {
        fprintf(stderr, "Text surface error: %s\n", TTF_GetError());
        if (controller) SDL_GameControllerClose(controller);
        TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    SDL_Rect dstrect = {50, 50, text_surface->w, text_surface->h};
    SDL_FreeSurface(text_surface);

    // Main loop
    bool running = true;
    SDL_Event event;

    while (running) {
        // Event handling
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;

                case SDL_KEYDOWN:
                    // Also respond to ESC key for desktop testing
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                        running = false;
                    break;

                case SDL_CONTROLLERBUTTONDOWN:
                    // Check for B button press (matches button 305 in Python example)
                    if (event.cbutton.button == SDL_CONTROLLER_BUTTON_B) {
                        printf("B button pressed - exiting\n");
                        running = false;
                    }
                    printf("Controller button pressed: %d\n", event.cbutton.button);
                    break;
            }
        }

        // Render
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, text_texture, NULL, &dstrect);
        SDL_RenderPresent(renderer);

        // Small delay to prevent CPU hogging
        SDL_Delay(16); // ~60 fps
    }

    // Cleanup
    SDL_DestroyTexture(text_texture);
    if (controller) SDL_GameControllerClose(controller);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
