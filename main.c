#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <stdbool.h>
#include <stdio.h>

#define B_BUTTON_CODE 305  // From your Python KEY_MAPPING
#define A_BUTTON_CODE 304  // From your Python KEY_MAPPING
#define SQUARE_SIZE 100    // Size of the square to draw

int main() {
    // Initialize SDL
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    // Create window and renderer
    SDL_Window *window = SDL_CreateWindow("SDL2 Button Test",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        640, 480, SDL_WINDOW_FULLSCREEN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

    // Load font and create text
    TTF_Font *font = TTF_OpenFont("DejaVuSans.ttf", 24);
    SDL_Color color = {255, 255, 255, 255};
    SDL_Surface *text_surface = TTF_RenderText_Solid(font,
        "Press A to show square, B to exit", color);
    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    SDL_Rect text_rect = {50, 50, text_surface->w, text_surface->h};
    SDL_FreeSurface(text_surface);

    // Open input device
    int fd = open("/dev/input/event1", O_RDONLY);
    if (fd < 0) {
        perror("Failed to open input device");
        // Continue anyway, but without input
    }

    // Square properties
    SDL_Rect square_rect = {
        640/2 - SQUARE_SIZE/2,  // Center X
        480/2 - SQUARE_SIZE/2,  // Center Y
        SQUARE_SIZE,
        SQUARE_SIZE
    };
    bool a_button_pressed = false;

    // Main loop
    bool running = true;
    struct input_event ev;
    fd_set readfds;

    while (running) {
        // Clear screen with black background
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw the text
        SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);

        // Draw square only when A is pressed
        if (a_button_pressed) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Red square
            SDL_RenderFillRect(renderer, &square_rect);
        }

        // Update screen
        SDL_RenderPresent(renderer);

        // Check for SDL window events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
        }

        // Check for gamepad input
        if (fd >= 0) {
            struct timeval tv = {0, 16000}; // 16ms timeout (approx 60fps)
            FD_ZERO(&readfds);
            FD_SET(fd, &readfds);
            int ret = select(fd + 1, &readfds, NULL, NULL, &tv);

            if (ret > 0 && FD_ISSET(fd, &readfds)) {
                ssize_t bytes = read(fd, &ev, sizeof(ev));
                if (bytes == sizeof(ev) && ev.type == EV_KEY) {
                    // Check if it's the A button
                    if (ev.code == A_BUTTON_CODE) {
                        a_button_pressed = (ev.value == 1);  // 1 = pressed, 0 = released
                        printf("A button %s\n", a_button_pressed ? "pressed" : "released");
                    }

                    // Check if it's the B button and has been pressed
                    if (ev.code == B_BUTTON_CODE && ev.value == 1) {
                        printf("B button pressed - exiting\n");
                        running = false;
                    }
                }
            }
        } else {
            SDL_Delay(16); // 60fps if no input device
        }
    }

    // Cleanup
    if (fd >= 0) close(fd);
    SDL_DestroyTexture(text_texture);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
