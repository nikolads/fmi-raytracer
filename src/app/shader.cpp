#include "shader.h"

#include <array>
#include <cassert>
#include <experimental/array>
#include <fstream>
#include <vector>

using std::experimental::make_array;

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

std::tuple<vk::UniqueDeviceMemory, vk::UniqueImage, vk::UniqueImageView> createImage(
    vk::Device device, vk::PhysicalDevice physical, vk::Extent2D extent)
{
    const auto info = vk::ImageCreateInfo(
        vk::ImageCreateFlags(),                         // flags
        vk::ImageType::e2D,                             // imageType
        vk::Format::eR32G32B32A32Sfloat,                // format
        vk::Extent3D(extent.width, extent.height, 1),   // extent
        1,                                              // mipLevels
        1,                                              // arrayLayers
        vk::SampleCountFlagBits::e1,                    // samples
        vk::ImageTiling::eOptimal,                      // tiling
        vk::ImageUsageFlagBits::eTransferSrc |
            vk::ImageUsageFlagBits::eTransferDst |
            vk::ImageUsageFlagBits::eStorage,           // usage
        vk::SharingMode::eExclusive,                    // sharingMode
        0,                                              // queueFamilyIndexCount
        nullptr,                                        // pQueueFamilyIndices
        vk::ImageLayout::eUndefined                     // initialLayout
    );

    auto image = device.createImageUnique(info, nullptr);

    const auto requirements = device.getImageMemoryRequirements(*image);
    const auto allocInfo = vk::MemoryAllocateInfo(
        requirements.size,
        findMemoryType(physical, requirements.memoryTypeBits, vk::MemoryPropertyFlags())
    );

    auto memory = device.allocateMemoryUnique(allocInfo, nullptr);
    device.bindImageMemory(*image, *memory, 0);

    const auto viewInfo = vk::ImageViewCreateInfo(
        vk::ImageViewCreateFlags(),             // flags
        *image,                                 // image
        vk::ImageViewType::e2D,                 // viewType
        vk::Format::eR32G32B32A32Sfloat,        // format
        vk::ComponentMapping(),                 // components
        vk::ImageSubresourceRange(              // subresourceRange
            vk::ImageAspectFlagBits::eColor,        // aspectMask
            0,                                      // baseMipLevel
            1,                                      // levelCount
            0,                                      // baseArrayLayer
            1                                       // layerCount
        )
    );

    auto view = device.createImageViewUnique(viewInfo, nullptr);

    return std::make_tuple(std::move(memory), std::move(image), std::move(view));
}

