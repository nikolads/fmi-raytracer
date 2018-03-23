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
    auto queues = Queues(device->getQueue(*computeQueue, 0), device->getQueue(*presentQueue, 0));

    return std::make_tuple(std::move(device), queues);
}

}
