#pragma once

#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_ASSERT [](bool _){}
#include "vulkan/vulkan.hpp"
#include "GLFW/glfw3.h"

#include "error.h"

#include <variant>

namespace app {

class App {
private:
    vk::UniqueInstance instance;

public:
    static std::variant<App, Error> create();

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    App(App&&) = default;
    App& operator=(App&&) = default;

private:
    App(vk::UniqueInstance&& instance);
};

} // namespace app
