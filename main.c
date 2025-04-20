#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <math.h>
#include "input.h"
#include "render.h"

#define SQUARE_SIZE 100

int main() {
    RenderContext ctx;
    render_init(&ctx);

    InputState input;
    if (!input_init(&input)) {
        printf("Input initialization failed, continuing without input\n");
    }

    // Centered square position
    int square_x = 640 / 2;
    int square_y = 480 / 2;

    bool running = true;
    while (running) {
        // Handle SDL events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
        }

        input_update(&input);

        // Exit with B
        if (button_pressed(&input, BUTTON_B)) {
            running = false;
        }

        // Move with D-pad
        int move_speed = 5;
        if (button_down(&input, BUTTON_DPAD_LEFT))  square_x -= move_speed;
        if (button_down(&input, BUTTON_DPAD_RIGHT)) square_x += move_speed;
        if (button_down(&input, BUTTON_DPAD_UP))    square_y -= move_speed;
        if (button_down(&input, BUTTON_DPAD_DOWN))  square_y += move_speed;

        // Move with left stick (analog)
        float left_x = get_left_stick_x(&input);
        float left_y = get_left_stick_y(&input);
        if (fabs(left_x) > 0.01f) square_x += (int)(left_x * 5.0f);
        if (fabs(left_y) > 0.01f) square_y += (int)(left_y * 5.0f);

        // Keep square within bounds
        if (square_x < SQUARE_SIZE/2) square_x = SQUARE_SIZE/2;
        if (square_y < SQUARE_SIZE/2) square_y = SQUARE_SIZE/2;
        if (square_x > 640 - SQUARE_SIZE/2) square_x = 640 - SQUARE_SIZE/2;
        if (square_y > 480 - SQUARE_SIZE/2) square_y = 480 - SQUARE_SIZE/2;

        draw_clear(&ctx);

        draw_text(&ctx, "A: red square | L1+L2: green squares | D-pad/Stick: move square", 50, 50);

        // Draw the red square if A is held
        if (button_down(&input, BUTTON_A)) {
            draw_square(&ctx, square_x, square_y, SQUARE_SIZE);
        }

        // Draw side squares if L1+L2 are held
        if (combo_down(&input, BUTTON_L1, BUTTON_L2)) {
            draw_square(&ctx, square_x - SQUARE_SIZE - 20, square_y, SQUARE_SIZE);
            draw_square(&ctx, square_x + SQUARE_SIZE + 20, square_y, SQUARE_SIZE);
        }

        // Print square location on screen
        char buf[64];
        snprintf(buf, sizeof(buf), "Square: X=%d, Y=%d", square_x, square_y);
        draw_text(&ctx, buf, 10, 120);

        // Print joystick values on screen
        snprintf(buf, sizeof(buf), "Left Stick: X=%.2f, Y=%.2f", left_x, left_y);
        draw_text(&ctx, buf, 10, 150);

        float right_x = get_right_stick_x(&input);
        float right_y = get_right_stick_y(&input);
        snprintf(buf, sizeof(buf), "Right Stick: X=%.2f, Y=%.2f", right_x, right_y);
        draw_text(&ctx, buf, 10, 180);

        render_present(&ctx);
        SDL_Delay(16);
    }

    input_cleanup(&input);
    render_cleanup(&ctx);
    return 0;
}
