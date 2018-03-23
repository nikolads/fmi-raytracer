#pragma once

#include "deps.h"

#include <tuple>

namespace app {

struct Queues {
    vk::Queue compute;
    vk::Queue present;

    Queues(vk::Queue compute, vk::Queue present): compute(compute), present(present) {}
};

vk::PhysicalDevice choosePhysicalDevice(vk::Instance instance, vk::SurfaceKHR surface);
std::tuple<vk::UniqueDevice, Queues> createDevice(vk::PhysicalDevice physical, vk::SurfaceKHR surface);

}
