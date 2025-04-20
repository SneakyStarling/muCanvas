#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include "input.h"

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
        "A: show center square | \nL1+L2: show side squares | B: exit", color);
    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    SDL_Rect text_rect = {50, 50, text_surface->w, text_surface->h};
    SDL_FreeSurface(text_surface);

    // Initialize input
    InputState input;
    if (!input_init(&input)) {
        printf("Input initialization failed, continuing without input\n");
    }

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

    // Main loop
    bool running = true;
    while (running) {
        // Process SDL events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
        }

        // Update input state
        input_update(&input);

        // Handle exit with B button
        if (button_pressed(&input, BUTTON_B)) {
            printf("B button pressed - exiting\n");
            running = false;
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw text
        SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);

        // Move red square with D-pad
        int move_speed = 5;
        if (button_down(&input, BUTTON_DPAD_LEFT)) {
            center_square.x -= move_speed;
        }
        if (button_down(&input, BUTTON_DPAD_RIGHT)) {
            center_square.x += move_speed;
        }
        if (button_down(&input, BUTTON_DPAD_UP)) {
            center_square.y -= move_speed;
        }
        if (button_down(&input, BUTTON_DPAD_DOWN)) {
            center_square.y += move_speed;
        }

        // Keep square within screen bounds
        if (center_square.x < 0) center_square.x = 0;
        if (center_square.y < 0) center_square.y = 0;
        if (center_square.x > 640 - SQUARE_SIZE) center_square.x = 640 - SQUARE_SIZE;
        if (center_square.y > 480 - SQUARE_SIZE) center_square.y = 480 - SQUARE_SIZE;

        // Get normalized joystick values (-1.0 to 1.0)
        float left_x = get_left_stick_x(&input);
        float left_y = get_left_stick_y(&input);
        float right_x = get_right_stick_x(&input);
        float right_y = get_right_stick_y(&input);

        // Use joystick values to move things
        if (fabs(left_x) > 0.0f) {
            // Move horizontally based on left stick X
            center_square.x += (int)(left_x * 5.0f);
        }

        if (fabs(left_y) > 0.0f) {
            // Move vertically based on left stick Y
            center_square.y -= (int)(left_y * 5.0f);  // Invert Y for natural movement
        }

        // Draw center square when A is pressed
        if (button_down(&input, BUTTON_A)) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Red square
            SDL_RenderFillRect(renderer, &center_square);
        }

        // Draw side squares when L1+L2 combo is pressed
        if (combo_down(&input, BUTTON_L1, BUTTON_L2)) {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);  // Green squares
            SDL_RenderFillRect(renderer, &left_square);
            SDL_RenderFillRect(renderer, &right_square);
        }

        // Update screen
        SDL_RenderPresent(renderer);

        // Cap frame rate
        SDL_Delay(16);  // ~60 FPS
    }

    // Cleanup
    input_cleanup(&input);
    SDL_DestroyTexture(text_texture);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
