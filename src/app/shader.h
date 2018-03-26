#pragma once

#include "deps.h"
#include "device.h"

namespace app {

std::tuple<vk::UniqueBuffer, vk::UniqueDeviceMemory> createBuffer(vk::Device device, vk::PhysicalDevice physical,
    uint32_t height, uint32_t width);

vk::UniqueDescriptorSetLayout createDescriptorSetLayoyt(vk::Device device);

std::tuple<vk::UniqueDescriptorPool, vk::DescriptorSet> createDescriptorSet(
    vk::Device device, vk::DescriptorSetLayout layout, vk::Buffer buffer);

std::tuple<vk::UniquePipeline, vk::UniquePipelineLayout, vk::UniqueShaderModule> createPipeline(
    vk::Device device, vk::DescriptorSetLayout descriptorLayout);

std::tuple<vk::UniqueCommandPool, std::vector<vk::UniqueCommandBuffer>> createCommands(
    vk::Device device, vk::SwapchainKHR swapchain, const Queues& queues, vk::Pipeline pipeline,
    vk::PipelineLayout pipelineLayout, vk::DescriptorSet descriptorSet);

}
