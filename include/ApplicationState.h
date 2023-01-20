#pragma once

#include <utils/InputUtils.h>

#define MIN_MENU_ID 0
#define MAX_MENU_ID 3

enum Task {
    backup = 0,
    restore = 1,
    wipe = 2,
    importLoadiine = 3,
    exportLoadiine = 4,
    copytoOtherDevice = 5
};

class ApplicationState {
public:
    enum eSubState {
        SUBSTATE_RUNNING,
        SUBSTATE_RETURN,
    };

    virtual void render() = 0;
    virtual eSubState update(Input *input) = 0;
};