#pragma once

#include <ApplicationState.h>
#include <utils/InputUtils.h>

#include <memory>

class MainMenuState : ApplicationState {
public:
    MainMenuState(Title *wiiutitles, Title *wiititles, int wiiuTitlesCount, int vWiiTitlesCount) :
        wiiutitles(wiiutitles),
        wiititles(wiititles),
        wiiuTitlesCount(wiiuTitlesCount),
        vWiiTitlesCount(vWiiTitlesCount) {}
    enum eState {
        STATE_MAIN_MENU,
        STATE_DO_SUBSTATE,
    };

    void render() override;
    ApplicationState::eSubState update(Input *input) override;
private:
    std::unique_ptr<ApplicationState> subState{};
    eState state = STATE_MAIN_MENU;

    Title *wiiutitles;
    Title *wiititles;

    int wiiuTitlesCount;
    int vWiiTitlesCount;
};