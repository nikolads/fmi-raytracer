#pragma once

#include "deps.h"

#include <memory>

namespace app {

using UniqueGlfwWindow = std::unique_ptr<GLFWwindow, void(*)(GLFWwindow*)>;

UniqueGlfwWindow createWindow(uint32_t width, uint32_t height, const char* title);
vk::UniqueSurfaceKHR createSurface(GLFWwindow* window, vk::Instance instance);

}
