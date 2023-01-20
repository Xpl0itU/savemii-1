#include <menu/BatchBackupState.h>
#include <savemng.h>
#include <utils/InputUtils.h>
#include <utils/LanguageUtils.h>

#include <coreinit/time.h>

#define ENTRYCOUNT 3

static int cursorPos = 0;

void BatchBackupState::render() {
    consolePrintPos(M_OFF, 2, LanguageUtils::gettext("   Backup All (%u Title%s)"), this->wiiuTitlesCount + this->vWiiTitlesCount,
                    ((this->wiiuTitlesCount + this->vWiiTitlesCount) > 1) ? "s" : "");
    consolePrintPos(M_OFF, 3, LanguageUtils::gettext("   Backup Wii U (%u Title%s)"), this->wiiuTitlesCount,
                    (this->wiiuTitlesCount > 1) ? "s" : "");
    consolePrintPos(M_OFF, 4, LanguageUtils::gettext("   Backup vWii (%u Title%s)"), this->vWiiTitlesCount,
                    (this->vWiiTitlesCount > 1) ? "s" : "");
    consolePrintPos(M_OFF, 2 + cursorPos, "\u2192");
    consolePrintPosAligned(17, 4, 2, LanguageUtils::gettext("\ue000: Backup  \ue001: Back"));
}

ApplicationState::eSubState BatchBackupState::update(Input *input) {
    if (input->get(TRIGGER, PAD_BUTTON_UP))
        if (--cursorPos == -1)
            ++cursorPos;
    if (input->get(TRIGGER, PAD_BUTTON_DOWN))
        if (++cursorPos == ENTRYCOUNT)
            --cursorPos;
    if (input->get(TRIGGER, PAD_BUTTON_B))
        return SUBSTATE_RETURN;
    if (input->get(TRIGGER, PAD_BUTTON_A)) {
        OSCalendarTime dateTime;
        switch (cursorPos) {
            case 0:
                dateTime.tm_year = 0;
                backupAllSave(this->wiiutitles, this->wiiuTitlesCount, &dateTime);
                backupAllSave(this->wiititles, this->vWiiTitlesCount, &dateTime);
                DrawUtils::setRedraw(true);
                return SUBSTATE_RETURN;
            case 1:
                backupAllSave(this->wiiutitles, this->wiiuTitlesCount, nullptr);
                DrawUtils::setRedraw(true);
                return SUBSTATE_RETURN;
            case 2:
                backupAllSave(this->wiititles, this->vWiiTitlesCount, nullptr);
                DrawUtils::setRedraw(true);
                return SUBSTATE_RETURN;
            default:
                return SUBSTATE_RETURN;
        }
    }
    return SUBSTATE_RUNNING;
}