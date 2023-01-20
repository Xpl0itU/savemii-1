#include <cstring>
#include <date.h>
#include <menu/TitleOptionsState.h>
#include <savemng.h>
#include <utils/InputUtils.h>
#include <utils/LanguageUtils.h>

#include <coreinit/time.h>

static int cursorPos = 0;
static int entrycount;
static uint8_t slot = 0;

void TitleOptionsState::render() {
    this->isWiiUTitle = this->title.highID != (0x00050000 | 0x00050002);
    entrycount = 3;
    consolePrintPos(M_OFF, 2, "[%08X-%08X] %s", this->title.highID, this->title.lowID,
                    this->title.shortName);

    if (this->task == copytoOtherDevice) {
        consolePrintPos(M_OFF, 4, LanguageUtils::gettext("Destination:"));
        consolePrintPos(M_OFF, 5, "    (%s)", this->title.isTitleOnUSB ? "NAND" : "USB");
    } else if (this->task > 2) {
        entrycount = 2;
        consolePrintPos(M_OFF, 4, LanguageUtils::gettext("Select %s:"), LanguageUtils::gettext("version"));
        consolePrintPos(M_OFF, 5, "   < v%u >", this->versionList != nullptr ? this->versionList[slot] : 0);
    } else if (this->task == wipe) {
        consolePrintPos(M_OFF, 4, LanguageUtils::gettext("Delete from:"));
        consolePrintPos(M_OFF, 5, "    (%s)", this->title.isTitleOnUSB ? "USB" : "NAND");
    } else {
        consolePrintPos(M_OFF, 4, LanguageUtils::gettext("Select %s:"), LanguageUtils::gettext("slot"));

        if (((this->title.highID & 0xFFFFFFF0) == 0x00010000) && (slot == 255))
            consolePrintPos(M_OFF, 5, "   < SaveGame Manager GX > (%s)",
                            isSlotEmpty(this->title.highID, this->title.lowID, slot) ? LanguageUtils::gettext("Empty")
                                                                                     : LanguageUtils::gettext("Used"));
        else
            consolePrintPos(M_OFF, 5, "   < %03u > (%s)", slot,
                            isSlotEmpty(this->title.highID, this->title.lowID, slot) ? LanguageUtils::gettext("Empty")
                                                                                     : LanguageUtils::gettext("Used"));
    }

    if (this->isWiiUTitle) {
        if (task == restore) {
            if (!isSlotEmpty(this->title.highID, this->title.lowID, slot)) {
                entrycount++;
                consolePrintPos(M_OFF, 7, LanguageUtils::gettext("Select SD user to copy from:"));
                if (sdusers == -1)
                    consolePrintPos(M_OFF, 8, "   < %s >", LanguageUtils::gettext("all users"));
                else
                    consolePrintPos(M_OFF, 8, "   < %s > (%s)", sdacc[sdusers].persistentID,
                                    hasAccountSave(&this->title, true, false, sdacc[sdusers].pID,
                                                   slot, 0)
                                            ? LanguageUtils::gettext("Has Save")
                                            : LanguageUtils::gettext("Empty"));
            }
        }

        if (task == wipe) {
            consolePrintPos(M_OFF, 7, LanguageUtils::gettext("Select Wii U user to delete from:"));
            if (this->allusers == -1)
                consolePrintPos(M_OFF, 8, "   < %s >", LanguageUtils::gettext("all users"));
            else
                consolePrintPos(M_OFF, 8, "   < %s (%s) > (%s)", wiiuacc[this->allusers].miiName,
                                wiiuacc[this->allusers].persistentID,
                                hasAccountSave(&this->title, false, false, wiiuacc[this->allusers].pID,
                                               slot, 0)
                                        ? LanguageUtils::gettext("Has Save")
                                        : LanguageUtils::gettext("Empty"));
        }

        if ((task == backup) || (task == restore) || (task == copytoOtherDevice)) {
            if ((task == restore) && isSlotEmpty(this->title.highID, this->title.lowID, slot))
                entrycount--;
            else {
                consolePrintPos(M_OFF, (task == restore) ? 10 : 7, LanguageUtils::gettext("Select Wii U user%s:"),
                                (task == copytoOtherDevice) ? LanguageUtils::gettext(" to copy from") : ((task == restore) ? LanguageUtils::gettext(" to copy to") : ""));
                if (this->allusers == -1)
                    consolePrintPos(M_OFF, (task == restore) ? 11 : 8, "   < %s >", LanguageUtils::gettext("all users"));
                else
                    consolePrintPos(M_OFF, (task == restore) ? 11 : 8, "   < %s (%s) > (%s)",
                                    wiiuacc[allusers].miiName, wiiuacc[allusers].persistentID,
                                    hasAccountSave(&this->title,
                                                   (!((task == backup) || (task == restore) || (task == copytoOtherDevice))),
                                                   (!((task < 3) || (task == copytoOtherDevice))),
                                                   wiiuacc[this->allusers].pID, slot,
                                                   this->versionList != nullptr ? this->versionList[slot] : 0)
                                            ? LanguageUtils::gettext("Has Save")
                                            : LanguageUtils::gettext("Empty"));
            }
        }
        if ((task == backup) || (task == restore))
            if (!isSlotEmpty(this->title.highID, this->title.lowID, slot)) {
                Date *dateObj = new Date(this->title.highID, this->title.lowID, slot);
                consolePrintPos(M_OFF, 15, LanguageUtils::gettext("Date: %s"),
                                dateObj->get().c_str());
                delete dateObj;
            }

        if (task == copytoOtherDevice) {
            entrycount++;
            consolePrintPos(M_OFF, 10, LanguageUtils::gettext("Select Wii U user%s:"), (task == copytoOtherDevice) ? LanguageUtils::gettext(" to copy to") : "");
            if (allusers_d == -1)
                consolePrintPos(M_OFF, 11, "   < %s >", LanguageUtils::gettext("all users"));
            else
                consolePrintPos(M_OFF, 11, "   < %s (%s) > (%s)", wiiuacc[allusers_d].miiName,
                                wiiuacc[allusers_d].persistentID,
                                hasAccountSave(&titles[this->title.dupeID], false, false,
                                               wiiuacc[allusers_d].pID, 0, 0)
                                        ? LanguageUtils::gettext("Has Save")
                                        : LanguageUtils::gettext("Empty"));
        }

        if ((task != importLoadiine) && (task != exportLoadiine)) {
            if (this->allusers > -1) {
                if (hasCommonSave(&this->title,
                                  (!((task == backup) || (task == wipe) || (task == copytoOtherDevice))),
                                  (!((task < 3) || (task == copytoOtherDevice))), slot,
                                  this->versionList != nullptr ? this->versionList[slot] : 0)) {
                    consolePrintPos(M_OFF, (task == restore) || (task == copytoOtherDevice) ? 13 : 10,
                                    LanguageUtils::gettext("Include 'common' save?"));
                    consolePrintPos(M_OFF, (task == restore) || (task == copytoOtherDevice) ? 14 : 11, "   < %s >",
                                    common ? LanguageUtils::gettext("yes") : LanguageUtils::gettext("no "));
                } else {
                    common = false;
                    consolePrintPos(M_OFF, (task == restore) || (task == copytoOtherDevice) ? 13 : 10,
                                    LanguageUtils::gettext("No 'common' save found."));
                    entrycount--;
                }
            } else {
                common = false;
                entrycount--;
            }
        } else {
            if (hasCommonSave(&this->title, true, true, slot, this->versionList != nullptr ? this->versionList[slot] : 0)) {
                consolePrintPos(M_OFF, 7, LanguageUtils::gettext("Include 'common' save?"));
                consolePrintPos(M_OFF, 8, "   < %s >", common ? LanguageUtils::gettext("yes") : LanguageUtils::gettext("no "));
            } else {
                common = false;
                consolePrintPos(M_OFF, 7, LanguageUtils::gettext("No 'common' save found."));
                entrycount--;
            }
        }

        consolePrintPos(M_OFF, 5 + cursorPos * 3, "\u2192");
        if (this->title.iconBuf != nullptr)
            DrawUtils::drawTGA(660, 100, 1, this->title.iconBuf);
    } else {
        entrycount = 1;
        if (this->title.iconBuf != nullptr)
            DrawUtils::drawRGB5A3(650, 100, 1, this->title.iconBuf);
        if (!isSlotEmpty(this->title.highID, this->title.lowID, slot)) {
            Date *dateObj = new Date(this->title.highID, this->title.lowID, slot);
            consolePrintPos(M_OFF, 15, LanguageUtils::gettext("Date: %s"),
                            dateObj->get().c_str());
            delete dateObj;
        }
    }

    switch (task) {
        case backup:
            consolePrintPosAligned(17, 4, 2, LanguageUtils::gettext("\ue000: Backup  \ue001: Back"));
            break;
        case restore:
            consolePrintPosAligned(17, 4, 2, LanguageUtils::gettext("\ue000: Restore  \ue001: Back"));
            break;
        case wipe:
            consolePrintPosAligned(17, 4, 2, LanguageUtils::gettext("\ue000: Wipe  \ue001: Back"));
            break;
        case importLoadiine:
            consolePrintPosAligned(17, 4, 2, LanguageUtils::gettext("\ue000: Import  \ue001: Back"));
            break;
        case exportLoadiine:
            consolePrintPosAligned(17, 4, 2, LanguageUtils::gettext("\ue000: Export  \ue001: Back"));
            break;
        case copytoOtherDevice:
            consolePrintPosAligned(17, 4, 2, LanguageUtils::gettext("\ue000: Copy  \ue001: Back"));
            break;
    }
}

