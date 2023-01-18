#pragma once

#define MIN_MENU_ID 0
#define MAX_MENU_ID 3

enum Menu {
    mainMenu = 0,
    selectTitle = 1,
    selectTask = 2,
    selectOptions = 3,
    configMenu = 4,
};

enum Mode {
    WiiU = 0,
    vWii = 1,
    batchBackup = 2
};

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
    virtual void update() = 0;
};