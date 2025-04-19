#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <stdbool.h>
#include <stdio.h>

#define B_BUTTON_CODE 305

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window *window = SDL_CreateWindow("SDL2 Text",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        640, 480, SDL_WINDOW_FULLSCREEN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

    TTF_Font *font = TTF_OpenFont("DejaVuSans.ttf", 24);
    SDL_Color color = {255, 255, 255, 255};
    SDL_Surface *text_surface = TTF_RenderText_Solid(font, "Hello world! Press B button to exit", color);
    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);

    SDL_Rect dstrect = {50, 50, text_surface->w, text_surface->h};
    SDL_FreeSurface(text_surface);

    // Open input device ONCE, in blocking mode
    int fd = open("/dev/input/event1", O_RDONLY);
    if (fd < 0) {
        perror("Failed to open input device");
        // Still run the app, but only window close will work
    }

    bool running = true;
    struct input_event ev;
    fd_set readfds;
    int maxfd = fd;

    while (running) {
        // Render
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, text_texture, NULL, &dstrect);
        SDL_RenderPresent(renderer);

        // Poll SDL events (for window close)
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
        }

        // Wait for input event or timeout
        if (fd >= 0) {
            struct timeval tv = {0, 16000}; // 16 ms timeout (about 60Hz)
            FD_ZERO(&readfds);
            FD_SET(fd, &readfds);
            int ret = select(fd + 1, &readfds, NULL, NULL, &tv);
            if (ret > 0 && FD_ISSET(fd, &readfds)) {
                ssize_t bytes = read(fd, &ev, sizeof(ev));
                if (bytes == sizeof(ev) && ev.type == EV_KEY && ev.value == 1) {
                    if (ev.code == B_BUTTON_CODE) {
                        printf("B button pressed - exiting\n");
                        running = false;
                    }
                }
            }
        } else {
            SDL_Delay(16); // fallback if no input device
        }
    }

    if (fd >= 0) close(fd);
    SDL_DestroyTexture(text_texture);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
