/* input.h */
#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>

#define MAX_BUTTONS 512
#define BUTTON_A 304
#define BUTTON_BUTTON_CODE 305
#define BUTTON_L1 308
#define BUTTON_L2 314

typedef struct {
    int fd;
    bool current[MAX_BUTTONS];
    bool previous[MAX_BUTTONS];
} InputState;

void input_init(InputState* input);
void input_update(InputState* input);
bool button_pressed(InputState* input, int button);
bool button_down(InputState* input, int button);
bool combo_pressed(InputState* input, int btn1, int btn2);
void input_cleanup(InputState* input);

#endif
