#pragma once

#include "deps.h"
#include "error.h"
#include "window.h"

#include <variant>

namespace app {

class App {
private:
    UniqueGlfwWindow window;
    vk::UniqueInstance instance;

public:
    static std::variant<App, Error> create();

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    App(App&&) = default;
    App& operator=(App&&) = default;

private:
    App(UniqueGlfwWindow&& window, vk::UniqueInstance&& instance);
};

} // namespace app
