#pragma once

#include <ApplicationState.h>
#include <utils/InputUtils.h>

#include <memory>

class TitleOptionsState : ApplicationState {
public:
    TitleOptionsState(Title title, Task task) :
    title(title),
    task(task) {}

    enum eState {
        STATE_TITLE_OPTIONS,
        STATE_DO_SUBSTATE,
    };

    void render() override;
    ApplicationState::eSubState update(Input *input) override;
private:
    std::unique_ptr<ApplicationState> subState{};
    eState state = STATE_TITLE_OPTIONS;

    Title title;
    Task task;

    bool isWiiUTitle;
};