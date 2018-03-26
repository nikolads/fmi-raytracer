#include "shader.h"

#include <cassert>
#include <fstream>
#include <vector>

namespace app {

std::vector<uint32_t> loadShader(const char* filename) {
    auto file = std::fstream(filename, std::ios::in | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("shader file not found");
    }

    file.seekp(0, std::ios::end);
    size_t fileLen = file.tellg();
    file.seekp(0, std::ios::beg);

    size_t bufferLen = fileLen + ((4 - fileLen % 4) % 4);

    auto bytes = std::vector<char>(bufferLen, 0);
    file.read(bytes.data(), fileLen);

    auto code = std::vector<uint32_t>();
    code.reserve(bufferLen / 4);

    for (size_t i = 0; i < bufferLen; i += 4) {
        uint32_t b0 = (uint32_t)(static_cast<uint8_t>(bytes[i + 0]));
        uint32_t b1 = (uint32_t)(static_cast<uint8_t>(bytes[i + 1]));
        uint32_t b2 = (uint32_t)(static_cast<uint8_t>(bytes[i + 2]));
        uint32_t b3 = (uint32_t)(static_cast<uint8_t>(bytes[i + 3]));

        code.push_back((b0 << 0) | (b1 << 8) | (b2 << 16) | (b3 << 24));
    }

    return code;
}

std::tuple<vk::UniquePipeline, vk::UniqueShaderModule> createPipeline(vk::Device device) {
    auto layoutInfo = vk::PipelineLayoutCreateInfo(
        vk::PipelineLayoutCreateFlags(),        // flags
        0,                                      // setLayoutCount
        nullptr,                                // pSetLayouts
        0,                                      // pushConstantRangeCount
        nullptr                                 // pPushConstantRanges
    );

    auto layout = device.createPipelineLayoutUnique(layoutInfo, nullptr);

    auto code = loadShader("shader/comp.spv");
    auto shaderInfo = vk::ShaderModuleCreateInfo(
        vk::ShaderModuleCreateFlags(),          // flags
        code.size() * 4,                        // codeSize
        code.data()                             // pCode
    );

    auto shader = device.createShaderModuleUnique(shaderInfo, nullptr);

    auto stageInfo = vk::PipelineShaderStageCreateInfo(
        vk::PipelineShaderStageCreateFlags(),   // flags
        vk::ShaderStageFlagBits::eCompute,      // stage
        *shader,                                // module
        "main",                                 // pName
        nullptr                                 // pSpecializationInfo
    );

    auto info = vk::ComputePipelineCreateInfo(
        vk::PipelineCreateFlags(),              // flags
        stageInfo,                              // stage
        *layout,                                // layout
        nullptr,                                // basePipelineHandle
        0                                       // basePipelineIndex
    );

    auto pipeline = device.createComputePipelineUnique(nullptr, info, nullptr);

    return std::make_tuple(std::move(pipeline), std::move(shader));
}

std::tuple<vk::UniqueCommandPool, vk::UniqueCommandBuffer> createCommands(vk::Device device,
    const Queues& queues, vk::Pipeline pipeline)
{
    auto poolInfo = vk::CommandPoolCreateInfo(
        vk::CommandPoolCreateFlags(),           // flags
        queues.computeQueueFamily               // queueFamilyIndex
    );

    auto pool = device.createCommandPoolUnique(poolInfo, nullptr);

    // TODO: from the tutorial
    // One of the drawing commands involves binding the right `VkFramebuffer`, so we'll have to record
    // a command buffer for every image in the swap chain.

    auto allocInfo = vk::CommandBufferAllocateInfo(
        *pool,                                  // commandPool,
        vk::CommandBufferLevel::ePrimary,       // level
        1                                       // commandBufferCount
    );

    auto buffers = device.allocateCommandBuffersUnique(allocInfo);
    auto buffer = std::move(buffers[0]);

    auto beginInfo = vk::CommandBufferBeginInfo(
        vk::CommandBufferUsageFlagBits::eSimultaneousUse,   // flags
        nullptr                                             // pInheritanceInfo
    );

    buffer->begin(beginInfo);

    buffer->bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);
    // buffer->cmdBindDescriptorSets(...);
    buffer->dispatch(1, 1, 1);

    buffer->end();

    return std::make_tuple(std::move(pool), std::move(buffer));
}

}
