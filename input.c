#include "input.h"
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <stdio.h>
#include <sys/select.h>

bool input_init(InputState* input) {
    // Initialize joystick subsystem
    SDL_InitSubSystem(SDL_INIT_JOYSTICK);

    // Set up evdev input
    input->fd = open("/dev/input/event1", O_RDONLY | O_NONBLOCK);
    if (input->fd < 0) {
        perror("Failed to open input device");
    }

    // Initialize button states
    memset(input->current, 0, sizeof(input->current));
    memset(input->previous, 0, sizeof(input->previous));

    // Initialize joystick
    input->joystick = NULL;
    if (SDL_NumJoysticks() > 0) {
        input->joystick = SDL_JoystickOpen(0);
        if (input->joystick) {
            printf("Joystick detected: %s\n", SDL_JoystickName(input->joystick));
            printf("Number of axes: %d\n", SDL_JoystickNumAxes(input->joystick));
            printf("Number of buttons: %d\n", SDL_JoystickNumButtons(input->joystick));
        } else {
            printf("Failed to open joystick: %s\n", SDL_GetError());
        }
    }

    // Initialize joystick values
    input->left_stick_x = 0;
    input->left_stick_y = 0;
    input->right_stick_x = 0;
    input->right_stick_y = 0;

    return true;
}

void input_cleanup(InputState* input) {
    if (input->fd >= 0) {
        close(input->fd);
        input->fd = -1;
    }

    if (input->joystick) {
        SDL_JoystickClose(input->joystick);
        input->joystick = NULL;
    }
}

float normalize_axis(int16_t axis) {
    // Convert axis value to float in range [-1.0, 1.0]
    // Apply deadzone
    float normalized = (float)axis / 32767.0f;
    if (fabs(normalized) < (float)JOYSTICK_DEADZONE / 32767.0f) {
        return 0.0f;
    }
    return normalized;
}

void input_update(InputState* input) {
    // Store previous state
    memcpy(input->previous, input->current, sizeof(input->previous));

    // Update joystick state if using SDL joystick
    if (input->joystick) {
        SDL_JoystickUpdate();

        // Read analog sticks
        input->left_stick_x = SDL_JoystickGetAxis(input->joystick, AXIS_Z);
        input->left_stick_y = SDL_JoystickGetAxis(input->joystick, AXIS_Y);
        input->right_stick_x = SDL_JoystickGetAxis(input->joystick, AXIS_RZ);
        input->right_stick_y = SDL_JoystickGetAxis(input->joystick, AXIS_RY);
    }

    // Read from evdev input device
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
                    // Regular button event
                    if (ev.code < MAX_BUTTONS) {
                        input->current[ev.code] = (ev.value != 0);
                    }
                    break;

                case EV_ABS:
                    switch (ev.code) {
                        // Handle D-pad axes
                        case AXIS_DPAD_X:
                            input->current[BUTTON_DPAD_LEFT] = (ev.value < 0);
                            input->current[BUTTON_DPAD_RIGHT] = (ev.value > 0);
                            break;

                        case AXIS_DPAD_Y:
                            input->current[BUTTON_DPAD_UP] = (ev.value < 0);
                            input->current[BUTTON_DPAD_DOWN] = (ev.value > 0);
                            break;

                        // Also update analog stick values from evdev
                        case AXIS_Y:
                            input->left_stick_y = ev.value;
                            break;

                        case AXIS_Z:
                            input->left_stick_x = ev.value;
                            break;

                        case AXIS_RY:
                            input->right_stick_y = ev.value;
                            break;

                        case AXIS_RZ:
                            input->right_stick_x = ev.value;
                            break;
                    }
                    break;
            }

            // Check for more events
            FD_ZERO(&readfds);
            FD_SET(input->fd, &readfds);
            tv.tv_sec = 0;
            tv.tv_usec = 0;
        }
    }
}

bool button_pressed(InputState* input, int button) {
    if (button >= MAX_BUTTONS) return false;
    return input->current[button] && !input->previous[button];
}

bool button_released(InputState* input, int button) {
    if (button >= MAX_BUTTONS) return false;
    return !input->current[button] && input->previous[button];
}

bool button_down(InputState* input, int button) {
    if (button >= MAX_BUTTONS) return false;
    return input->current[button];
}

bool combo_down(InputState* input, int button1, int button2) {
    return button_down(input, button1) && button_down(input, button2);
}

bool combo_pressed(InputState* input, int button1, int button2) {
    return (button_pressed(input, button1) && button_down(input, button2)) ||
           (button_pressed(input, button2) && button_down(input, button1));
}

float get_left_stick_x(InputState* input) {
    return normalize_axis(input->left_stick_x);
}

float get_left_stick_y(InputState* input) {
    return normalize_axis(input->left_stick_y);
}

float get_right_stick_x(InputState* input) {
    return normalize_axis(input->right_stick_x);
}

float get_right_stick_y(InputState* input) {
    return normalize_axis(input->right_stick_y);
}
