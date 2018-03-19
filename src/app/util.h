#pragma once

#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_ASSERT [](bool _){}
#include "vulkan/vulkan.hpp"

#include <variant>

template<typename T> std::variant<T, vk::Result> asVariant(vk::ResultValue<T> packed) {
    return packed.result == vk::Result::eSuccess
        ? std::variant<T, vk::Result>(std::move(packed.value))
        : std::variant<T, vk::Result>(std::move(packed.result));
}

#define RETURN_ON_ERROR(result, ErrType)                        \
    if (std::holds_alternative<vk::Result>(result)) {           \
        return Error(ErrType(std::get<vk::Result>(result)));    \
    }                                                           \

