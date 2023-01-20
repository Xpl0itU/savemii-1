#pragma once

class State {
public:
    static void init();
    static bool AppRunning();
    static void shutdown();

private:
    static bool aroma;
};