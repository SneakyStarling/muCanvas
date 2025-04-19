#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <stdbool.h>
#include <stdio.h>

// Button code definitions
#define B_BUTTON_CODE 305
#define A_BUTTON_CODE 304
#define L1_BUTTON_CODE 308
#define L2_BUTTON_CODE 314
#define SQUARE_SIZE 100

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
    TTF_Font *font = TTF_OpenFont("./fonts/DejaVuSans.ttf", 24);
    SDL_Color color = {255, 255, 255, 255};
    SDL_Surface *text_surface = TTF_RenderText_Solid(font,
        "A: show center square | L1+L2: show side squares | B: exit", color);
    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    SDL_Rect text_rect = {50, 50, text_surface->w, text_surface->h};
    SDL_FreeSurface(text_surface);

    // Open input device
    int fd = open("/dev/input/event1", O_RDONLY);
    if (fd < 0) {
        perror("Failed to open input device");
        // Continue anyway, but without input
    }

    // Track button states for combinations
    bool button_states[512] = {false};  // Array to track state of each button

    // Square properties
    SDL_Rect center_square = {
        640/2 - SQUARE_SIZE/2,  // Center X
        480/2 - SQUARE_SIZE/2,  // Center Y
        SQUARE_SIZE,
        SQUARE_SIZE
    };

    // Left and right squares for L1+L2 combination
    SDL_Rect left_square = {
        640/2 - SQUARE_SIZE/2 - SQUARE_SIZE - 20,  // Left of center
        480/2 - SQUARE_SIZE/2,                     // Same Y as center
        SQUARE_SIZE,
        SQUARE_SIZE
    };

    SDL_Rect right_square = {
        640/2 - SQUARE_SIZE/2 + SQUARE_SIZE + 20,  // Right of center
        480/2 - SQUARE_SIZE/2,                     // Same Y as center
        SQUARE_SIZE,
        SQUARE_SIZE
    };

    bool a_button_pressed = false;
    bool l1_l2_combo_pressed = false;

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

        // Draw center square when A is pressed
        if (a_button_pressed && !l1_l2_combo_pressed) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Red square
            SDL_RenderFillRect(renderer, &center_square);
        }

        // Draw side squares when L1+L2 combo is pressed
        if (l1_l2_combo_pressed) {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);  // Green squares
            SDL_RenderFillRect(renderer, &left_square);
            SDL_RenderFillRect(renderer, &right_square);
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
            struct timeval tv = {0, 16000};  // 16ms timeout (approx 60fps)
            FD_ZERO(&readfds);
            FD_SET(fd, &readfds);
            int ret = select(fd + 1, &readfds, NULL, NULL, &tv);

            if (ret > 0 && FD_ISSET(fd, &readfds)) {
                ssize_t bytes = read(fd, &ev, sizeof(ev));
                if (bytes == sizeof(ev) && ev.type == EV_KEY) {
                    // Update button state for this button
                    button_states[ev.code] = (ev.value == 1);

                    // Process individual buttons
                    if (ev.code == A_BUTTON_CODE) {
                        a_button_pressed = button_states[A_BUTTON_CODE];
                        printf("A button %s\n", a_button_pressed ? "pressed" : "released");
                    }

                    // Process L1+L2 button combination
                    if (ev.code == L1_BUTTON_CODE || ev.code == L2_BUTTON_CODE) {
                        // Check if both L1 AND L2 are pressed
                        l1_l2_combo_pressed = button_states[L1_BUTTON_CODE] && button_states[L2_BUTTON_CODE];
                        printf("L1+L2 combo %s\n", l1_l2_combo_pressed ? "active" : "inactive");
                    }

                    // Check if it's the B button and has been pressed
                    if (ev.code == B_BUTTON_CODE && ev.value == 1) {
                        printf("B button pressed - exiting\n");
                        running = false;
                    }
                }
            }
        } else {
            SDL_Delay(16);  // 60fps if no input device
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
