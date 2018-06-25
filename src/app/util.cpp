#include "util.h"

namespace app {

void copyBuffer(vk::Device device, vk::CommandPool commandPool, const Queues& queues, vk::Buffer srcBuffer,
    vk::Buffer dstBuffer, size_t bufferSize);

// Fill a GPU buffer with zeroes the way one should not do it
void zeroBuffer(vk::Device device, vk::PhysicalDevice physical, vk::CommandPool commandPool,
    const Queues& queues, vk::Buffer dstBuffer, size_t bufferSize)
{
    const auto info = vk::BufferCreateInfo(
        vk::BufferCreateFlags(),                    // flags
        bufferSize,                                 // size
        vk::BufferUsageFlagBits::eTransferSrc,      // usage
        vk::SharingMode::eExclusive,                // sharingMode
        0,                                          // queueFamilyIndexCount
        nullptr                                     // pQueueFamilyIndices
    );

    auto srcBuffer = device.createBufferUnique(info);

    const auto allocInfo = vk::MemoryAllocateInfo(
        bufferSize,
        findMemoryType(physical, ~0, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
    );

    auto memory = device.allocateMemoryUnique(allocInfo, nullptr);
    device.bindBufferMemory(*srcBuffer, *memory, 0);

    auto ptr = device.mapMemory(*memory, 0, bufferSize, vk::MemoryMapFlags());
    memset(ptr, 0, bufferSize);
    device.unmapMemory(*memory);

    copyBuffer(device, commandPool, queues, *srcBuffer, dstBuffer, bufferSize);
}

void copyBuffer(vk::Device device, vk::CommandPool commandPool, const Queues& queues, vk::Buffer srcBuffer,
    vk::Buffer dstBuffer, size_t bufferSize)
{
    auto allocInfo = vk::CommandBufferAllocateInfo(
        commandPool,                            // commandPool,
        vk::CommandBufferLevel::ePrimary,       // level
        1                                       // commandBufferCount
    );

    auto cmds = device.allocateCommandBuffers(allocInfo);
    auto cmd = std::move(cmds[0]);

    auto beginInfo = vk::CommandBufferBeginInfo(
        vk::CommandBufferUsageFlagBits::eSimultaneousUse,   // flags
        nullptr                                             // pInheritanceInfo
    );
    cmd.begin(beginInfo);

    auto region = vk::BufferCopy(0, 0, bufferSize);
    cmd.copyBuffer(srcBuffer, dstBuffer, region);

    cmd.end();

    auto dstStages = vk::PipelineStageFlags();
    auto submitInfo = vk::SubmitInfo(
        0,                                  // waitSemaphoreCount
        nullptr,                            // pWaitSemaphores
        &dstStages,                         // pWaitDstStageMask
        1,                                  // commandBufferCount
        &cmd,                               // pCommandBuffers
        0,                                  // signalSemaphoreCount
        nullptr                             // pSignalSemaphores
    );

    queues.compute.submit(1, &submitInfo, nullptr);
    queues.compute.waitIdle();
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

} // namespace app
