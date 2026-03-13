#ifndef _INPUT_H
#define _INPUT_H

#include <stdbool.h>
#include "utypes.h"

typedef struct Input {
    bool move_forward;
    bool move_backward;
    bool turn_left;
    bool turn_right;
    bool strafe_left;
    bool strafe_right;
} Input;

typedef enum InputState {
    INPUT_STATE_MOVE_FORWARD  = 1,
    INPUT_STATE_MOVE_BACKWARD = -1,
    INPUT_STATE_TURN_LEFT     = -2,
    INPUT_STATE_TURN_RIGHT    = 2,
    INPUT_STATE_STRAFE_LEFT   = -4,
    INPUT_STATE_STRAFE_RIGHT  = 4
} InputState;

#endif // _INPUT_H