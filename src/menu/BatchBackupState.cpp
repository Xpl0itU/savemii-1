#include <coreinit/time.h>
#include <coreinit/debug.h>
#include <menu/BatchBackupState.h>
#include <menu/BackupSetListState.h>
#include <menu/BatchBackupTitleSelectState.h>
#include <BackupSetList.h>
#include <savemng.h>
#include <utils/InputUtils.h>
#include <utils/LanguageUtils.h>
#include <utils/DrawUtils.h>
#include <utils/Colors.h>

#define ENTRYCOUNT 3

static int cursorPos = 0;

void BatchBackupState::render() {
    if (this->state == STATE_DO_SUBSTATE) {
        if (this->subState == nullptr) {
            OSFatal("SubState was null");
        }
        this->subState->render();
        return;
    }
    if (this->state == STATE_BATCH_BACKUP_MENU) {
        DrawUtils::setFontColor(COLOR_INFO);
        consolePrintPosAligned(0, 4, 1,LanguageUtils::gettext("Batch Backup"));
        DrawUtils::setFontColorByCursor(COLOR_TEXT,COLOR_TEXT_AT_CURSOR,cursorPos,0);
        consolePrintPos(M_OFF, 2, LanguageUtils::gettext("   Backup All (%u Title%s)"), this->wiiuTitlesCount + this->vWiiTitlesCount,
                        ((this->wiiuTitlesCount + this->vWiiTitlesCount) > 1) ? "s" : "");
        DrawUtils::setFontColorByCursor(COLOR_TEXT,COLOR_TEXT_AT_CURSOR,cursorPos,1);
        consolePrintPos(M_OFF, 3, LanguageUtils::gettext("   Backup Wii U (%u Title%s)"), this->wiiuTitlesCount,
                        (this->wiiuTitlesCount > 1) ? "s" : "");
        DrawUtils::setFontColorByCursor(COLOR_TEXT,COLOR_TEXT_AT_CURSOR,cursorPos,2);
        consolePrintPos(M_OFF, 4, LanguageUtils::gettext("   Backup vWii (%u Title%s)"), this->vWiiTitlesCount,
                        (this->vWiiTitlesCount > 1) ? "s" : "");
        DrawUtils::setFontColor(COLOR_TEXT);
        consolePrintPos(M_OFF, 2 + cursorPos, "\u2192");
        consolePrintPosAligned(17, 4, 2, LanguageUtils::gettext("\ue000: Backup  \ue001: Back"));
    }
}

ApplicationState::eSubState BatchBackupState::update(Input *input) {
    if (this->state == STATE_BATCH_BACKUP_MENU) {
        if (input->get(TRIGGER, PAD_BUTTON_UP))
            if (--cursorPos == -1)
                ++cursorPos;
        if (input->get(TRIGGER, PAD_BUTTON_DOWN))
            if (++cursorPos == ENTRYCOUNT)
                --cursorPos;
        if (input->get(TRIGGER, PAD_BUTTON_B))
            return SUBSTATE_RETURN;
        if (input->get(TRIGGER, PAD_BUTTON_A)) {
            const std::string batchDatetime = getNowDateForFolder();
            switch (cursorPos) {
                case 0:
                    backupAllSave(this->wiiutitles, this->wiiuTitlesCount, batchDatetime);
                    backupAllSave(this->wiititles, this->vWiiTitlesCount, batchDatetime);
                    writeBackupAllMetadata(batchDatetime,"WiiU and vWii titles");
                    BackupSetList::setIsInitializationRequired(true);
                    DrawUtils::setRedraw(true);
                    break;
                case 1: 
                    this->state = STATE_DO_SUBSTATE;
                    this->subState = std::make_unique<BatchBackupTitleSelectState>(this->wiiutitles, this->wiiuTitlesCount, true);
                    break;
                case 2:
                    this->state = STATE_DO_SUBSTATE;
                    this->subState = std::make_unique<BatchBackupTitleSelectState>(this->wiititles, this->vWiiTitlesCount, false);
                    break;
                default:
                    return SUBSTATE_RUNNING;
            }
        }
    } else if (this->state == STATE_DO_SUBSTATE) {
        auto retSubState = this->subState->update(input);
        if (retSubState == SUBSTATE_RUNNING) {
            // keep running.
            return SUBSTATE_RUNNING;
        } else if (retSubState == SUBSTATE_RETURN) {
            this->subState.reset();
            this->state = STATE_BATCH_BACKUP_MENU;
        }
    }
    return SUBSTATE_RUNNING;
}