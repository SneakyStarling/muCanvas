#include "input.h"
#include "render.h"

int main() {
    InputState input;
    RenderContext render;

    input_init(&input);
    render_init(&render);

    int center_x = 640/2;
    int center_y = 480/2;

    bool running = true;
    while (running) {
        input_update(&input);

        // Handle input
        if (button_pressed(&input, BUTTON_B)) running = false;

        // Update rendering
        draw_clear(&render);

        if (button_down(&input, BUTTON_A)) {
            draw_square(&render, center_x, center_y, 100);
        }

        if (combo_pressed(&input, BUTTON_L1, BUTTON_L2)) {
            draw_square(&render, center_x - 150, center_y, 100);
            draw_square(&render, center_x + 150, center_y, 100);
        }

        draw_text(&render, "A: Red Square | L1+L2: Side Squares | B: Exit", 50, 50);
        render_present(&render);

        SDL_Delay(16);
    }

    input_cleanup(&input);
    render_cleanup(&render);
    return 0;
}
