#pragma once

#include "deps.h"
#include "device.h"

namespace app {

std::tuple<vk::UniquePipeline, vk::UniqueShaderModule> createPipeline(vk::Device device);

std::tuple<vk::UniqueCommandPool, vk::UniqueCommandBuffer> createCommands(vk::Device device,
    const Queues& queues, vk::Pipeline pipeline);

}
