#pragma once

#include <ApplicationState.h>
#include <utils/InputUtils.h>

#include <savemng.h>

#include <memory>

class TitleTaskState : public ApplicationState {
public:
    TitleTaskState(Title title, Title *titles, int titlesCount) : title(title),
                                                                  titles(titles),
                                                                  titlesCount(titlesCount) {}
    ~TitleTaskState() {
        free(this->versionList);
    }
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
    Title *titles;
    int titlesCount;
    bool isWiiUTitle;

    Task task;
    int *versionList = (int *) malloc(0x100 * sizeof(int));
};