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

uint32_t findMemoryType(vk::PhysicalDevice physical, uint32_t mask, vk::MemoryPropertyFlags desired) {
    auto available = physical.getMemoryProperties();

    for (uint32_t i = 0; i < available.memoryTypeCount; i++) {
        if ((mask & (1 << i)) && (available.memoryTypes[i].propertyFlags & desired) == desired) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

std::tuple<vk::UniqueBuffer, vk::UniqueDeviceMemory> createBuffer(vk::Device device, vk::PhysicalDevice physical,
    uint32_t height, uint32_t width)
{
    const auto info = vk::BufferCreateInfo(
        vk::BufferCreateFlags(),                                                            // flags
        height * width * 4 * sizeof(float),                                                                      // size
        vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eStorageBuffer,    // usage
        vk::SharingMode::eExclusive,                                                        // sharingMode
        0,                                                                                  // queueFamilyIndexCount
        nullptr                                                                             // pQueueFamilyIndices
    );

    auto buffer = device.createBufferUnique(info, nullptr);

    const auto requirements = device.getBufferMemoryRequirements(*buffer);
    const auto allocInfo = vk::MemoryAllocateInfo(
        requirements.size,                                                                  // allocationSize
        findMemoryType(physical, requirements.memoryTypeBits, vk::MemoryPropertyFlags())    // memoryTypeIndex
    );

    auto memory = device.allocateMemoryUnique(allocInfo, nullptr);
    device.bindBufferMemory(*buffer, *memory, 0);

    return std::make_tuple(std::move(buffer), std::move(memory));
}

vk::UniqueDescriptorSetLayout createDescriptorSetLayoyt(vk::Device device) {
    const auto binding = vk::DescriptorSetLayoutBinding(
        0,                                      // binding
        vk::DescriptorType::eStorageBuffer,     // descriptorType
        1,                                      // descriptorCount
        vk::ShaderStageFlagBits::eCompute,      // stageFlags
        nullptr                                 // pImmutableSamplers
    );

    const auto layoutInfo = vk::DescriptorSetLayoutCreateInfo(
        vk::DescriptorSetLayoutCreateFlags(),   // flags
        1,                                      // bindingCount
        &binding                                // pBindings
    );

    return device.createDescriptorSetLayoutUnique(layoutInfo, nullptr);
}

std::tuple<vk::UniqueDescriptorPool, vk::UniqueDescriptorSet> createDescriptorSet(
    vk::Device device, vk::DescriptorSetLayout layout, vk::Buffer buffer)
{
    const auto poolSize = vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, 1);
    const auto poolInfo = vk::DescriptorPoolCreateInfo(
        vk::DescriptorPoolCreateFlags(),        // flags
        1,                                      // maxSets
        1,                                      // poolSizeCount
        &poolSize                               // pPoolSizes
    );

    auto pool = device.createDescriptorPoolUnique(poolInfo, nullptr);

    const auto allocInfo = vk::DescriptorSetAllocateInfo(
        *pool,                                  // descriptorPool
        1,                                      // descriptorSetCount
        &layout                                 // pSetLayouts
    );

    auto sets = device.allocateDescriptorSetsUnique(allocInfo);
    auto set = std::move(sets[0]);

    const auto bufferInfo = vk::DescriptorBufferInfo(buffer, 0, VK_WHOLE_SIZE);
    const auto writeInfo = vk::WriteDescriptorSet(
        *set,                                   // dstSet
        0,                                      // dstBinding
        0,                                      // dstArrayElement
        1,                                      // descriptorCount
        vk::DescriptorType::eStorageBuffer,     // descriptorType
        nullptr,                                // pImageInfo
        &bufferInfo,                            // pBufferInfo
        nullptr                                 // pTexelBufferView
    );

    device.updateDescriptorSets(1, &writeInfo, 0, nullptr);

    return std::make_tuple(std::move(pool), std::move(set));
}

std::tuple<vk::UniquePipeline, vk::UniquePipelineLayout, vk::UniqueShaderModule> createPipeline(
    vk::Device device, vk::DescriptorSetLayout descriptorLayout)
{
    auto layoutInfo = vk::PipelineLayoutCreateInfo(
        vk::PipelineLayoutCreateFlags(),        // flags
        1,                                      // setLayoutCount
        &descriptorLayout,                      // pSetLayouts
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

    return std::make_tuple(std::move(pipeline), std::move(layout), std::move(shader));
}

std::tuple<vk::UniqueCommandPool, vk::UniqueCommandBuffer> createCommands(vk::Device device,
    const Queues& queues, vk::Pipeline pipeline, vk::PipelineLayout pipelineLayout, vk::DescriptorSet descriptorSet)
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
    buffer->bindDescriptorSets(
        vk::PipelineBindPoint::eCompute,        // pipelineBindPoint,
        pipelineLayout,                         // layout
        0,                                      // firstSet
        1,                                      // descriptorSetCount
        &descriptorSet,                         // pDescriptorSets
        0,                                      // dynamicOffsetCount
        nullptr                                 // pDynamicOffsets
    );

    buffer->dispatch(1, 1, 1);

    buffer->end();

    return std::make_tuple(std::move(pool), std::move(buffer));
}

}
