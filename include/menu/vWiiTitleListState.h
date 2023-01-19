#pragma once

#include <ApplicationState.h>
#include <savemng.h>
#include <utils/InputUtils.h>
#include <utils/LanguageUtils.h>

#include <memory>
#include <vector>

class vWiiTitleListState : public ApplicationState {
public:
    vWiiTitleListState(Title *titles, int titlesCount) :
    titles(titles),
    titlesCount(titlesCount) {}
    enum eState {
        STATE_VWII_TITLE_LIST,
        STATE_DO_SUBSTATE,
    };

    void render() override;
    ApplicationState::eSubState update(Input *input) override;
private:
    std::unique_ptr<ApplicationState> subState{};
    eState state = STATE_VWII_TITLE_LIST;

    Title *titles;

    int titlesCount;

    std::vector<const char*> sortNames = {LanguageUtils::gettext("None"), 
                                        LanguageUtils::gettext("Name"),
                                        LanguageUtils::gettext("Storage"),
                                        LanguageUtils::gettext("Storage+Name")};

    int titleSort = 1;
    int scroll = 0;
    bool sortAscending = true;
    int targ = 0;
};