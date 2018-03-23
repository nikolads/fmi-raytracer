#pragma once

#include "deps.h"

#include <tuple>

namespace app {

struct Queues {
    size_t computeQueueFamily;
    size_t presentQueueFamily;
    vk::Queue compute;
    vk::Queue present;
};

vk::PhysicalDevice choosePhysicalDevice(vk::Instance instance, vk::SurfaceKHR surface);
std::tuple<vk::UniqueDevice, Queues> createDevice(vk::PhysicalDevice physical, vk::SurfaceKHR surface);
vk::UniqueSwapchainKHR createSwapchain(vk::PhysicalDevice physical, vk::Device device, vk::SurfaceKHR surface,
    const Queues& queues, uint32_t windowWidth, uint32_t windowHeight);

}
