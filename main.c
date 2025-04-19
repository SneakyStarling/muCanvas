#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <cJSON.h>

// ===== Constants and Configuration =====
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define INPUT_DEVICE "/dev/input/event1"
#define FONT_PATH "fonts/DejaVuSans.ttf"
#define MAX_SHAPES 20
#define MAX_TEXTS 20
#define MAX_BUTTONS 32
#define MAX_COLOR_SCHEMES 5

// Button code definitions
typedef enum {
    BUTTON_A = 304,
    BUTTON_B = 305,
    BUTTON_Y = 306,
    BUTTON_X = 307,
    BUTTON_L1 = 308,
    BUTTON_R1 = 309,
    BUTTON_L2 = 314,
    BUTTON_R2 = 315,
    BUTTON_DY = 17,
    BUTTON_DX = 16,
    BUTTON_SELECT = 310,
    BUTTON_START = 311,
    BUTTON_MENU = 312,
    BUTTON_VOLUP = 114,
    BUTTON_VOLDOWN = 115
} ButtonCode;

// ===== Type Definitions =====

// Color scheme structure
typedef struct {
    char name[32];          // Theme name
    SDL_Color background;   // Background color
    SDL_Color text;         // Main text color
    SDL_Color highlight;    // Selected items
    SDL_Color accent;       // Accent color
    SDL_Color shadow;       // Secondary elements
} ColorScheme;

// UI shape object
typedef struct {
    SDL_Rect rect;         // Position and size
    SDL_Color color;       // Color
    bool visible;          // Whether it's currently visible
    bool filled;           // Whether to fill or outline
    int borderWidth;       // Border width for outlines
    int id;                // Identifier for tracking
} Shape;

// UI text object
typedef struct {
    char text[256];        // Text content
    SDL_Point position;    // Position
    SDL_Color color;       // Text color
    int fontSize;          // Font size
    bool visible;          // Whether it's currently visible
    SDL_Texture *texture;  // Cached texture
    SDL_Rect rect;         // Destination rectangle
    int id;                // Identifier for tracking
} Text;

// Input system and button state tracking
typedef struct {
    int fd;                        // File descriptor for input device
    bool buttonState[MAX_BUTTONS]; // Current state of all buttons
    bool prevButtonState[MAX_BUTTONS]; // Previous state for edge detection
    bool buttonChanged[MAX_BUTTONS];   // Whether button changed this frame
} InputSystem;

// Rendering system
typedef struct {
    SDL_Renderer *renderer;
    TTF_Font *fonts[3];       // Small, Medium, Large fonts
    Shape shapes[MAX_SHAPES]; // Shape buffer
    Text texts[MAX_TEXTS];    // Text buffer
    int shapeCount;
    int textCount;
    ColorScheme colorSchemes[MAX_COLOR_SCHEMES];
    int activeColorScheme;
} RenderSystem;

// Main application state
typedef struct {
    SDL_Window *window;
    bool running;
    InputSystem input;
    RenderSystem render;
    int selectedItem;
    int menuItemCount;
} App;

// ===== Function Prototypes =====

// Initialization functions
bool initialize_sdl(App *app);
bool initialize_input(InputSystem *input);
bool initialize_render(RenderSystem *render, SDL_Window *window);
void setup_color_schemes(RenderSystem *render);

// Cleanup functions
void cleanup_sdl(App *app);
void cleanup_input(InputSystem *input);
void cleanup_render(RenderSystem *render);

// Input handling
void process_input(App *app);
bool is_button_pressed(InputSystem *input, ButtonCode button);
bool is_button_released(InputSystem *input, ButtonCode button);
bool is_button_down(InputSystem *input, ButtonCode button);
bool are_buttons_down(InputSystem *input, ButtonCode *buttons, int count);

// Rendering functions
int add_shape(RenderSystem *render, SDL_Rect rect, SDL_Color color, bool filled, int borderWidth);
int add_text(RenderSystem *render, const char *text, SDL_Point position, SDL_Color color, int fontSize);
void update_shape(RenderSystem *render, int id, SDL_Rect rect, SDL_Color color, bool visible);
void update_text(RenderSystem *render, int id, const char *text, SDL_Point position, SDL_Color color, bool visible);
void render_frame(App *app);
void render_menu(App *app);

