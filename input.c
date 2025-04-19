/* input.c */
#include "input.h"
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/select.h>

void input_init(InputState* input) {
    input->fd = open("/dev/input/event1", O_RDONLY);
    memset(input->current, 0, sizeof(input->current));
    memset(input->previous, 0, sizeof(input->previous));
}

void input_update(InputState* input) {
    memcpy(input->previous, input->current, sizeof(input->previous));

    if (input->fd >= 0) {
        struct input_event ev;
        fd_set fds;
        struct timeval tv = {0, 0};

        FD_ZERO(&fds);
        FD_SET(input->fd, &fds);

        while (select(input->fd+1, &fds, NULL, NULL, &tv) > 0) {
            if (read(input->fd, &ev, sizeof(ev)) == sizeof(ev) && ev.type == EV_KEY) {
                input->current[ev.code] = (ev.value != 0);
            }
        }
    }
}

bool button_pressed(InputState* input, int button) {
    return input->current[button] && !input->previous[button];
}

bool button_down(InputState* input, int button) {
    return input->current[button];
}

bool combo_pressed(InputState* input, int btn1, int btn2) {
    return input->current[btn1] && input->current[btn2];
}

void input_cleanup(InputState* input) {
    if (input->fd >= 0) close(input->fd);
}
