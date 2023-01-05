#include <state.h>
#include <coreinit/foreground.h>
#include <whb/proc.h>

static bool aroma;

static bool isAroma() {
    OSDynLoad_Module mod;
    aroma = OSDynLoad_Acquire("homebrew_kernel", &mod) == OS_DYNLOAD_OK;
    if (aroma)
        OSDynLoad_Release(mod);
    return aroma;
}

bool AppRunning() {
    if(isAroma()) {
        bool app = true;
        if (OSIsMainCore()) {
            switch (ProcUIProcessMessages(true)) {
                case PROCUI_STATUS_EXITING:
                    // Being closed, prepare to exit
                    app = false;
                    break;
                case PROCUI_STATUS_RELEASE_FOREGROUND:
                    // Free up MEM1 to next foreground app, deinit screen, etc.
                    ProcUIDrawDoneRelease();
                    break;
                case PROCUI_STATUS_IN_FOREGROUND:
                    // Executed while app is in foreground
                    app = true;
                    break;
                case PROCUI_STATUS_IN_BACKGROUND:
                    OSSleepTicks(OSMillisecondsToTicks(20));
                    break;
            }
        }

        return app;
    }
    return WHBProcIsRunning();
}

void initState() {
    if(isAroma()) {
        ProcUIInit(&OSSavesDone_ReadyToRelease);
        OSEnableHomeButtonMenu(true);
    } else
        WHBProcInit();
}

void shutdownState() {
    if(!isAroma())
        WHBProcShutdown();
    ProcUIShutdown();
}