// ===== SDL Initialization and Cleanup =====
bool initialize_sdl(App *app) {
    // Initialize SDL with video subsystem
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return false;
    }

    // Initialize TTF
    if (TTF_Init() < 0) {
        fprintf(stderr, "TTF_Init error: %s\n", TTF_GetError());
        SDL_Quit();
        return false;
    }

    // Create window
    app->window = SDL_CreateWindow(
        "SDL2 Controller Example",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_FULLSCREEN
    );
    if (!app->window) {
        fprintf(stderr, "Window creation error: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    return true;
}

void cleanup_sdl(App *app) {
    if (app->window) SDL_DestroyWindow(app->window);
    TTF_Quit();
    SDL_Quit();
}

// ===== Input System =====
bool initialize_input(InputSystem *input) {
    // Open input device
    input->fd = open(INPUT_DEVICE, O_RDONLY);
    if (input->fd < 0) {
        perror("Failed to open input device");
        return false;
    }

    // Initialize button states
    memset(input->buttonState, 0, sizeof(input->buttonState));
    memset(input->prevButtonState, 0, sizeof(input->prevButtonState));
    memset(input->buttonChanged, 0, sizeof(input->buttonChanged));

    return true;
}

void cleanup_input(InputSystem *input) {
    if (input->fd >= 0) {
        close(input->fd);
        input->fd = -1;
    }
}

void process_input(App *app) {
    InputSystem *input = &app->input;

    // Save previous button states for edge detection
    memcpy(input->prevButtonState, input->buttonState, sizeof(input->buttonState));

    // Reset change flags
    memset(input->buttonChanged, 0, sizeof(input->buttonChanged));

    // Read all pending input events
    if (input->fd >= 0) {
        struct input_event ev;
        fd_set readfds;
        struct timeval tv = {0, 0}; // Non-blocking

        FD_ZERO(&readfds);
        FD_SET(input->fd, &readfds);

        while (select(input->fd + 1, &readfds, NULL, NULL, &tv) > 0) {
            ssize_t bytes = read(input->fd, &ev, sizeof(ev));

            if (bytes == sizeof(ev) && ev.type == EV_KEY && ev.code < MAX_BUTTONS) {
                // Update button state
                bool newState = (ev.value != 0);
                if (input->buttonState[ev.code] != newState) {
                    input->buttonState[ev.code] = newState;
                    input->buttonChanged[ev.code] = true;

                    if (newState) {
                        printf("Button %d pressed\n", ev.code);
                    } else {
                        printf("Button %d released\n", ev.code);
                    }
                }
            }

            // Check if more events are pending
            FD_ZERO(&readfds);
            FD_SET(input->fd, &readfds);
            tv.tv_sec = 0;
            tv.tv_usec = 0;
        }
    }
}

bool is_button_pressed(InputSystem *input, ButtonCode button) {
    return input->buttonState[button] && input->buttonChanged[button];
}

bool is_button_released(InputSystem *input, ButtonCode button) {
    return !input->buttonState[button] && input->buttonChanged[button];
}

bool is_button_down(InputSystem *input, ButtonCode button) {
    return input->buttonState[button];
}

bool are_buttons_down(InputSystem *input, ButtonCode *buttons, int count) {
    for (int i = 0; i < count; i++) {
        if (!input->buttonState[buttons[i]]) {
            return false;
        }
    }
    return true;
}

// ===== Render System =====
bool initialize_render(RenderSystem *render, SDL_Window *window) {
    // Create renderer (software for Mali GPU compatibility)
    render->renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!render->renderer) {
        fprintf(stderr, "Renderer creation error: %s\n", SDL_GetError());
        return false;
    }

    // Load fonts at different sizes
    render->fonts[0] = TTF_OpenFont(FONT_PATH, 16); // Small
    render->fonts[1] = TTF_OpenFont(FONT_PATH, 24); // Medium
    render->fonts[2] = TTF_OpenFont(FONT_PATH, 32); // Large

    if (!render->fonts[0] || !render->fonts[1] || !render->fonts[2]) {
        fprintf(stderr, "Font loading error: %s\n", TTF_GetError());
        return false;
    }

    // Initialize shape and text buffers
    render->shapeCount = 0;
    render->textCount = 0;

    // Setup color schemes
    setup_color_schemes(render);
    render->activeColorScheme = 0;

    return true;
}

