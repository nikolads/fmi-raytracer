#pragma once

#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_ASSERT [](bool _){}
#include "vulkan/vulkan.hpp"

#include <iostream>
#include <variant>

namespace app {

struct InstanceCreateError {
    vk::Result result;

    InstanceCreateError(vk::Result result):
        result(result)
    {
    }
};

std::ostream& operator<<(std::ostream& os, const InstanceCreateError& err);

using Error = std::variant<InstanceCreateError>;

std::ostream& operator<<(std::ostream& os, const Error& err);

}
