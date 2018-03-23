#include "window.h"

namespace app {

UniqueGlfwWindow createWindow(uint32_t width, uint32_t height, const char* title) {
    if (glfwInit() != GLFW_TRUE) {
        throw std::runtime_error("glfwInit failed");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    auto window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (window == nullptr) {
        throw std::runtime_error("glfwCreateWindow failed");
    }

    return UniqueGlfwWindow(window, [](GLFWwindow*){ glfwTerminate(); });
}

vk::UniqueSurfaceKHR createSurface(GLFWwindow* window, vk::Instance instance) {
    vk::SurfaceKHR surface;
    auto result = static_cast<vk::Result>(glfwCreateWindowSurface(
        instance, window, nullptr, reinterpret_cast<VkSurfaceKHR*>(&surface)));

    auto deleter = vk::ObjectDestroy<vk::Instance>(instance, nullptr);
    return vk::createResultValue(result, surface, "glfwCreateWindowSurface", deleter);
}

}
