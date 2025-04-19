#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <stdbool.h>

#define B_BUTTON_CODE 305

// Structure for event handling
struct input_state {
    int current_code;
    char current_code_name[32];
    int current_value;
};

// Function to check for input events
bool check_input(const char* device_path, struct input_state* state) {
    int fd = open(device_path, O_RDONLY | O_NONBLOCK);
    if (fd == -1) {
        fprintf(stderr, "Could not open input device: %s\n", device_path);
        return false;
    }

    struct input_event event;
    ssize_t bytes = read(fd, &event, sizeof(event));

    if (bytes > 0 && event.type == EV_KEY) {
        if (event.value != 0) {  // Button pressed or held
            state->current_code = event.code;
            state->current_value = (event.value != 1) ? -1 : event.value;
            printf("Key pressed: %d, value: %d\n", state->current_code, state->current_value);
            close(fd);
            return true;
        }
    }

    close(fd);
    return false;
}

// Function to check if a specific button is pressed
bool key_pressed(struct input_state* state, int key_code) {
    return state->current_code == key_code;
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window *window = SDL_CreateWindow("SDL2 Text",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        640, 480, SDL_WINDOW_FULLSCREEN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);  // No flags, use default

    TTF_Font *font = TTF_OpenFont("DejaVuSans.ttf", 24);
    SDL_Color color = {255, 255, 255, 255};
    SDL_Surface *text_surface = TTF_RenderText_Solid(font,
        "Hello world! Press B button to exit", color);
    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);

    SDL_Rect dstrect = {50, 50, text_surface->w, text_surface->h};
    SDL_FreeSurface(text_surface);

    // Main loop
    struct input_state state = {0};
    bool running = true;

    while (running) {
        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Render text
        SDL_RenderCopy(renderer, text_texture, NULL, &dstrect);
        SDL_RenderPresent(renderer);

        // Process SDL events for window management
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // Check direct input
        if (check_input("/dev/input/event1", &state)) {
            if (key_pressed(&state, B_BUTTON_CODE)) {
                printf("B button pressed - exiting\n");
                running = false;
            }
        }

        // Small delay to prevent CPU hogging
        SDL_Delay(16);
    }

    SDL_DestroyTexture(text_texture);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
