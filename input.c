/* input.c */
#include "input.h"
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

void input_init(InputState* input) {
    input->fd = open("/dev/input/event1", O_RDONLY | O_NONBLOCK);
    memset(input->current_buttons, 0, sizeof(input->current_buttons));
    memset(input->prev_buttons, 0, sizeof(input->prev_buttons));
    input->left_stick_x = 0.0f;
    input->left_stick_y = 0.0f;
    input->right_stick_x = 0.0f;
    input->right_stick_y = 0.0f;
}

void input_update(InputState* input) {
    struct input_event ev;

    // Save previous states
    memcpy(input->prev_buttons, input->current_buttons, sizeof(input->prev_buttons));

    // Process all pending events
    while(read(input->fd, &ev, sizeof(ev)) == sizeof(ev)) {
        switch(ev.type) {
            case EV_KEY:
                if(ev.code < sizeof(input->current_buttons)) {
                    input->current_buttons[ev.code] = ev.value;
                }
                break;

            case EV_ABS:
                // Normalize analog values to [-1.0, 1.0]
                float normalized = (float)ev.value / 32767.0f;

                switch(ev.code) {
                    case AXIS_LEFT_X:
                        input->left_stick_x = fabsf(normalized) > JOYSTICK_DEADZONE ? normalized : 0.0f;
                        break;
                    case AXIS_LEFT_Y:
                        input->left_stick_y = fabsf(normalized) > JOYSTICK_DEADZONE ? normalized : 0.0f;
                        break;
                    case AXIS_RIGHT_X:
                        input->right_stick_x = fabsf(normalized) > JOYSTICK_DEADZONE ? normalized : 0.0f;
                        break;
                    case AXIS_RIGHT_Y:
                        input->right_stick_y = fabsf(normalized) > JOYSTICK_DEADZONE ? normalized : 0.0f;
                        break;
                }
                break;
        }
    }
}

void input_cleanup(InputState* input) {
    if(input->fd >= 0) close(input->fd);
}

bool button_pressed(InputState* input, int button) {
    return input->current_buttons[button] && !input->prev_buttons[button];
}

bool button_released(InputState* input, int button) {
    return !input->current_buttons[button] && input->prev_buttons[button];
}

bool button_down(InputState* input, int button) {
    return input->current_buttons[button];
}

float get_left_stick_x(InputState* input) { return input->left_stick_x; }
float get_left_stick_y(InputState* input) { return input->left_stick_y; }
float get_right_stick_x(InputState* input) { return input->right_stick_x; }
float get_right_stick_y(InputState* input) { return input->right_stick_y; }

bool combo_pressed(InputState* input, int btn1, int btn2) {
    return button_pressed(input, btn1) && button_down(input, btn2) ||
           button_pressed(input, btn2) && button_down(input, btn1);
}

bool combo_down(InputState* input, int btn1, int btn2) {
    return button_down(input, btn1) && button_down(input, btn2);
}
