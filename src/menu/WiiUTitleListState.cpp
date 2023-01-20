#include <cstring>
#include <menu/TitleTaskState.h>
#include <menu/WiiUTitleListState.h>
#include <savemng.h>
#include <utils/InputUtils.h>
#include <utils/LanguageUtils.h>
#include <utils/StringUtils.h>

#include <algorithm>

#include <coreinit/time.h>

#define MAX_TITLE_SHOW 14
static int cursorPos = 0;

template<class It>
static void sortTitle(It titles, It last, int tsort = 1, bool sortAscending = true) {
    switch (tsort) {
        case 0:
            std::ranges::sort(titles, last, std::ranges::less{}, &Title::listID);
            break;
        case 1: {
            const auto proj = [](const Title &title) {
                return std::string_view(title.shortName);
            };
            if (sortAscending == true) {
                std::ranges::sort(titles, last, std::ranges::less{}, proj);
            } else {
                std::ranges::sort(titles, last, std::ranges::greater{}, proj);
            }
            break;
        }
        case 2:
            if (sortAscending == true) {
                std::ranges::sort(titles, last, std::ranges::less{}, &Title::isTitleOnUSB);
            } else {
                std::ranges::sort(titles, last, std::ranges::greater{}, &Title::isTitleOnUSB);
            }
            break;
        case 3: {
            const auto proj = [](const Title &title) {
                return std::make_tuple(title.isTitleOnUSB,
                                       std::string_view(title.shortName));
            };
            if (sortAscending == true) {
                std::ranges::sort(titles, last, std::ranges::less{}, proj);
            } else {
                std::ranges::sort(titles, last, std::ranges::greater{}, proj);
            }
            break;
        }
    }
}

void WiiUTitleListState::render() {
    if ((this->titles == nullptr) || (this->titlesCount == 0)) {
        promptError(LanguageUtils::gettext("No Wii U titles found."));
        this->noTitles = true;
    }
    consolePrintPos(39, 0, LanguageUtils::gettext("%s Sort: %s \ue084"),
                    (this->titleSort > 0) ? ((this->sortAscending == true) ? "\ue083 \u2193" : "\ue083 \u2191") : "", this->sortNames[this->titleSort]);
    for (int i = 0; i < MAX_TITLE_SHOW; i++) {
        if (i + this->scroll < 0 || i + this->scroll >= this->titlesCount)
            break;
        DrawUtils::setFontColor(static_cast<Color>(0x00FF00FF));
        if (!this->titles[i + this->scroll].saveInit)
            DrawUtils::setFontColor(static_cast<Color>(0xFFFF00FF));
        if (strcmp(this->titles[i + this->scroll].shortName, "DONT TOUCH ME") == 0)
            DrawUtils::setFontColor(static_cast<Color>(0xFF0000FF));
        if (strlen(this->titles[i + this->scroll].shortName) != 0u)
            consolePrintPos(M_OFF, i + 2, "   %s %s%s%s", this->titles[i + this->scroll].shortName,
                            this->titles[i + this->scroll].isTitleOnUSB ? "(USB)" : "(NAND)",
                            this->titles[i + this->scroll].isTitleDupe ? " [D]" : "",
                            this->titles[i + this->scroll].saveInit ? "" : LanguageUtils::gettext(" [Not Init]"));
        else
            consolePrintPos(M_OFF, i + 2, "   %08lx%08lx", this->titles[i + this->scroll].highID,
                            this->titles[i + this->scroll].lowID);
        DrawUtils::setFontColor(COLOR_TEXT);
        if (this->titles[i + this->scroll].iconBuf != nullptr) {
            DrawUtils::drawTGA((M_OFF + 4) * 12 - 2, (i + 3) * 24, 0.18, this->titles[i + this->scroll].iconBuf);
        }
    }
    consolePrintPos(-1, 2 + cursorPos, "\u2192");
    consolePrintPosAligned(17, 4, 2, LanguageUtils::gettext("\ue000: Select Game  \ue001: Back"));
}

ApplicationState::eSubState WiiUTitleListState::update(Input *input) {
    if (input->get(TRIGGER, PAD_BUTTON_B) || noTitles)
        return SUBSTATE_RETURN;
    if (input->get(TRIGGER, PAD_BUTTON_R)) {
        this->titleSort = (this->titleSort + 1) % 4;
        sortTitle(this->titles, this->titles + this->titlesCount, this->titleSort, this->sortAscending);
    }
    if (input->get(TRIGGER, PAD_BUTTON_L)) {
        if (this->titleSort > 0) {
            this->sortAscending = !this->sortAscending;
            sortTitle(this->titles, this->titles + this->titlesCount, this->titleSort, this->sortAscending);
        }
    }
    if (input->get(TRIGGER, PAD_BUTTON_A)) {
        this->targ = cursorPos + this->scroll;
        if (this->titles[this->targ].highID == 0 || this->titles[this->targ].lowID == 0)
            return SUBSTATE_RUNNING;
        if (strcmp(this->titles[this->targ].shortName, "DONT TOUCH ME") == 0) {
            if (!promptConfirm(ST_ERROR, LanguageUtils::gettext("CBHC save. Could be dangerous to modify. Continue?")) ||
                !promptConfirm(ST_WARNING, LanguageUtils::gettext("Are you REALLY sure?"))) {
                return SUBSTATE_RUNNING;
            }
        }
        std::string path = StringUtils::stringFormat("%s/usr/title/000%x/%x/code/fw.img",
                                                     (this->titles[this->targ].isTitleOnUSB) ? getUSB().c_str() : "storage_mlc01:", this->titles[this->targ].highID,
                                                     this->titles[this->targ].lowID);
        if (checkEntry(path.c_str()) != 0)
            if (!promptConfirm(ST_ERROR, LanguageUtils::gettext("vWii saves are in the vWii section. Continue?")))
                return SUBSTATE_RUNNING;
        if (!this->titles[this->targ].saveInit) {
            if (!promptConfirm(ST_WARNING, LanguageUtils::gettext("Recommended to run Game at least one time. Continue?")) ||
                !promptConfirm(ST_WARNING, LanguageUtils::gettext("Are you REALLY sure?"))) {
                return SUBSTATE_RUNNING;
            }
        }
        this->state = STATE_DO_SUBSTATE;
        this->subState = std::make_unique<TitleTaskState>(this->titles[this->targ], this->titles, this->titlesCount);
    }
    return SUBSTATE_RUNNING;
}