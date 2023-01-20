#include <cstring>
#include <menu/TitleTaskState.h>
#include <menu/vWiiTitleListState.h>
#include <savemng.h>
#include <utils/InputUtils.h>
#include <utils/LanguageUtils.h>

#include <algorithm>

#include <coreinit/debug.h>

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
            if (sortAscending) {
                std::ranges::sort(titles, last, std::ranges::less{}, proj);
            } else {
                std::ranges::sort(titles, last, std::ranges::greater{}, proj);
            }
            break;
        }
        case 2:
            if (sortAscending) {
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
            if (sortAscending) {
                std::ranges::sort(titles, last, std::ranges::less{}, proj);
            } else {
                std::ranges::sort(titles, last, std::ranges::greater{}, proj);
            }
            break;
        }
        default:
            break;
    }
}

void vWiiTitleListState::render() {
    if (this->state == STATE_DO_SUBSTATE) {
        if (this->subState == nullptr) {
            OSFatal("SubState was null");
        }
        this->subState->render();
        return;
    }
    if (this->state == STATE_VWII_TITLE_LIST) {
        if ((this->titles == nullptr) || (this->titlesCount == 0)) {
            promptError(LanguageUtils::gettext("No vWii titles found."));
            this->noTitles = true;
        }
        consolePrintPos(39, 0, LanguageUtils::gettext("%s Sort: %s \ue084"),
                        (titleSort > 0) ? ((sortAscending == true) ? "\ue083 \u2193" : "\ue083 \u2191") : "", this->sortNames[this->titleSort]);
        for (int i = 0; i < 14; i++) {
            if (i + this->scroll < 0 || i + this->scroll >= this->titlesCount)
                break;
            DrawUtils::setFontColor(static_cast<Color>(0x00FF00FF));
            if (!titles[i + this->scroll].saveInit)
                DrawUtils::setFontColor(static_cast<Color>(0xFFFF00FF));
            if (strcmp(titles[i + this->scroll].shortName, "DONT TOUCH ME") == 0)
                DrawUtils::setFontColor(static_cast<Color>(0xFF0000FF));
            if (strlen(titles[i + this->scroll].shortName) != 0u)
                consolePrintPos(M_OFF, i + 2, "   %s %s%s", titles[i + this->scroll].shortName,
                                titles[i + this->scroll].isTitleDupe ? " [D]" : "",
                                titles[i + this->scroll].saveInit ? "" : LanguageUtils::gettext(" [Not Init]"));
            else
                consolePrintPos(M_OFF, i + 2, "   %08lx%08lx", titles[i + this->scroll].highID,
                                titles[i + this->scroll].lowID);
            DrawUtils::setFontColor(COLOR_TEXT);
            if (titles[i + this->scroll].iconBuf != nullptr) {
                DrawUtils::drawRGB5A3((M_OFF + 2) * 12 - 2, (i + 3) * 24 + 3, 0.25,
                                      titles[i + this->scroll].iconBuf);
            }
        }
        consolePrintPos(-3, 2 + cursorPos, "\u2192");
        consolePrintPosAligned(17, 4, 2, LanguageUtils::gettext("\ue000: Select Game  \ue001: Back"));
    }
}

ApplicationState::eSubState vWiiTitleListState::update(Input *input) {
    if (this->state == STATE_VWII_TITLE_LIST) {
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
            if (!this->titles[this->targ].saveInit) {
                if (!promptConfirm(ST_WARNING, LanguageUtils::gettext("Recommended to run Game at least one time. Continue?")) ||
                    !promptConfirm(ST_WARNING, LanguageUtils::gettext("Are you REALLY sure?"))) {
                    return SUBSTATE_RUNNING;
                }
            }
            DrawUtils::setRedraw(true);
            this->state = STATE_DO_SUBSTATE;
            this->subState = std::make_unique<TitleTaskState>(this->titles[this->targ], this->titles, this->titlesCount);
        }
        if (input->get(TRIGGER, PAD_BUTTON_DOWN)) {
            if (this->titlesCount <= 14)
                cursorPos = (cursorPos + 1) % this->titlesCount;
            else if (cursorPos < 6)
                cursorPos++;
            else if (((cursorPos + this->scroll + 1) % this->titlesCount) != 0)
                scroll++;
            else
                cursorPos = scroll = 0;
        } else if (input->get(TRIGGER, PAD_BUTTON_UP)) {
            if (scroll > 0)
                cursorPos -= (cursorPos > 6) ? 1 : 0 * (scroll--);
            else if (cursorPos > 0)
                cursorPos--;
            else if (this->titlesCount > 14)
                scroll = this->titlesCount - (cursorPos = 6) - 1;
            else
                cursorPos = this->titlesCount - 1;
        }
    } else if (this->state == STATE_DO_SUBSTATE) {
        auto retSubState = this->subState->update(input);
        if (retSubState == SUBSTATE_RUNNING) {
            // keep running.
            return SUBSTATE_RUNNING;
        } else if (retSubState == SUBSTATE_RETURN) {
            this->subState.reset();
            this->state = STATE_VWII_TITLE_LIST;
        }
    }
    return SUBSTATE_RUNNING;
}