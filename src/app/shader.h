#pragma once

#include "deps.h"
#include "device.h"

namespace app {

std::tuple<vk::UniqueDeviceMemory, vk::UniqueImage, vk::UniqueImageView> createImage(
    vk::Device device, vk::PhysicalDevice physical, vk::Extent2D extent);

std::tuple<vk::UniqueDeviceMemory, vk::UniqueBuffer> createBuffer(vk::Device device, vk::PhysicalDevice physical);

vk::UniqueDescriptorSetLayout createDescriptorSetLayoyt(vk::Device device);

std::tuple<vk::UniqueDescriptorPool, vk::DescriptorSet> createDescriptorSet(
    vk::Device device, vk::DescriptorSetLayout layout, vk::ImageView workImageView, vk::Buffer buffer);

std::tuple<vk::UniquePipeline, vk::UniquePipelineLayout, vk::UniqueShaderModule> createPipeline(
    vk::Device device, vk::DescriptorSetLayout descriptorLayout);

std::tuple<vk::UniqueCommandPool, std::vector<vk::UniqueCommandBuffer>> createCommands(
    vk::Device device, vk::SwapchainKHR swapchain, const Queues& queues, vk::Pipeline pipeline,
    vk::PipelineLayout pipelineLayout, vk::DescriptorSet descriptorSet, vk::Image workImage,
    vk::Extent2D imageExtent);

}
