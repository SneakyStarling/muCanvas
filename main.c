#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window *window = SDL_CreateWindow("SDL2 Text", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_FULLSCREEN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

    TTF_Font *font = TTF_OpenFont("DejaVuSans.ttf", 24);
    SDL_Color color = {255, 255, 255, 255};
    SDL_Surface *text_surface = TTF_RenderText_Solid(font, "Hello world!", color);
    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);

    SDL_Rect dstrect = {50, 50, text_surface->w, text_surface->h};
    SDL_FreeSurface(text_surface);

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, text_texture, NULL, &dstrect);
    int i;
    for (i = 1; i <= 50; i++) {
        SDL_RenderPresent(renderer);
    }

    SDL_Delay(20000);

    SDL_DestroyTexture(text_texture);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