ApplicationState::eSubState TitleOptionsState::update(Input *input) {
    if (input->get(TRIGGER, PAD_BUTTON_B))
        return SUBSTATE_RETURN;
    if (input->get(TRIGGER, PAD_BUTTON_LEFT)) {
        if (this->task == copytoOtherDevice) {
            switch (cursorPos) {
                case 0:
                    break;
                case 1:
                    this->allusers = ((this->allusers == -1) ? -1 : (this->allusers - 1));
                    allusers_d = this->allusers;
                    break;
                case 2:
                    allusers_d = (((this->allusers == -1) || (allusers_d == -1)) ? -1 : (allusers_d - 1));
                    allusers_d = ((this->allusers > -1) && (allusers_d == -1)) ? 0 : allusers_d;
                    break;
                case 3:
                    common ^= 1;
                    break;
            }
        } else if (this->task == restore) {
            switch (cursorPos) {
                case 0:
                    slot--;
                    getAccountsSD(&this->title, slot);
                    break;
                case 1:
                    sdusers = ((sdusers == -1) ? -1 : (sdusers - 1));
                    this->allusers = ((sdusers == -1) ? -1 : this->allusers);
                    break;
                case 2:
                    allusers = (((allusers == -1) || (sdusers == -1)) ? -1 : (allusers - 1));
                    allusers = ((sdusers > -1) && (allusers == -1)) ? 0 : allusers;
                    break;
                case 3:
                    common ^= 1;
                    break;
            }
        } else if (this->task == wipe) {
            switch (cursorPos) {
                case 0:
                    break;
                case 1:
                    allusers = ((allusers == -1) ? -1 : (allusers - 1));
                    break;
                case 2:
                    common ^= 1;
                    break;
            }
        } else if ((this->task == importLoadiine) || (this->task == exportLoadiine)) {
            switch (cursorPos) {
                case 0:
                    slot--;
                    break;
                case 1:
                    common ^= 1;
                    break;
            }
        } else {
            switch (cursorPos) {
                case 0:
                    slot--;
                    break;
                case 1:
                    allusers = ((allusers == -1) ? -1 : (allusers - 1));
                    break;
                case 2:
                    common ^= 1;
                    break;
            }
        }
    }
    if (input->get(TRIGGER, PAD_BUTTON_RIGHT)) {
        if (this->task == copytoOtherDevice) {
            switch (cursorPos) {
                case 0:
                    break;
                case 1:
                    allusers = ((allusers == (wiiuaccn - 1)) ? (wiiuaccn - 1) : (allusers + 1));
                    allusers_d = allusers;
                    break;
                case 2:
                    allusers_d = ((allusers_d == (wiiuaccn - 1)) ? (wiiuaccn - 1) : (allusers_d + 1));
                    allusers_d = (allusers == -1) ? -1 : allusers_d;
                    break;
                case 3:
                    common ^= 1;
                    break;
            }
        } else if (this->task == restore) {
            switch (cursorPos) {
                case 0:
                    slot++;
                    getAccountsSD(&this->title, slot);
                    break;
                case 1:
                    sdusers = ((sdusers == (sdaccn - 1)) ? (sdaccn - 1) : (sdusers + 1));
                    allusers = ((sdusers > -1) && (allusers == -1)) ? 0 : allusers;
                    break;
                case 2:
                    allusers = ((allusers == (wiiuaccn - 1)) ? (wiiuaccn - 1) : (allusers + 1));
                    allusers = (sdusers == -1) ? -1 : allusers;
                    break;
                case 3:
                    common ^= 1;
                    break;
            }
        } else if (this->task == wipe) {
            switch (cursorPos) {
                case 0:
                    break;
                case 1:
                    allusers = ((allusers == (wiiuaccn - 1)) ? (wiiuaccn - 1) : (allusers + 1));
                    break;
                case 2:
                    common ^= 1;
                    break;
            }
        } else if ((this->task == importLoadiine) || (this->task == exportLoadiine)) {
            switch (cursorPos) {
                case 0:
                    slot++;
                    break;
                case 1:
                    common ^= 1;
                    break;
            }
        } else {
            switch (cursorPos) {
                case 0:
                    slot++;
                    break;
                case 1:
                    allusers = ((allusers == (wiiuaccn - 1)) ? (wiiuaccn - 1) : (allusers + 1));
                    break;
                case 2:
                    common ^= 1;
                    break;
            }
        }
    }
    if (input->get(TRIGGER, PAD_BUTTON_A)) {
        switch (this->task) {
            case backup:
                backupSavedata(&this->title, slot, allusers, common);
                break;
            case restore:
                restoreSavedata(&this->title, slot, sdusers, allusers, common);
                break;
            case wipe:
                wipeSavedata(&this->title, allusers, common);
                break;
            case copytoOtherDevice:
                for (int i = 0; i < this->titleCount; i++) {
                    if (titles[i].listID == this->title.dupeID) {
                        copySavedata(&this->title, &titles[i], allusers, allusers_d, common);
                        break;
                    }
                }
                break;
        }
    }
    return SUBSTATE_RUNNING;
}