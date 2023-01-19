#pragma once

#include <ApplicationState.h>
#include <utils/InputUtils.h>

#include <utils/LanguageUtils.h>

#include <memory>

class WiiUTitleListState : ApplicationState {
public:
    explicit WiiUTitleListState(Title *titles, int titlesCount) :
        titles(titles),
        titlesCount(titlesCount) {
            this->sortNames[0] = LanguageUtils::gettext("None");
            this->sortNames[1] = LanguageUtils::gettext("Name");
            this->sortNames[2] = LanguageUtils::gettext("Storage");
            this->sortNames[3] = LanguageUtils::gettext("Storage+Name");
        }
    enum eState {
        STATE_WIIU_TITLE_LIST,
        STATE_DO_SUBSTATE,
    };

    void render() override;
    ApplicationState::eSubState update(Input *input) override;
private:
    std::unique_ptr<ApplicationState> subState{};
    eState state = STATE_WIIU_TITLE_LIST;

    Title *titles;

    int titlesCount;

    char sortNames[4];

    int titleSort = 1;
    int scroll = 0;
    bool sortAscending = true;
    int targ = 0;
};