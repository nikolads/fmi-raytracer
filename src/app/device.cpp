#include "boost/range/irange.hpp"

#include "device.h"

#include <algorithm>
#include <experimental/array>
#include <iostream>

using std::experimental::make_array;

namespace app {

vk::PhysicalDevice choosePhysicalDevice(vk::Instance instance, vk::SurfaceKHR surface) {
    const size_t DEVICE_INDEX = 0;

    auto devices = instance.enumeratePhysicalDevices();

    std::cout << "Available devices:" << "\n";
    for (auto& device: devices) {
        auto properties = device.getProperties();
        std::cout << "    [" << std::distance(devices.data(), &device) << "] " << properties.deviceName << "\n";
    }

    std::cout << "Choosing device " << DEVICE_INDEX << "\n";

    // There should be checks if the device supports everything we need..
    // but we just pick the first device found for simplicity
    if (devices.size() < DEVICE_INDEX + 1) {
        throw std::runtime_error("no device");
    }

    return devices[DEVICE_INDEX];
}

std::tuple<vk::UniqueDevice, Queues> createDevice(vk::PhysicalDevice physical, vk::SurfaceKHR surface) {
    const auto deviceExtensions = make_array(
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    );

    const auto queueFamilies = physical.getQueueFamilyProperties();
    const auto queueFamilyIndices = boost::irange(size_t(0), queueFamilies.size());

    const auto computeQueue = std::find_if(queueFamilyIndices.begin(), queueFamilyIndices.end(),
        [&](size_t i){ return queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute; });

    if (computeQueue == queueFamilyIndices.end()) {
        throw std::runtime_error("no queue with compute capability");
    }

    const auto presentQueue = std::find_if(queueFamilyIndices.begin(), queueFamilyIndices.end(),
        [&](size_t i){ return physical.getSurfaceSupportKHR(i, surface); });

    if (presentQueue == queueFamilyIndices.end()) {
        throw std::runtime_error("no queue with present capability");
    }

    const auto queuePriorities = make_array(1.0f);
    auto queueInfos = std::vector<vk::DeviceQueueCreateInfo>();

    queueInfos.push_back(vk::DeviceQueueCreateInfo(
        vk::DeviceQueueCreateFlags(),   // flags
        *computeQueue,                  // queueFamilyIndex
        queuePriorities.size(),         // queueCount
        queuePriorities.data()          // pQueuePriorities
    ));

    if (*presentQueue != *computeQueue) {
        queueInfos.push_back(vk::DeviceQueueCreateInfo(
            vk::DeviceQueueCreateFlags(),   // flags
            *presentQueue,                  // queueFamilyIndex
            queuePriorities.size(),         // queueCount
            queuePriorities.data()          // pQueuePriorities
        ));
    }

    const auto features = vk::PhysicalDeviceFeatures();

    const auto deviceInfo = vk::DeviceCreateInfo(
        vk::DeviceCreateFlags(),        // flags
        queueInfos.size(),              // queueCreateInfoCount
        queueInfos.data(),              // pQueueCreateInfos
        0,                              // enabledLayerCount
        nullptr,                        // ppEnabledLayerNames
        deviceExtensions.size(),        // enabledExtensionCount
        deviceExtensions.data(),        // ppEnabledExtensionNames
        &features
    );

    auto device = physical.createDeviceUnique(deviceInfo, nullptr);
    auto queues = Queues {
        *computeQueue,
        *presentQueue,
        device->getQueue(*computeQueue, 0),
        device->getQueue(*presentQueue, 0)
    };

    return std::make_tuple(std::move(device), queues);
}

vk::SurfaceFormatKHR chooseFormat(const std::vector<vk::SurfaceFormatKHR>& formats) {
    auto preferred = vk::SurfaceFormatKHR { vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };

    if (formats.size() == 1 && formats[0].format == vk::Format::eUndefined) {
        return preferred;
    }

    if (std::find(formats.begin(), formats.end(), preferred) != formats.end()) {
        return preferred;
    }

    throw std::runtime_error("preferred surface format not found");
}

vk::PresentModeKHR choosePresentMode(const std::vector<vk::PresentModeKHR>& modes) {
    return std::find(modes.begin(), modes.end(), vk::PresentModeKHR::eMailbox) != modes.end()
        ? vk::PresentModeKHR::eMailbox
        : vk::PresentModeKHR::eFifo;
}

vk::Extent2D chooseExtent(const vk::SurfaceCapabilitiesKHR& capabilities, uint32_t windowWidth, uint32_t windowHeight) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max() ||
        capabilities.currentExtent.height != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }

    return vk::Extent2D {
        std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, windowWidth)),
        std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, windowHeight))
    };
}

std::tuple<vk::UniqueSwapchainKHR, vk::SurfaceFormatKHR, vk::Extent2D> createSwapchain(
    vk::PhysicalDevice physical, vk::Device device, vk::SurfaceKHR surface,
    const Queues& queues, uint32_t windowWidth, uint32_t windowHeight)
{
    auto capabilities = physical.getSurfaceCapabilitiesKHR(surface);
    auto imageCount = capabilities.maxImageCount == 0
        ? capabilities.minImageCount + 1
        : std::max(capabilities.minImageCount + 1, capabilities.maxImageCount);

    auto format = chooseFormat(physical.getSurfaceFormatsKHR(surface));
    auto presentMode = choosePresentMode(physical.getSurfacePresentModesKHR(surface));
    auto extent = chooseExtent(capabilities, windowWidth, windowHeight);

    vk::SwapchainCreateInfoKHR info;
    if (queues.compute == queues.present) {
        info = vk::SwapchainCreateInfoKHR(
            vk::SwapchainCreateFlagBitsKHR(),       // flags
            surface,                                // surface
            imageCount,                             // minImageCount
            format.format,                          // imageFormat
            format.colorSpace,                      // imageColorSpace
            extent,                                 // imageExtent
            1,                                      // imageArrayLayers
            vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst,   // imageUsage
            vk::SharingMode::eExclusive,            // imageSharingMode
            0,                                      // queueFamilyIndexCount
            nullptr,                                // pQueueFamilyIndices
            capabilities.currentTransform,          // preTransform
            vk::CompositeAlphaFlagBitsKHR::eOpaque, // compositeAlpha
            presentMode,                            // presentMode
            true,                                   // clipped
            nullptr                                 // oldSwapchain
        );
    } else {
        throw std::runtime_error("unimplemented");
    }

    auto swapchain = device.createSwapchainKHRUnique(info, nullptr);
    return std::make_tuple(std::move(swapchain), format, extent);
}

std::vector<vk::UniqueImageView> createImageViews(vk::Device device, vk::SwapchainKHR swapchain,
    const vk::SurfaceFormatKHR& format)
{
    std::vector<vk::UniqueImageView> views;

    for (auto image: device.getSwapchainImagesKHR(swapchain)) {
        auto info = vk::ImageViewCreateInfo(
            vk::ImageViewCreateFlags(),         // flags
            image,                              // image
            vk::ImageViewType::e2D,             // viewType
            format.format,                      // format
            vk::ComponentMapping(),             // components
            vk::ImageSubresourceRange(          // subresourceRange
                vk::ImageAspectFlagBits::eColor,    // aspectMask
                0,                                  // baseMipLevel
                1,                                  // levelCount
                0,                                  // baseArrayLayer
                1                                   // layerCount
            )
        );

        views.push_back(device.createImageViewUnique(info));
    }

    return views;
}

}
