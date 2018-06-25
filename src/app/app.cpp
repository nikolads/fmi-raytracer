#include "app.h"
#include "device.h"
#include "instance.h"
#include "shader.h"
#include "window.h"

namespace app {

App::App(UniqueGlfwWindow&& window, vk::UniqueInstance&& instance, vk::UniqueSurfaceKHR&& surface,
    vk::UniqueDevice&& device, Queues queues, vk::UniqueSwapchainKHR&& swapchain,
    vk::UniqueDescriptorSetLayout&& descriptorLayout, vk::UniqueDeviceMemory&& memory,
    vk::UniqueDeviceMemory&& bufferMemory, vk::UniqueImage&& workImage, vk::UniqueImageView&& workImageView,
    vk::UniqueBuffer&& stateBuffer, vk::UniqueDescriptorPool&& descriptorPool,
    vk::UniquePipeline&& pipeline, vk::UniquePipelineLayout&& pipelineLayout,
    vk::UniqueCommandPool&& cmdPool, std::vector<vk::UniqueCommandBuffer>&& cmdBuffers,
    vk::UniqueSemaphore&& imageAvailableSemaphore):
    window(std::move(window)),
    instance(std::move(instance)),
    surface(std::move(surface)),
    device(std::move(device)),
    queues(queues),
    swapchain(std::move(swapchain)),
    descriptorLayout(std::move(descriptorLayout)),
    memory(std::move(memory)),
    bufferMemory(std::move(bufferMemory)),
    workImage(std::move(workImage)),
    workImageView(std::move(workImageView)),
    stateBuffer(std::move(stateBuffer)),
    descriptorPool(std::move(descriptorPool)),
    pipeline(std::move(pipeline)),
    pipelineLayout(std::move(pipelineLayout)),
    cmdPool(std::move(cmdPool)),
    cmdBuffers(std::move(cmdBuffers)),
    imageAvailableSemaphore(std::move(imageAvailableSemaphore))
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
    auto descriptorLayout = createDescriptorSetLayoyt(*device);
    auto [memory, workImage, workImageView] = createImage(*device, physical, extent);
    auto [bufferMemory, stateBuffer] = createBuffer(*device, physical);
    auto [descriptorPool, descriptorSet] = createDescriptorSet(*device, *descriptorLayout, *workImageView, *stateBuffer);
    auto [pipeline, pipelineLayout, shader] = createPipeline(*device, *descriptorLayout);
    auto [cmdPool, cmdBuffers] = createCommands(*device, *swapchain, queues, *pipeline, *pipelineLayout,
        descriptorSet, *workImage, extent);
    auto imageAvailableSemaphore = device->createSemaphoreUnique(vk::SemaphoreCreateInfo(), nullptr);

    return App(std::move(window), std::move(instance), std::move(surface),
        std::move(device), queues, std::move(swapchain), std::move(descriptorLayout),
        std::move(memory), std::move(bufferMemory), std::move(workImage), std::move(workImageView),
        std::move(stateBuffer), std::move(descriptorPool), std::move(pipeline), std::move(pipelineLayout),
        std::move(cmdPool), std::move(cmdBuffers), std::move(imageAvailableSemaphore));
}

void App::mainLoop() {
    bool running = true;

    while (running) {
        glfwPollEvents();

        this->drawFrame();

        running &= !glfwWindowShouldClose(&*this->window);
        running &= !glfwGetKey(&*this->window, GLFW_KEY_ESCAPE);
    }

    this->device->waitIdle();
}

void App::drawFrame() {
    auto imageIndex = this->device->acquireNextImageKHR(*this->swapchain,
        std::numeric_limits<uint64_t>::max(), *this->imageAvailableSemaphore, nullptr).value;

    auto waitStage = vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput);

    auto submitInfo = vk::SubmitInfo(
        1,                                  // waitSemaphoreCount
        &*this->imageAvailableSemaphore,    // pWaitSemaphores
        &waitStage,                         // pWaitDstStageMask
        1,                                  // commandBufferCount
        &*this->cmdBuffers[imageIndex],     // pCommandBuffers
        0,                                  // signalSemaphoreCount
        nullptr                             // pSignalSemaphores
    );

    this->queues.compute.submit(1, &submitInfo, nullptr);

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
