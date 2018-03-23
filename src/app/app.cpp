#include "app.h"
#include "device.h"
#include "instance.h"
#include "window.h"

namespace app {

App::App(UniqueGlfwWindow&& window, vk::UniqueInstance&& instance, vk::UniqueSurfaceKHR&& surface,
    vk::UniqueDevice&& device, Queues queues, vk::UniqueSwapchainKHR&& swapchain):
    window(std::move(window)),
    instance(std::move(instance)),
    surface(std::move(surface)),
    device(std::move(device)),
    queues(queues),
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
    auto [swapchain, format, extent] = createSwapchain(physical, *device, *surface, queues, width, height);
    auto imageViews = createImageViews(*device, *swapchain, format);

    return App(std::move(window), std::move(instance), std::move(surface),
        std::move(device), queues, std::move(swapchain));
}

void App::mainLoop() {
    bool running = true;

    while (running) {
        glfwPollEvents();

        // this->drawFrame();

        running &= !glfwWindowShouldClose(&*this->window);
        running &= !glfwGetKey(&*this->window, GLFW_KEY_ESCAPE);
    }

    this->device->waitIdle();
}

void App::drawFrame() {
    // TODO: add semaphore or fence
    auto imageIndex = this->device->acquireNextImageKHR(*this->swapchain,
        std::numeric_limits<uint64_t>::max(), nullptr, nullptr).value;

    // TODO: submit

    auto presentInfo = vk::PresentInfoKHR(
        0,                      // waitSemaphoreCount
        nullptr,                // pWaitSemaphores
        1,                      // swapchainCount
        &*this->swapchain,      // pSwapchains
        &imageIndex,            // pImageIndices
        nullptr                 // pResults
    );

    this->queues.present.presentKHR(&presentInfo);
}

} // namespace app