void setup_color_schemes(RenderSystem *render) {
    // Default dark scheme
    render->colorSchemes[0].background = (SDL_Color){0, 0, 0, 255};
    render->colorSchemes[0].text = (SDL_Color){255, 255, 255, 255};
    render->colorSchemes[0].highlight = (SDL_Color){255, 0, 0, 255};
    render->colorSchemes[0].accent = (SDL_Color){0, 255, 255, 255};
    render->colorSchemes[0].shadow = (SDL_Color){128, 128, 128, 255};

    // Light scheme
    render->colorSchemes[1].background = (SDL_Color){240, 240, 240, 255};
    render->colorSchemes[1].text = (SDL_Color){0, 0, 0, 255};
    render->colorSchemes[1].highlight = (SDL_Color){255, 0, 0, 255};
    render->colorSchemes[1].accent = (SDL_Color){0, 128, 255, 255};
    render->colorSchemes[1].shadow = (SDL_Color){128, 128, 128, 255};

    // High contrast
    render->colorSchemes[2].background = (SDL_Color){0, 0, 0, 255};
    render->colorSchemes[2].text = (SDL_Color){255, 255, 0, 255};
    render->colorSchemes[2].highlight = (SDL_Color){255, 255, 255, 255};
    render->colorSchemes[2].accent = (SDL_Color){0, 255, 0, 255};
    render->colorSchemes[2].shadow = (SDL_Color){128, 0, 128, 255};
}

void cleanup_render(RenderSystem *render) {
    // Free all text textures
    for (int i = 0; i < render->textCount; i++) {
        if (render->texts[i].texture) {
            SDL_DestroyTexture(render->texts[i].texture);
        }
    }

    // Free fonts
    for (int i = 0; i < 3; i++) {
        if (render->fonts[i]) {
            TTF_CloseFont(render->fonts[i]);
        }
    }

    // Destroy renderer
    if (render->renderer) {
        SDL_DestroyRenderer(render->renderer);
    }
}

int add_shape(RenderSystem *render, SDL_Rect rect, SDL_Color color, bool filled, int borderWidth) {
    if (render->shapeCount >= MAX_SHAPES) {
        return -1; // Buffer full
    }

    int id = render->shapeCount++;
    render->shapes[id].rect = rect;
    render->shapes[id].color = color;
    render->shapes[id].visible = true;
    render->shapes[id].filled = filled;
    render->shapes[id].borderWidth = borderWidth;
    render->shapes[id].id = id;

    return id;
}

int add_text(RenderSystem *render, const char *text, SDL_Point position, SDL_Color color, int fontSize) {
    if (render->textCount >= MAX_TEXTS) {
        return -1; // Buffer full
    }

    int id = render->textCount++;
    strncpy(render->texts[id].text, text, sizeof(render->texts[id].text)-1);
    render->texts[id].position = position;
    render->texts[id].color = color;
    render->texts[id].fontSize = fontSize;
    render->texts[id].visible = true;
    render->texts[id].id = id;

    // Create texture
    TTF_Font *font = render->fonts[fontSize];
    SDL_Surface *surface = TTF_RenderText_Blended(font, text, color);
    render->texts[id].texture = SDL_CreateTextureFromSurface(render->renderer, surface);
    render->texts[id].rect.x = position.x;
    render->texts[id].rect.y = position.y;
    render->texts[id].rect.w = surface->w;
    render->texts[id].rect.h = surface->h;
    SDL_FreeSurface(surface);

    return id;
}

void update_shape(RenderSystem *render, int id, SDL_Rect rect, SDL_Color color, bool visible) {
    if (id < 0 || id >= render->shapeCount) {
        return;
    }

    render->shapes[id].rect = rect;
    render->shapes[id].color = color;
    render->shapes[id].visible = visible;
}

