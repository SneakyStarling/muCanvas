#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>

#define BUTTON_A        304
#define BUTTON_B        305
#define BUTTON_Y        306
#define BUTTON_X        307
#define BUTTON_L1       308
#define BUTTON_R1       309
#define BUTTON_L2       314
#define BUTTON_R2       315
#define BUTTON_SELECT   310
#define BUTTON_START    311
#define BUTTON_MENU     312
#define BUTTON_VOLUP    114
#define BUTTON_VOLDOWN  115

// D-pad virtual button codes (above 1000 to avoid conflicts)
#define BUTTON_DPAD_LEFT  1001
#define BUTTON_DPAD_RIGHT 1002
#define BUTTON_DPAD_UP    1003
#define BUTTON_DPAD_DOWN  1004

// Axis codes from evtest
#define AXIS_Y       1  // Left stick Y (ABS_Y)
#define AXIS_Z       2  // Left stick X (ABS_Z)
#define AXIS_RY      4  // Right stick Y (ABS_RY)
#define AXIS_RZ      5  // Right stick Z (ABS_RZ)
#define AXIS_DPAD_X  16
#define AXIS_DPAD_Y  17
// todo joystick press find with: evtest /dev/input/event1

// Constants
#define MAX_BUTTONS 2000
#define JOYSTICK_DEADZONE 4000  // Deadzone for analog sticks

typedef struct {
    int fd;                        // Input device file descriptor
    bool current[MAX_BUTTONS];     // Current button states
    bool previous[MAX_BUTTONS];    // Previous button states

    // Joystick state
    SDL_Joystick *joystick;        // SDL Joystick handle
    int16_t left_stick_x;          // Left stick X position (-32768 to 32767)
    int16_t left_stick_y;          // Left stick Y position (-32768 to 32767)
    int16_t right_stick_x;         // Right stick X position (-32768 to 32767)
    int16_t right_stick_y;         // Right stick Y position (-32768 to 32767)
} InputState;

// Initialization and cleanup
bool input_init(InputState* input);
void input_cleanup(InputState* input);

// Update and state tracking
void input_update(InputState* input);

// Button state queries
bool button_pressed(InputState* input, int button);
bool button_released(InputState* input, int button);
bool button_down(InputState* input, int button);
bool combo_pressed(InputState* input, int button1, int button2);
bool combo_down(InputState* input, int button1, int button2);

// Joystick state queries
float get_left_stick_x(InputState* input);
float get_left_stick_y(InputState* input);
float get_right_stick_x(InputState* input);
float get_right_stick_y(InputState* input);
float normalize_axis(int16_t axis);

#endif
