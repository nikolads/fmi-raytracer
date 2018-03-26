#pragma once

#include "deps.h"
#include "device.h"
#include "window.h"

namespace app {

class App {
private:
    UniqueGlfwWindow window;
    vk::UniqueInstance instance;
    vk::UniqueSurfaceKHR surface;
    vk::UniqueDevice device;
    Queues queues;
    vk::UniqueSwapchainKHR swapchain;
    vk::UniqueDescriptorSetLayout descriptorLayout;
    vk::UniqueBuffer buffer;
    vk::UniqueDeviceMemory memory;
    vk::UniqueDescriptorPool descriptorPool;
    vk::UniqueDescriptorSet descriptorSet;
    vk::UniquePipeline pipeline;
    vk::UniquePipelineLayout pipelineLayout;
    vk::UniqueCommandPool cmdPool;
    vk::UniqueCommandBuffer cmdBuffer;
    vk::UniqueSemaphore imageAvailableSemaphore;

public:
    static App create();

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    App(App&&) = default;
    App& operator=(App&&) = default;

    void mainLoop();
    void drawFrame();

private:
    App(UniqueGlfwWindow&& window, vk::UniqueInstance&& instance, vk::UniqueSurfaceKHR&& surface,
        vk::UniqueDevice&& device, Queues queues, vk::UniqueSwapchainKHR&& swapchain,
        vk::UniqueDescriptorSetLayout&& descriptorLayout, vk::UniqueBuffer&& buffer,
        vk::UniqueDeviceMemory&& memory, vk::UniqueDescriptorPool&& descriptorPool,
        vk::UniqueDescriptorSet&& descriptorSet, vk::UniquePipeline&& pipeline,
        vk::UniquePipelineLayout&& pipelineLayout,vk::UniqueCommandPool&& cmdPool,
        vk::UniqueCommandBuffer&& cmdBuffer, vk::UniqueSemaphore&& imageAvailableSemaphore);
};

} // namespace app
