#pragma once

#include <ApplicationState.h>
#include <memory>
#include <savemng.h>
#include <utils/InputUtils.h>
#include <utils/LanguageUtils.h>
#include <vector>

class WiiUTitleListState : public ApplicationState {
public:
    explicit WiiUTitleListState(Title *titles, int titlesCount) : titles(titles),
                                                                  titlesCount(titlesCount) {}
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

    std::vector<const char *> sortNames = {LanguageUtils::gettext("None"),
                                           LanguageUtils::gettext("Name"),
                                           LanguageUtils::gettext("Storage"),
                                           LanguageUtils::gettext("Storage+Name")};

    int titleSort = 1;
    static int scroll;
    static int cursorPos;
    bool sortAscending = true;
    int targ = 0;

    bool noTitles = false;
};