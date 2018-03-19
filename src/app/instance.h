#pragma once

#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_ASSERT [](bool _){}
#include "vulkan/vulkan.hpp"
#include "GLFW/glfw3.h"

#include <variant>

namespace app {

std::variant<vk::UniqueInstance, vk::Result> createInstance();

}
