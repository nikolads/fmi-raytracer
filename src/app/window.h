#pragma once

#include "deps.h"
#include "error.h"

#include <memory>
#include <optional>

namespace app {

using UniqueGlfwWindow = std::unique_ptr<GLFWwindow, void(*)(GLFWwindow*)>;

std::optional<UniqueGlfwWindow> createWindow(uint32_t width, uint32_t height, const char* title);

}
