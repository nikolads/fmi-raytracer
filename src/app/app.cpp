#include "app.h"
#include "device.h"
#include "instance.h"
#include "window.h"

namespace app {

App::App(UniqueGlfwWindow&& window, vk::UniqueInstance&& instance, vk::UniqueSurfaceKHR&& surface,
    vk::UniqueDevice&& device):
    window(std::move(window)),
    instance(std::move(instance)),
    surface(std::move(surface)),
    device(std::move(device))
{
}

App App::create() {
    auto window = createWindow(800, 600, "GPU raytracer");
    auto instance = createInstance();
    auto surface = createSurface(&*window, *instance);
    auto physical = choosePhysicalDevice(*instance, *surface);
    auto [device, queues] = createDevice(physical, *surface);

    return App(std::move(window), std::move(instance), std::move(surface),
        std::move(device));
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
