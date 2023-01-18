#include <menu/ConfigMenuState.h>
#include <cstring>
#include <utils/InputUtils.h>
#include <utils/LanguageUtils.h>
#include <savemng.h>
#include <utils/StateUtils.h>

static int cursorPos = 0;
static bool redraw = true;

static std::string language = "";

static void drawConfigMenuFrame() {
    DrawUtils::beginDraw();
    DrawUtils::clear(COLOR_BACKGROUND);
    consolePrintPos(0, 0, "SaveMii v%u.%u.%u", VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO);
    DrawUtils::drawRectFilled(48, 49, 526, 51, COLOR_WHITE);
    consolePrintPos(M_OFF, 2, LanguageUtils::gettext("   Language: %s"), language.c_str());
    consolePrintPos(M_OFF, 2 + cursorPos, "\u2192");
    DrawUtils::drawRectFilled(48, 406, 526, 408, COLOR_WHITE);
    consolePrintPos(0, 17, LanguageUtils::gettext("Press \ue044 to exit."));
    DrawUtils::endDraw();
}

void configMenu() {
    Input input;
    language = LanguageUtils::getLoadedLanguage();
    while (State::AppRunning()) {
        input.read();
        if (input.get(TRIGGER, PAD_BUTTON_ANY))
            redraw = true;
        if (input.get(TRIGGER, PAD_BUTTON_B))
            break;
        if (input.get(TRIGGER, PAD_BUTTON_RIGHT)) {
            if (language == LanguageUtils::gettext("Japanese"))
                LanguageUtils::loadLanguage(Swkbd_LanguageType__Italian);
            else if (language == LanguageUtils::gettext("Italian"))
                LanguageUtils::loadLanguage(Swkbd_LanguageType__Spanish);
            else if (language == LanguageUtils::gettext("Spanish"))
                LanguageUtils::loadLanguage(Swkbd_LanguageType__Chinese1);
            else if (language == LanguageUtils::gettext("Traditional Chinese"))
                LanguageUtils::loadLanguage(Swkbd_LanguageType__Korean);
            else if (language == LanguageUtils::gettext("Korean"))
                LanguageUtils::loadLanguage(Swkbd_LanguageType__Russian);
            else if (language == LanguageUtils::gettext("Russian"))
                LanguageUtils::loadLanguage(Swkbd_LanguageType__Chinese2);
            else if (language == LanguageUtils::gettext("Simplified Chinese"))
                LanguageUtils::loadLanguage(Swkbd_LanguageType__English);
            else if (language == LanguageUtils::gettext("English"))
                LanguageUtils::loadLanguage(Swkbd_LanguageType__Japanese);
        }
        if (input.get(TRIGGER, PAD_BUTTON_LEFT)) {
            if (language == LanguageUtils::gettext("Japanese"))
                LanguageUtils::loadLanguage(Swkbd_LanguageType__English);
            else if (language == LanguageUtils::gettext("English"))
                LanguageUtils::loadLanguage(Swkbd_LanguageType__Chinese2);
            else if (language == LanguageUtils::gettext("Simplified Chinese"))
                LanguageUtils::loadLanguage(Swkbd_LanguageType__Russian);
            else if (language == LanguageUtils::gettext("Russian"))
                LanguageUtils::loadLanguage(Swkbd_LanguageType__Korean);
            else if (language == LanguageUtils::gettext("Korean"))
                LanguageUtils::loadLanguage(Swkbd_LanguageType__Chinese1);
            else if (language == LanguageUtils::gettext("Traditional Chinese"))
                LanguageUtils::loadLanguage(Swkbd_LanguageType__Spanish);
            else if (language == LanguageUtils::gettext("Spanish"))
                LanguageUtils::loadLanguage(Swkbd_LanguageType__Italian);
            else if (language == LanguageUtils::gettext("Italian"))
                LanguageUtils::loadLanguage(Swkbd_LanguageType__Japanese);
        }
        if (redraw) {
            language = LanguageUtils::getLoadedLanguage();
            drawConfigMenuFrame();
            redraw = false;
        }
    }
    return;
}