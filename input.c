#include "input.h"
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <stdio.h>
#include <sys/select.h>

bool input_init(InputState* input) {
    input->fd = open("/dev/input/event1", O_RDONLY | O_NONBLOCK);
    if (input->fd < 0) {
        perror("Failed to open input device");
        return false;
    }

    memset(input->current, 0, sizeof(input->current));
    memset(input->previous, 0, sizeof(input->previous));

    // Initialize joystick values to center
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
}

float normalize_axis(int16_t axis) {
    // Convert to -1.0~1.0 float
    float normalized = (float)axis / 4096.0f;

    // Apply deadzone (adjust percentage as needed)
    if (fabs(normalized) < 0.05f) {
        return 0.0f;
    }

    return normalized;
}

void input_update(InputState* input) {
    // Store previous state
    memcpy(input->previous, input->current, sizeof(input->previous));

    // Read from evdev input device
    if (input->fd >= 0) {
        struct input_event ev;
        fd_set readfds;
        struct timeval tv = {0, 0}; // Non-blocking

        while (1) {
            FD_ZERO(&readfds);
            FD_SET(input->fd, &readfds);

            int ret = select(input->fd + 1, &readfds, NULL, NULL, &tv);
            if (ret <= 0) break; // No more events

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
    if (!input) return 0.0f;
    return normalize_axis(input->left_stick_x);
}

float get_left_stick_y(InputState* input) {
    if (!input) return 0.0f;
    return normalize_axis(input->left_stick_y);
}

float get_right_stick_x(InputState* input) {
    if (!input) return 0.0f;
    return normalize_axis(input->right_stick_x);
}

float get_right_stick_y(InputState* input) {
    if (!input) return 0.0f;
    return normalize_axis(input->right_stick_y);
}
