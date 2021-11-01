#include "acceleration_structure.hpp"

#include <vulkan/vulkan_core.h>

#include "memory.hpp"

// TODO: Improve clarity.
void AccelerationStructure::create_buffer(
    VkDevice& device, VkPhysicalDevice* p_physical_device,
    VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo) {
    VkBufferCreateInfo buffer_create_info{};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.size = buildSizeInfo.accelerationStructureSize;
    buffer_create_info.usage =
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    if (vkCreateBuffer(device, &buffer_create_info, nullptr, &this->buffer) !=
        VK_SUCCESS) {
    }
    VkMemoryRequirements memory_requirements{};
    vkGetBufferMemoryRequirements(device, this->buffer, &memory_requirements);
    VkMemoryAllocateFlagsInfo memory_allocate_flags_info{};
    memory_allocate_flags_info.sType =
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    memory_allocate_flags_info.flags =
        VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkMemoryAllocateInfo memory_allocate_info{};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.pNext = &memory_allocate_flags_info;
    memory_allocate_info.allocationSize = memory_requirements.size;
    memory_allocate_info.memoryTypeIndex = find_memory_type(
        memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        p_physical_device);
    if (vkAllocateMemory(device, &memory_allocate_info, nullptr,
                         &this->memory) != VK_SUCCESS) {
    }
    if (vkBindBufferMemory(device, this->buffer, this->memory, 0) !=
        VK_SUCCESS) {
    }
}
