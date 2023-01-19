#include <menu/ConfigMenuState.h>
#include <menu/BatchBackupState.h>
#include <menu/WiiUTitleListState.h>
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
        consolePrintPos(M_OFF, 2, LanguageUtils::gettext("   Wii U Save Management (%u Title%s)"), this->wiiuTitlesCount,
                        (this->wiiuTitlesCount > 1) ? "s" : "");
        consolePrintPos(M_OFF, 3, LanguageUtils::gettext("   vWii Save Management (%u Title%s)"), this->vWiiTitlesCount,
                        (this->vWiiTitlesCount > 1) ? "s" : "");
        consolePrintPos(M_OFF, 4, LanguageUtils::gettext("   Batch Backup"));
        consolePrintPos(M_OFF, 2 + cursor, "\u2192");
        consolePrintPosAligned(17, 4, 2, LanguageUtils::gettext("\uE002: Options \ue000: Select Mode"));
    }
}

ApplicationState::eSubState MainMenuState::update(Input *input) {
    if (this->state == STATE_MAIN_MENU) {
        if(input->get(TRIGGER, PAD_BUTTON_A)) {
            switch(cursor) {
                case 0:
                    this->state = STATE_DO_SUBSTATE;
                    this->subState = std::make_unique<WiiUTitleListState>(this->wiiutitles, this->wiiuTitlesCount);
                    break;
                case 1:
                    this->state = STATE_DO_SUBSTATE;
                    this->subState = std::make_unique<vWiiTitleListState>(this->wiititles, this->vWiiTitlesCount);
                    break;
                case 2:
                    this->state = STATE_DO_SUBSTATE;
                    this->subState = std::make_unique<BatchBackupState>(this->wiiutitles, this->wiititles, this->wiiuTitlesCount, this->vWiiTitlesCount);
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
            if(cursor-- == -1)
                cursor++;
        if(input->get(TRIGGER, PAD_BUTTON_DOWN))
            if(cursor++ == ENTRYCOUNT + 1)
                cursor--;
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