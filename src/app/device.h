#pragma once

#include "deps.h"

#include <tuple>
#include <vector>

namespace app {

struct Queues {
    size_t computeQueueFamily;
    size_t presentQueueFamily;
    vk::Queue compute;
    vk::Queue present;
};

vk::PhysicalDevice choosePhysicalDevice(vk::Instance instance, vk::SurfaceKHR surface);

std::tuple<vk::UniqueDevice, Queues> createDevice(vk::PhysicalDevice physical, vk::SurfaceKHR surface);

std::tuple<vk::UniqueSwapchainKHR, vk::SurfaceFormatKHR, vk::Extent2D> createSwapchain(
    vk::PhysicalDevice physical, vk::Device device, vk::SurfaceKHR surface,
    const Queues& queues, uint32_t windowWidth, uint32_t windowHeight);

std::vector<vk::UniqueImageView> createImageViews(vk::Device device, vk::SwapchainKHR swapchain,
    const vk::SurfaceFormatKHR& format);
}
