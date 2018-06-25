#pragma once

#include "deps.h"
#include "device.h"
#include "util.h"
#include "window.h"

#include <vector>

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
    vk::UniqueDeviceMemory memory;
    vk::UniqueDeviceMemory bufferMemory;
    vk::UniqueImage workImage;
    vk::UniqueImageView workImageView;
    vk::UniqueBuffer stateBuffer;
    vk::UniqueDescriptorPool descriptorPool;
    vk::UniquePipeline pipeline;
    vk::UniquePipelineLayout pipelineLayout;
    vk::UniqueCommandPool cmdPool;
    std::vector<vk::UniqueCommandBuffer> cmdBuffers;
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
        vk::UniqueDescriptorSetLayout&& descriptorLayout, vk::UniqueDeviceMemory&& memory,
        vk::UniqueDeviceMemory&& bufferMemory, vk::UniqueImage&& workImage, vk::UniqueImageView&& workImageView,
        vk::UniqueBuffer&& stateBuffer, vk::UniqueDescriptorPool&& descriptorPool,
        vk::UniquePipeline&& pipeline, vk::UniquePipelineLayout&& pipelineLayout,
        vk::UniqueCommandPool&& cmdPool, std::vector<vk::UniqueCommandBuffer>&& cmdBuffers,
        vk::UniqueSemaphore&& imageAvailableSemaphore);
};

} // namespace app
