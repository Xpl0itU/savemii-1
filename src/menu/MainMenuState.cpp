#include <menu/ConfigMenuState.h>
#include <menu/MainMenuState.h>

#include <savemng.h>
#include <utils/InputUtils.h>
#include <utils/LanguageUtils.h>

#define ENTRYCOUNT 3

static int cursor = 0;

void MainMenuState::render() {
    if (this->state == STATE_DO_SUBSTATE) {
        if (this->subState == nullptr) {
            OSFatal("SubState was null");
        }
        this->subState->render();
        return;
    }
    if(this->state = STATE_MAIN_MENU) {
        consolePrintPos(M_OFF, 2, LanguageUtils::gettext("   Wii U Save Management (%u Title%s)"), wiiuTitlesCount,
                        (wiiuTitlesCount > 1) ? "s" : "");
        consolePrintPos(M_OFF, 3, LanguageUtils::gettext("   vWii Save Management (%u Title%s)"), vWiiTitlesCount,
                        (vWiiTitlesCount > 1) ? "s" : "");
        consolePrintPos(M_OFF, 4, LanguageUtils::gettext("   Batch Backup"));
        consolePrintPos(M_OFF, 2 + cursor, "\u2192");
        consolePrintPosAligned(17, 4, 2, LanguageUtils::gettext("\uE002: Options \ue000: Select Mode"));
    }
}

ApplicationState::eSubState MainMenuState::update(Input *input) {
    input->read();
    if (this->state == STATE_MAIN_MENU) {
        if(input->get(TRIGGER, PAD_BUTTON_A)) {
            switch(cursor) {
                case 0:
                    this->state = STATE_DO_SUBSTATE;
                    this->subState = std::make_unique<WiiUTitleListState>();
                    break;
                case 1:
                    this->state = STATE_DO_SUBSTATE;
                    this->subState = std::make_unique<vWiiTitleListState>();
                    break;
                case 2:
                    this->state = STATE_DO_SUBSTATE;
                    this->subState = std::make_unique<BatchBackupState>();
                    break;
                default:
                    break;
            }
        }
        if(input->get(TRIGGER, PAD_BUTTON_X)) {
            this->state = STATE_DO_SUBSTATE;
            this->subState = std::make_unique<ConfigMenuState>();
        }
        if(input->get(TRIGGER, PAD_BUTTON_UP))
            cursor--;
        if(input->get(TRIGGER, PAD_BUTTON_DOWN))
            cursor++;
    } else if (this->state == STATE_DO_SUBSTATE) {
        auto retSubState = this->subState->update(input);
        if (retSubState == SUBSTATE_RUNNING) {
            // keep running.
            return SUBSTATE_RUNNING;
        } else if (retSubState == SUBSTATE_RETURN) {
            this->subState.reset();
            this->state = STATE_MAIN_MENU;
        }
    }
    return SUBSTATE_RUNNING;
}