#pragma once

#include "deps.h"
#include "window.h"

namespace app {

class App {
private:
    UniqueGlfwWindow window;
    vk::UniqueInstance instance;
    vk::UniqueSurfaceKHR surface;
    vk::UniqueDevice device;

public:
    static App create();

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    App(App&&) = default;
    App& operator=(App&&) = default;

    void mainLoop();

private:
    App(UniqueGlfwWindow&& window, vk::UniqueInstance&& instance, vk::UniqueSurfaceKHR&& surface,
        vk::UniqueDevice&& device);
};

} // namespace app
