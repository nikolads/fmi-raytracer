#include "app.h"
#include "device.h"
#include "instance.h"
#include "window.h"

namespace app {

App::App(UniqueGlfwWindow&& window, vk::UniqueInstance&& instance, vk::UniqueSurfaceKHR&& surface,
    vk::UniqueDevice&& device, vk::UniqueSwapchainKHR&& swapchain):
    window(std::move(window)),
    instance(std::move(instance)),
    surface(std::move(surface)),
    device(std::move(device)),
    swapchain(std::move(swapchain))
{
}

App App::create() {
    const uint32_t width = 800;
    const uint32_t height = 600;

    auto window = createWindow(width, height, "GPU raytracer");
    auto instance = createInstance();
    auto surface = createSurface(&*window, *instance);
    auto physical = choosePhysicalDevice(*instance, *surface);
    auto [device, queues] = createDevice(physical, *surface);
    auto swapchain = createSwapchain(physical, *device, *surface, queues, width, height);

    return App(std::move(window), std::move(instance), std::move(surface),
        std::move(device), std::move(swapchain));
}

void App::mainLoop() {
    bool running = true;

    while (running) {
        glfwPollEvents();

        running &= !glfwWindowShouldClose(&*this->window);
        running &= !glfwGetKey(&*this->window, GLFW_KEY_ESCAPE);
    }
}

} // namespace app