void update_text(RenderSystem *render, int id, const char *text, SDL_Point position, SDL_Color color, bool visible) {
    if (id < 0 || id >= render->textCount) {
        return;
    }

    // Only recreate texture if text or color changed
    if (strcmp(render->texts[id].text, text) != 0 ||
        memcmp(&render->texts[id].color, &color, sizeof(SDL_Color)) != 0) {

        // Update text
        strncpy(render->texts[id].text, text, sizeof(render->texts[id].text)-1);
        render->texts[id].color = color;

        // Recreate texture
        if (render->texts[id].texture) {
            SDL_DestroyTexture(render->texts[id].texture);
        }

        TTF_Font *font = render->fonts[render->texts[id].fontSize];
        SDL_Surface *surface = TTF_RenderText_Blended(font, text, color);
        render->texts[id].texture = SDL_CreateTextureFromSurface(render->renderer, surface);
        render->texts[id].rect.w = surface->w;
        render->texts[id].rect.h = surface->h;
        SDL_FreeSurface(surface);
    }

    // Update position and visibility
    render->texts[id].position = position;
    render->texts[id].rect.x = position.x;
    render->texts[id].rect.y = position.y;
    render->texts[id].visible = visible;
}

void render_frame(App *app) {
    RenderSystem *render = &app->render;

    // Clear screen with background color
    SDL_Color bg = render->colorSchemes[render->activeColorScheme].background;
    SDL_SetRenderDrawColor(render->renderer, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderClear(render->renderer);

    // Draw all visible shapes
    for (int i = 0; i < render->shapeCount; i++) {
        Shape *shape = &render->shapes[i];
        if (!shape->visible) continue;

        SDL_SetRenderDrawColor(render->renderer,
                              shape->color.r,
                              shape->color.g,
                              shape->color.b,
                              shape->color.a);

        if (shape->filled) {
            SDL_RenderFillRect(render->renderer, &shape->rect);
        } else {
            if (shape->borderWidth <= 1) {
                SDL_RenderDrawRect(render->renderer, &shape->rect);
            } else {
                // Draw thick border
                SDL_Rect outer = shape->rect;
                SDL_Rect inner = {
                    outer.x + shape->borderWidth,
                    outer.y + shape->borderWidth,
                    outer.w - 2 * shape->borderWidth,
                    outer.h - 2 * shape->borderWidth
                };

                SDL_RenderFillRect(render->renderer, &outer);
                SDL_SetRenderDrawColor(render->renderer, bg.r, bg.g, bg.b, bg.a);
                SDL_RenderFillRect(render->renderer, &inner);
            }
        }
    }

    // Draw all visible texts
    for (int i = 0; i < render->textCount; i++) {
        Text *text = &render->texts[i];
        if (!text->visible || !text->texture) continue;

        SDL_RenderCopy(render->renderer, text->texture, NULL, &text->rect);
    }

    // Render demo menu
    render_menu(app);

    // Update screen
    SDL_RenderPresent(render->renderer);
}

// Process button events through a switch-case paradigm
void process_button_event(App *app, ButtonCode button, bool pressed) {
    if (pressed) {
        switch (button) {
            case BUTTON_A:
                printf("A button pressed\n");
                // A button functionality handled in selection_menu
                break;

            case BUTTON_B:
                printf("B button pressed - exit\n");
                // Handled in respective menus or main loop
                break;

            case BUTTON_DY:
            case BUTTON_DX:
                // Navigation handled in selection_menu
                printf("Direction button %d pressed\n", button);
                break;

            case BUTTON_START:
                printf("Start button pressed - theme menu\n");
                break;

            case BUTTON_Y:
                // Check for button combinations
                if (app->input.buttonState[BUTTON_X]) {
                    printf("X+Y combination detected!\n");
                    // Special action for X+Y combination
                }
                break;

            case BUTTON_X:
                // Check for button combinations
                if (app->input.buttonState[BUTTON_Y]) {
                    printf("Y+X combination detected!\n");
                    // Special action for Y+X combination
                }
                break;

            default:
                printf("Button %d pressed\n", button);
                break;
        }
    } else {
        printf("Button %d released\n", button);
    }
}

// Load themes from directory
bool load_theme_files(App *app) {
    DIR *dir = opendir("themes");
    if (!dir) {
        mkdir("themes", 0755);  // Create if doesn't exist
        app->render.colorSchemeCount = 2;  // Use defaults
        return false;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && strstr(entry->d_name, ".json")) {
            char path[256];
            snprintf(path, sizeof(path), "themes/%s", entry->d_name);
            load_theme_from_file(&app->render, path);
        }
    }

    closedir(dir);
    return true;
}

