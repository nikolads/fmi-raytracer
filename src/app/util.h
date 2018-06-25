#include "deps.h"
#include "device.h"

namespace app {

void zeroBuffer(vk::Device device, vk::PhysicalDevice physical, vk::CommandPool commandPool,
    const Queues& queues, vk::Buffer dstBuffer, size_t bufferSize);

uint32_t findMemoryType(vk::PhysicalDevice physical, uint32_t mask, vk::MemoryPropertyFlags desired);

}
