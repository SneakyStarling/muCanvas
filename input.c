#include "input.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>

bool input_init(InputState* input, const char* device_path) {
    // Open input device
    input->fd = open(device_path, O_RDONLY | O_NONBLOCK);
    if (input->fd < 0) {
        perror("Failed to open input device");
        return false;
    }

    // Initialize state arrays
    memset(input->current, 0, sizeof(input->current));
    memset(input->previous, 0, sizeof(input->previous));

    // Initialize joystick values
    input->joystick_left_y = 0;
    input->joystick_left_z = 0;
    input->joystick_right_y = 0;
    input->joystick_right_z = 0;

    return true;
}

void input_cleanup(InputState* input) {
    if (input->fd >= 0) {
        close(input->fd);
        input->fd = -1;
    }
}

void input_update(InputState* input) {
    // Save previous button states
    memcpy(input->previous, input->current, sizeof(input->previous));

    // Process all pending input events
    if (input->fd >= 0) {
        struct input_event ev;
        fd_set readfds;
        struct timeval tv = {0, 0}; // Non-blocking

        FD_ZERO(&readfds);
        FD_SET(input->fd, &readfds);

        while (select(input->fd + 1, &readfds, NULL, NULL, &tv) > 0) {
            ssize_t bytes = read(input->fd, &ev, sizeof(ev));
            if (bytes != sizeof(ev)) break;

            switch (ev.type) {
                case EV_KEY:
                    // Update button state
                    if (ev.code < MAX_BUTTONS) {
                        input->current[ev.code] = (ev.value != 0);
                        if (ev.value == 1) { // Button press
                            printf("Button %d pressed\n", ev.code);
                        } else if (ev.value == 0) { // Button release
                            printf("Button %d released\n", ev.code);
                        }
                    }
                    break;

                case EV_ABS:
                    // Update joystick values
                    switch (ev.code) {
                        case AXIS_LEFT_Y:
                            input->joystick_left_y = ev.value;
                            break;
                        case AXIS_LEFT_Z:
                            input->joystick_left_z = ev.value;
                            break;
                        case AXIS_RIGHT_RY:
                            input->joystick_right_y = ev.value;
                            break;
                        case AXIS_RIGHT_RZ:
                            input->joystick_right_z = ev.value;
                            break;
                    }
                    break;
            }

            // Check if there are more events
            FD_ZERO(&readfds);
            FD_SET(input->fd, &readfds);
            tv.tv_sec = 0;
            tv.tv_usec = 0;
        }
    }
}

bool button_pressed(InputState* input, int button) {
    return input->current[button] && !input->previous[button];
}

bool button_released(InputState* input, int button) {
    return !input->current[button] && input->previous[button];
}

bool button_down(InputState* input, int button) {
    return input->current[button];
}

bool combo_down(InputState* input, int btn1, int btn2) {
    return input->current[btn1] && input->current[btn2];
}

int16_t get_left_stick_y(InputState* input) {
    return (abs(input->joystick_left_y) > JOYSTICK_DEADZONE) ? input->joystick_left_y : 0;
}

int16_t get_left_stick_x(InputState* input) {
    return (abs(input->joystick_left_z) > JOYSTICK_DEADZONE) ? input->joystick_left_z : 0;
}

int16_t get_right_stick_y(InputState* input) {
    return (abs(input->joystick_right_y) > JOYSTICK_DEADZONE) ? input->joystick_right_y : 0;
}

int16_t get_right_stick_x(InputState* input) {
    return (abs(input->joystick_right_z) > JOYSTICK_DEADZONE) ? input->joystick_right_z : 0;
}
