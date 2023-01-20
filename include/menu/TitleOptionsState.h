#pragma once

#include <ApplicationState.h>
#include <utils/InputUtils.h>

#include <savemng.h>

#include <memory>

class TitleOptionsState : public ApplicationState {
public:
    TitleOptionsState(Title title, Task task, int *versionList, int8_t sdusers, int8_t allusers, bool common, int8_t allusers_d, Title *titles, int titleCount) :
    title(title),
    task(task),
    versionList(versionList),
    sdusers(sdusers),
    allusers(allusers),
    common(common),
    allusers_d(allusers_d),
    titles(titles),
    titleCount(titleCount) {}

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

    int *versionList;

    int8_t sdusers;
    int8_t allusers;
    bool common;
    int8_t allusers_d;

    Title *titles;
    int titleCount;

    bool isWiiUTitle;
};