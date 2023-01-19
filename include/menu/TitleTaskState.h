#pragma once

#include <ApplicationState.h>
#include <utils/InputUtils.h>

#include <memory>

class TitleTaskState : ApplicationState {
public:
    TitleTaskState(Title title) :
    title(title) {}
    enum eState {
        STATE_TITLE_TASKS,
        STATE_DO_SUBSTATE,
    };

    void render() override;
    ApplicationState::eSubState update(Input *input) override;
private:
    std::unique_ptr<ApplicationState> subState{};
    eState state = STATE_TITLE_TASKS;

    Title title;
    bool isWiiUTitle;

    Task task;
};