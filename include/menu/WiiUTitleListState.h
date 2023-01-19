#pragma once

#include <ApplicationState.h>
#include <utils/InputUtils.h>

#include <memory>

class WiiUTitleListState : ApplicationState {
public:
    WiiUTitleListState(Title *titles, int titlesCount) :
    titles(titles),
    titlesCount(titlesCount) {
        this->sortn = {LanguageUtils::gettext("None"), LanguageUtils::gettext("Name"), LanguageUtils::gettext("Storage"), LanguageUtils::gettext("Storage+Name")};
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