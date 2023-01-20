#include <utils/InputUtils.h>

void Input::read() {
    VPADRead(VPAD_CHAN_0, &vpad_status, 1, &vpad_error);
    if (vpad_error != VPAD_READ_SUCCESS)
        memset(&vpad_status, 0, sizeof(VPADStatus));

    memset(&kpad_status, 0, sizeof(KPADStatus));
    WPADExtensionType controllerType;
    for (int i = 0; i < 4; i++) {
        if (WPADProbe((WPADChan) i, &controllerType) == 0) {
            KPADRead((WPADChan) i, &kpad_status, 1);
            break;
        }
    }
}

bool Input::get(ButtonState state, Button button) const {
    uint32_t vpadState = 0;
    uint32_t kpadState = 0;
    uint32_t kpadClassicState = 0;
    uint32_t kpadProState = 0;

    switch (state) {
        case TRIGGER:
            vpadState = vpad_status.trigger;
            kpadState = kpad_status.trigger;
            kpadClassicState = kpad_status.classic.trigger;
            kpadProState = kpad_status.pro.trigger;
            break;
        case HOLD:
            vpadState = vpad_status.hold;
            kpadState = kpad_status.hold;
            kpadClassicState = kpad_status.classic.hold;
            kpadProState = kpad_status.pro.hold;
            break;
        case RELEASE:
            vpadState = vpad_status.release;
            kpadState = kpad_status.release;
            kpadClassicState = kpad_status.classic.release;
            kpadProState = kpad_status.pro.release;
            break;
    }
    if ((vpadState != 0) || (kpadState != 0) || (kpadClassicState != 0) || (kpadProState != 0)) {
        switch (button) {
            case PAD_BUTTON_A:
                if (vpadState & VPAD_BUTTON_A) return true;
                if (kpadState & WPAD_BUTTON_A) return true;
                if (kpadClassicState & WPAD_CLASSIC_BUTTON_A) return true;
                if (kpadProState & WPAD_PRO_BUTTON_A) return true;
                break;
            case PAD_BUTTON_B:
                if (vpadState & VPAD_BUTTON_B) return true;
                if (kpadState & WPAD_BUTTON_B) return true;
                if (kpadClassicState & WPAD_CLASSIC_BUTTON_B) return true;
                if (kpadProState & WPAD_PRO_BUTTON_B) return true;
                break;
            case PAD_BUTTON_X:
                if (vpadState & VPAD_BUTTON_X) return true;
                if (kpadState & WPAD_BUTTON_1) return true;
                if (kpadClassicState & WPAD_CLASSIC_BUTTON_X) return true;
                if (kpadProState & WPAD_PRO_BUTTON_X) return true;
                break;
            case PAD_BUTTON_UP:
                if (vpadState & VPAD_BUTTON_UP) return true;
                if (vpadState & VPAD_STICK_L_EMULATION_UP) return true;
                if (kpadState & WPAD_BUTTON_UP) return true;
                if (kpadClassicState & WPAD_CLASSIC_BUTTON_UP) return true;
                if (kpadClassicState & WPAD_CLASSIC_STICK_L_EMULATION_UP) return true;
                if (kpadProState & WPAD_PRO_BUTTON_UP) return true;
                if (kpadProState & WPAD_PRO_STICK_L_EMULATION_UP) return true;
                break;
            case PAD_BUTTON_DOWN:
                if (vpadState & VPAD_BUTTON_DOWN) return true;
                if (vpadState & VPAD_STICK_L_EMULATION_DOWN) return true;
                if (kpadState & WPAD_BUTTON_DOWN) return true;
                if (kpadClassicState & WPAD_CLASSIC_BUTTON_DOWN) return true;
                if (kpadClassicState & WPAD_CLASSIC_STICK_L_EMULATION_DOWN) return true;
                if (kpadProState & WPAD_PRO_BUTTON_DOWN) return true;
                if (kpadProState & WPAD_PRO_STICK_L_EMULATION_DOWN) return true;
                break;
            case PAD_BUTTON_LEFT:
                if (vpadState & VPAD_BUTTON_LEFT) return true;
                if (vpadState & VPAD_STICK_L_EMULATION_LEFT) return true;
                if (kpadState & WPAD_BUTTON_LEFT) return true;
                if (kpadClassicState & WPAD_CLASSIC_BUTTON_LEFT) return true;
                if (kpadClassicState & WPAD_CLASSIC_STICK_L_EMULATION_LEFT) return true;
                if (kpadProState & WPAD_PRO_BUTTON_LEFT) return true;
                if (kpadProState & WPAD_PRO_STICK_L_EMULATION_LEFT) return true;
                break;
            case PAD_BUTTON_RIGHT:
                if (vpadState & VPAD_BUTTON_RIGHT) return true;
                if (vpadState & VPAD_STICK_L_EMULATION_RIGHT) return true;
                if (kpadState & WPAD_BUTTON_RIGHT) return true;
                if (kpadClassicState & WPAD_CLASSIC_BUTTON_RIGHT) return true;
                if (kpadClassicState & WPAD_CLASSIC_STICK_L_EMULATION_RIGHT) return true;
                if (kpadProState & WPAD_PRO_BUTTON_RIGHT) return true;
                if (kpadProState & WPAD_PRO_STICK_L_EMULATION_RIGHT) return true;
                break;
            case PAD_BUTTON_L:
                if (vpadState & VPAD_BUTTON_L) return true;
                if (kpadState & WPAD_BUTTON_MINUS) return true;
                if (kpadClassicState & WPAD_CLASSIC_BUTTON_L) return true;
                if (kpadProState & WPAD_PRO_TRIGGER_L) return true;
                break;
            case PAD_BUTTON_R:
                if (vpadState & VPAD_BUTTON_R) return true;
                if (kpadState & WPAD_BUTTON_PLUS) return true;
                if (kpadClassicState & WPAD_CLASSIC_BUTTON_R) return true;
                if (kpadProState & WPAD_PRO_TRIGGER_R) return true;
                break;
            case PAD_BUTTON_ANY:
                if (vpadState) return true;
                if (kpadState) return true;
                if (kpadClassicState) return true;
                if (kpadProState) return true;
                break;
        }
    }
    return false;
}