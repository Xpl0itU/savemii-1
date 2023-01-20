#pragma once

#include <cstring>
#include <padscore/kpad.h>
#include <vpad/input.h>

typedef enum Button {
    PAD_BUTTON_A,
    PAD_BUTTON_B,
    PAD_BUTTON_X,
    PAD_BUTTON_UP,
    PAD_BUTTON_DOWN,
    PAD_BUTTON_LEFT,
    PAD_BUTTON_RIGHT,
    PAD_BUTTON_L,
    PAD_BUTTON_R,
    PAD_BUTTON_ANY
} Button;

typedef enum ButtonState {
    TRIGGER,
    HOLD,
    RELEASE
} ButtonState;

class Input {
public:
    void read() __attribute__((hot));
    bool get(ButtonState state, Button button) const __attribute__((hot));

private:
    VPADStatus vpad_status;
    VPADReadError vpad_error;
    KPADStatus kpad[4], kpad_status;
};
