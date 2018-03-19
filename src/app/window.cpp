#include "window.h"

namespace app {

std::optional<UniqueGlfwWindow> createWindow(uint32_t width, uint32_t height, const char* title) {
    if (glfwInit() != GLFW_TRUE) {
        return std::optional<UniqueGlfwWindow>();
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    auto window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    return std::optional(UniqueGlfwWindow(window, [](GLFWwindow*){ glfwTerminate(); }));
}

}