// Load individual theme file
bool load_theme_from_file(RenderSystem *render, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return false;

    // Read file content
    char buffer[4096] = {0};
    fread(buffer, 1, sizeof(buffer)-1, fp);
    fclose(fp);

    // Parse JSON
    cJSON *json = cJSON_Parse(buffer);
    if (!json) return false;

    // Extract theme data
    ColorScheme theme = {0};

    // Get theme name
    cJSON *name = cJSON_GetObjectItem(json, "name");
    if (cJSON_IsString(name)) {
        strncpy(theme.name, name->valuestring, sizeof(theme.name)-1);
    }

    // Parse colors (hex format: "#RRGGBB" or array [R,G,B,A])
    SDL_Color parse_color(cJSON *color_obj) {
        SDL_Color color = {255, 255, 255, 255};

        if (cJSON_IsString(color_obj)) {
            // Parse hex color (#RRGGBB)
            const char *hex = color_obj->valuestring;
            if (hex[0] == '#') {
                sscanf(hex+1, "%2hhx%2hhx%2hhx", &color.r, &color.g, &color.b);
            }
        }
        else if (cJSON_IsArray(color_obj)) {
            // Parse [R,G,B,A] array
            color.r = cJSON_GetArrayItem(color_obj, 0)->valueint;
            color.g = cJSON_GetArrayItem(color_obj, 1)->valueint;
            color.b = cJSON_GetArrayItem(color_obj, 2)->valueint;
            if (cJSON_GetArraySize(color_obj) > 3) {
                color.a = cJSON_GetArrayItem(color_obj, 3)->valueint;
            }
        }

        return color;
    }

    // Get all theme colors
    theme.background = parse_color(cJSON_GetObjectItem(json, "background"));
    theme.text = parse_color(cJSON_GetObjectItem(json, "text"));
    theme.highlight = parse_color(cJSON_GetObjectItem(json, "highlight"));
    theme.accent = parse_color(cJSON_GetObjectItem(json, "accent"));
    theme.shadow = parse_color(cJSON_GetObjectItem(json, "shadow"));

    // Add theme to collection
    if (render->colorSchemeCount < MAX_COLOR_SCHEMES) {
        render->colorSchemes[render->colorSchemeCount++] = theme;
    }

    cJSON_Delete(json);
    return true;
}

// Render a menu with options
void render_menu(App *app, const char *title, const char **options, int optionCount, int selectedIndex) {
    RenderSystem *render = &app->render;
    ColorScheme *scheme = &render->colorSchemes[render->activeColorScheme];

    // Clear screen with background color
    SDL_SetRenderDrawColor(render->renderer,
                          scheme->background.r,
                          scheme->background.g,
                          scheme->background.b,
                          255);
    SDL_RenderClear(render->renderer);

    // Draw title
    SDL_Surface *titleSurface = TTF_RenderText_Blended(
        render->fonts[2], title, scheme->accent);
    SDL_Texture *titleTexture = SDL_CreateTextureFromSurface(
        render->renderer, titleSurface);

    SDL_Rect titleRect = {
        (SCREEN_WIDTH - titleSurface->w) / 2,  // Center horizontally
        30,                                    // Top position
        titleSurface->w,
        titleSurface->h
    };

    SDL_RenderCopy(render->renderer, titleTexture, NULL, &titleRect);
    SDL_FreeSurface(titleSurface);

    // Draw menu options
    int startY = 120;
    int itemHeight = 50;

    for (int i = 0; i < optionCount; i++) {
        SDL_Rect itemRect = {
            100,                       // Left margin
            startY + i * itemHeight,   // Vertical position
            SCREEN_WIDTH - 200,        // Width with margins
            40                         // Item height
        };

        // Draw selection highlight for selected item
        if (i == selectedIndex) {
            SDL_SetRenderDrawColor(render->renderer,
                scheme->highlight.r,
                scheme->highlight.g,
                scheme->highlight.b,
                255);
            SDL_RenderFillRect(render->renderer, &itemRect);

            // Draw text in contrast color
            SDL_Surface *textSurface = TTF_RenderText_Blended(
                render->fonts[1], options[i], scheme->background);
            SDL_Texture *textTexture = SDL_CreateTextureFromSurface(
                render->renderer, textSurface);

            SDL_Rect textRect = {
                itemRect.x + 15,          // Left padding
                itemRect.y + 10,          // Center in item
                textSurface->w,
                textSurface->h
            };

            SDL_RenderCopy(render->renderer, textTexture, NULL, &textRect);
            SDL_FreeSurface(textSurface);
            SDL_DestroyTexture(textTexture);
        } else {
            // Draw normal text for unselected items
            SDL_Surface *textSurface = TTF_RenderText_Blended(
                render->fonts[1], options[i], scheme->text);
            SDL_Texture *textTexture = SDL_CreateTextureFromSurface(
                render->renderer, textSurface);

            SDL_Rect textRect = {
                itemRect.x + 15,
                itemRect.y + 10,
                textSurface->w,
                textSurface->h
            };

            SDL_RenderCopy(render->renderer, textTexture, NULL, &textRect);
            SDL_FreeSurface(textSurface);
            SDL_DestroyTexture(textTexture);
        }
    }

    // Draw instructions at bottom
    SDL_Surface *instructSurface = TTF_RenderText_Blended(
        render->fonts[0], "DY/DX: Navigate | A: Select | B: Cancel",
        scheme->shadow);
    SDL_Texture *instructTexture = SDL_CreateTextureFromSurface(
        render->renderer, instructSurface);

    SDL_Rect instructRect = {
        (SCREEN_WIDTH - instructSurface->w) / 2,  // Center
        SCREEN_HEIGHT - 40,                       // Bottom
        instructSurface->w,
        instructSurface->h
    };

    SDL_RenderCopy(render->renderer, instructTexture, NULL, &instructRect);
    SDL_FreeSurface(instructSurface);
    SDL_DestroyTexture(instructTexture);

    // Update screen
    SDL_RenderPresent(render->renderer);
}

