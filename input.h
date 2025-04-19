/* input.h */
#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>

// Button codes (based on common Anbernic mappings and Linux input events)
#define BUTTON_A         0x130   // 304 (Confirm/Right face button)
#define BUTTON_B         0x131   // 305 (Back/Left face button)
#define BUTTON_X         0x133   // 307
#define BUTTON_Y         0x134   // 308
#define BUTTON_L1        0x136   // 310
#define BUTTON_L2        0x138   // 312
#define BUTTON_R1        0x137   // 311
#define BUTTON_R2        0x139   // 313
#define BUTTON_START     0x13a   // 314
#define BUTTON_SELECT    0x13b   // 315
#define BUTTON_MENU      0x13c   // 316 (Function/F key)
#define BUTTON_POWER     0x74    // 116 (Power button)

// Analog stick axes (from evdev ABS codes)
#define AXIS_LEFT_X      0x00
#define AXIS_LEFT_Y      0x01
#define AXIS_RIGHT_X     0x03
#define AXIS_RIGHT_Y     0x04

// Deadzone threshold for analog sticks (12% of max range)
#define JOYSTICK_DEADZONE 0.12f

typedef struct {
    int fd;
    bool current_buttons[0x200];
    bool prev_buttons[0x200];
    float left_stick_x;
    float left_stick_y;
    float right_stick_x;
    float right_stick_y;
} InputState;

void input_init(InputState* input);
void input_update(InputState* input);
void input_cleanup(InputState* input);

// Button queries
bool button_pressed(InputState* input, int button);
bool button_released(InputState* input, int button);
bool button_down(InputState* input, int button);

// Analog stick queries
float get_left_stick_x(InputState* input);
float get_left_stick_y(InputState* input);
float get_right_stick_x(InputState* input);
float get_right_stick_y(InputState* input);

// Combination checks
bool combo_pressed(InputState* input, int btn1, int btn2);
bool combo_down(InputState* input, int btn1, int btn2);

#endif
