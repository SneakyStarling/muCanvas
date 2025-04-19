#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include <stdint.h>
#include <linux/input.h>

// Button definitions for Anbernic RG40XX H
#define BUTTON_A        304
#define BUTTON_B        305
#define BUTTON_Y        306
#define BUTTON_X        307
#define BUTTON_L1       308
#define BUTTON_R1       309
#define BUTTON_L2       314
#define BUTTON_R2       315
#define BUTTON_DY       17
#define BUTTON_DX       16
#define BUTTON_SELECT   310
#define BUTTON_START    311
#define BUTTON_MENU     312
#define BUTTON_VOLUP    114
#define BUTTON_VOLDOWN  115

// Joystick axes
#define AXIS_LEFT_Y     0  // ABS_Y
#define AXIS_LEFT_Z     1  // ABS_Z
#define AXIS_RIGHT_RY   4  // ABS_RY
#define AXIS_RIGHT_RZ   5  // ABS_RZ

#define MAX_BUTTONS 512
#define JOYSTICK_DEADZONE 4000

typedef struct {
    int fd;                          // Input device file descriptor
    bool current[MAX_BUTTONS];       // Current button states
    bool previous[MAX_BUTTONS];      // Previous button states
    int16_t joystick_left_y;         // Left joystick Y axis
    int16_t joystick_left_z;         // Left joystick Z axis (X equivalent)
    int16_t joystick_right_y;        // Right joystick Y axis
    int16_t joystick_right_z;        // Right joystick Z axis (X equivalent)
} InputState;

// Initialization and cleanup
bool input_init(InputState* input);
void input_cleanup(InputState* input);

// Update and state tracking
void input_update(InputState* input);

// Button state queries
bool button_pressed(InputState* input, int button);   // Button just pressed this frame
bool button_released(InputState* input, int button);  // Button just released this frame
bool button_down(InputState* input, int button);      // Button currently held down

// Button combination checks
bool combo_down(InputState* input, int btn1, int btn2);  // Both buttons held down

// Joystick access
int16_t get_left_stick_y(InputState* input);
int16_t get_left_stick_x(InputState* input);
int16_t get_right_stick_y(InputState* input);
int16_t get_right_stick_x(InputState* input);

#endif // INPUT_H