// Interactive menu with navigation and selection
int selection_menu(App *app, const char *title, const char **options, int optionCount) {
    if (optionCount == 0) return -1;

    int selectedIndex = 0;
    bool menuActive = true;

    // Menu loop
    while (menuActive && app->running) {
        // Handle SDL events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                app->running = false;
                return -1;
            }
        }

        // Process input
        process_input(app);

        // Handle navigation
        if (is_button_pressed(&app->input, BUTTON_DY)) {
            selectedIndex = (selectedIndex + 1) % optionCount;
        }

        if (is_button_pressed(&app->input, BUTTON_DX)) {
            selectedIndex = (selectedIndex - 1 + optionCount) % optionCount;
        }

        // Handle selection
        if (is_button_pressed(&app->input, BUTTON_A)) {
            return selectedIndex;  // Return selected option
        }

        // Handle cancel
        if (is_button_pressed(&app->input, BUTTON_B)) {
            return -1;  // Cancel selection
        }

        // Render the menu
        render_menu(app, title, options, optionCount, selectedIndex);

        // Frame timing
        SDL_Delay(16);  // ~60fps
    }

    return -1;  // Default cancel if loop exits abnormally
}

// Function to show theme selection menu
int theme_selection_menu(App *app) {
    // Build array of theme names
    const char *themeNames[MAX_COLOR_SCHEMES];
    for (int i = 0; i < app->render.colorSchemeCount; i++) {
        themeNames[i] = app->render.colorSchemes[i].name;
    }

    // Display theme selection menu
    int selectedTheme = selection_menu(app, "Select Theme",
                                     themeNames,
                                     app->render.colorSchemeCount);

    // Apply selected theme if valid
    if (selectedTheme >= 0) {
        app->render.activeColorScheme = selectedTheme;
        printf("Theme changed to: %s\n", themeNames[selectedTheme]);
    }

    return selectedTheme;
}

int main() {
    // Initialize app
    App app = {0};
    initialize_sdl(&app);
    initialize_input(&app.input);
    initialize_render(&app.render);

    // Load theme files from themes directory
    load_theme_files(&app);

    // Main loop
    app.running = true;
    while (app.running) {
        // Process input
        process_input(&app);

        // Open theme menu when START is pressed
        if (is_button_pressed(&app.input, BUTTON_START)) {
            theme_selection_menu(&app);
        }

        // Exit with B button
        if (is_button_pressed(&app.input, BUTTON_B)) {
            app.running = false;
        }

        // Render main screen
        render_frame(&app);
        SDL_Delay(16);  // ~60fps
    }

    cleanup(&app);
    return 0;
}
