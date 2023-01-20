#include <menu/ConfigMenuState.h>
#include <savemng.h>
#include <utils/InputUtils.h>
#include <utils/LanguageUtils.h>

static int cursorPos = 0;

static std::string language;

void ConfigMenuState::render() {
    language = LanguageUtils::getLoadedLanguage();
    consolePrintPos(M_OFF, 2, LanguageUtils::gettext("   Language: %s"), language.c_str());
    consolePrintPos(M_OFF, 2 + cursorPos, "\u2192");
}

ApplicationState::eSubState ConfigMenuState::update(Input *input) {
    if (input->get(TRIGGER, PAD_BUTTON_B))
        return SUBSTATE_RETURN;
    if (input->get(TRIGGER, PAD_BUTTON_RIGHT)) {
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
    if (input->get(TRIGGER, PAD_BUTTON_LEFT)) {
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
    return SUBSTATE_RUNNING;
}