vk::UniqueDescriptorSetLayout createDescriptorSetLayoyt(vk::Device device) {
    const auto binding = vk::DescriptorSetLayoutBinding(
        0,                                      // binding
        vk::DescriptorType::eStorageImage,      // descriptorType
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

std::tuple<vk::UniqueDescriptorPool, vk::DescriptorSet> createDescriptorSet(
    vk::Device device, vk::DescriptorSetLayout layout, vk::ImageView workImageView)
{
    const auto poolSize = vk::DescriptorPoolSize(vk::DescriptorType::eStorageImage, 1);
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

    auto sets = device.allocateDescriptorSets(allocInfo);
    auto set = std::move(sets[0]);

    const auto imageInfo = vk::DescriptorImageInfo(nullptr, workImageView, vk::ImageLayout::eGeneral);
    const auto writeInfo = vk::WriteDescriptorSet(
        set,                                    // dstSet
        0,                                      // dstBinding
        0,                                      // dstArrayElement
        1,                                      // descriptorCount
        vk::DescriptorType::eStorageImage,      // descriptorType
        &imageInfo,                             // pImageInfo
        nullptr,                                // pBufferInfo
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

std::tuple<vk::UniqueCommandPool, std::vector<vk::UniqueCommandBuffer>> createCommands(
    vk::Device device, vk::SwapchainKHR swapchain, const Queues& queues, vk::Pipeline pipeline,
    vk::PipelineLayout pipelineLayout, vk::DescriptorSet descriptorSet, vk::Image workImage,
    vk::Extent2D extent)
{
    auto poolInfo = vk::CommandPoolCreateInfo(
        vk::CommandPoolCreateFlags(),           // flags
        queues.computeQueueFamily               // queueFamilyIndex
    );

    auto pool = device.createCommandPoolUnique(poolInfo, nullptr);

    auto images = device.getSwapchainImagesKHR(swapchain);

    auto allocInfo = vk::CommandBufferAllocateInfo(
        *pool,                                  // commandPool,
        vk::CommandBufferLevel::ePrimary,       // level
        images.size()                           // commandBufferCount
    );

    auto buffers = device.allocateCommandBuffersUnique(allocInfo);

    for (size_t i = 0; i < buffers.size(); i++) {
        auto& buffer = buffers[i];
        auto& image = images[i];

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

        const auto initialLayouts = make_array(
            vk::ImageMemoryBarrier(
                vk::AccessFlags(),                      // srcAccessMask
                vk::AccessFlagBits::eMemoryWrite,       // dstAccessMask
                vk::ImageLayout::eUndefined,            // oldLayout
                vk::ImageLayout::eTransferDstOptimal,   // newLayout
                queues.computeQueueFamily,              // srcQueueFamilyIndex
                queues.computeQueueFamily,              // dstQueueFamilyIndex
                image,                                  // image
                vk::ImageSubresourceRange(              // subresourceRange
                    vk::ImageAspectFlagBits::eColor,        // aspectMask
                    0,                                      // baseMipLevel
                    1,                                      // levelCount
                    0,                                      // baseArrayLayer
                    1                                       // layerCount
                )
            ),
            vk::ImageMemoryBarrier(
                vk::AccessFlags(),                      // srcAccessMask
                vk::AccessFlagBits::eMemoryWrite |
                    vk::AccessFlagBits::eShaderRead |
                    vk::AccessFlagBits::eShaderWrite,       // dstAccessMask
                vk::ImageLayout::eUndefined,            // oldLayout
                vk::ImageLayout::eGeneral,              // newLayout
                queues.computeQueueFamily,              // srcQueueFamilyIndex
                queues.computeQueueFamily,              // dstQueueFamilyIndex
                workImage,                              // image
                vk::ImageSubresourceRange(              // subresourceRange
                    vk::ImageAspectFlagBits::eColor,        // aspectMask
                    0,                                      // baseMipLevel
                    1,                                      // levelCount
                    0,                                      // baseArrayLayer
                    1                                       // layerCount
                )
            )
        );

        buffer->pipelineBarrier(
            vk::PipelineStageFlagBits::eTopOfPipe,      // srcStageMask
            vk::PipelineStageFlagBits::eComputeShader,  // dstStageMask
            vk::DependencyFlags(),                      // dependencyFlags
            0,                                          // memoryBarrierCount
            nullptr,                                    // pMemoryBarriers
            0,                                          // bufferMemoryBarrierCount
            nullptr,                                    // pBufferMemoryBarriers
            initialLayouts.size(),                      // imageMemoryBarrierCount
            initialLayouts.data()                       // pImageMemoryBarriers
        );

        auto clearRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
        auto clearColor = vk::ClearColorValue(make_array(0.0f, 0.0f, 0.0f, 1.0f));
        buffer->clearColorImage(
            workImage,                                  // image
            vk::ImageLayout::eGeneral,                  // imageLayout
            &clearColor,                                // pColor
            1,                                          // rangeCount
            &clearRange                                 // pRange
        );

        const size_t WORKGROUP_SIZE = 32;
        const size_t N_RAYS = 4;
        auto workgroupsX = (extent.width / WORKGROUP_SIZE) + (extent.width % WORKGROUP_SIZE != 0);
        auto workgroupsY = (extent.height / WORKGROUP_SIZE) + (extent.height % WORKGROUP_SIZE != 0);
        buffer->dispatch(workgroupsX, workgroupsY, N_RAYS);

        const auto generalToTransfer = vk::ImageMemoryBarrier(
            vk::AccessFlags(),                      // srcAccessMask
            vk::AccessFlagBits::eMemoryWrite,       // dstAccessMask
            vk::ImageLayout::eGeneral,              // oldLayout
            vk::ImageLayout::eTransferSrcOptimal,   // newLayout
            queues.computeQueueFamily,              // srcQueueFamilyIndex
            queues.computeQueueFamily,              // dstQueueFamilyIndex
            workImage,                              // image
            vk::ImageSubresourceRange(              // subresourceRange
                vk::ImageAspectFlagBits::eColor,        // aspectMask
                0,                                      // baseMipLevel
                1,                                      // levelCount
                0,                                      // baseArrayLayer
                1                                       // layerCount
            )
        );

        buffer->pipelineBarrier(
            vk::PipelineStageFlagBits::eComputeShader,  // srcStageMask
            vk::PipelineStageFlagBits::eBottomOfPipe,   // dstStageMask
            vk::DependencyFlags(),                      // dependencyFlags
            0,                                          // memoryBarrierCount
            nullptr,                                    // pMemoryBarriers
            0,                                          // bufferMemoryBarrierCount
            nullptr,                                    // pBufferMemoryBarriers
            1,                                          // imageMemoryBarrierCount
            &generalToTransfer                          // pImageMemoryBarriers
        );

        const auto offsets = make_array(
            vk::Offset3D(0, 0, 0),
            vk::Offset3D(extent.width, extent.height, 1));

        const auto copyInfo = vk::ImageBlit(
            vk::ImageSubresourceLayers(                     // srcSubresource
                vk::ImageAspectFlagBits::eColor,                // aspectMask
                0,                                              // mipLevel
                0,                                              // baseArrayLayer
                1                                               // layerCount
            ),
            offsets,                                        // srcOffsets
            vk::ImageSubresourceLayers(                     // dstSubresource
                vk::ImageAspectFlagBits::eColor,                // aspectMask
                0,                                              // mipLevel
                0,                                              // baseArrayLayer
                1                                               // layerCount
            ),
            offsets                                         // dstOffsets
        );

        buffer->blitImage(
            workImage,                              // srcImage
            vk::ImageLayout::eTransferSrcOptimal,   // srcImageLayout
            image,                                  // dstImage
            vk::ImageLayout::eTransferDstOptimal,   // dstImageLayout
            1,                                      // regionCount
            &copyInfo,                              // pRegions
            vk::Filter::eNearest                    // filter
        );

        const auto transferToPresent = vk::ImageMemoryBarrier(
            vk::AccessFlagBits::eShaderRead,            // srcAccessMask
            vk::AccessFlagBits::eMemoryWrite,           // dstAccessMask
            vk::ImageLayout::eTransferDstOptimal,       // oldLayout
            vk::ImageLayout::ePresentSrcKHR,            // newLayout
            queues.computeQueueFamily,                  // srcQueueFamilyIndex
            queues.computeQueueFamily,                  // dstQueueFamilyIndex
            image,                                      // image
            vk::ImageSubresourceRange(                  // subresourceRange
                vk::ImageAspectFlagBits::eColor,            // aspectMask
                0,                                          // baseMipLevel
                1,                                          // levelCount
                0,                                          // baseArrayLayer
                1                                           // layerCount
            )
        );

        buffer->pipelineBarrier(
            vk::PipelineStageFlagBits::eComputeShader,  // srcStageMask
            vk::PipelineStageFlagBits::eBottomOfPipe,   // dstStageMask
            vk::DependencyFlags(),                      // dependencyFlags
            0,                                          // memoryBarrierCount
            nullptr,                                    // pMemoryBarriers
            0,                                          // bufferMemoryBarrierCount
            nullptr,                                    // pBufferMemoryBarriers
            1,                                          // imageMemoryBarrierCount
            &transferToPresent                          // pImageMemoryBarriers
        );

        buffer->end();
    }

    return std::make_tuple(std::move(pool), std::move(buffers));
}

}
