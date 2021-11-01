#pragma once

#include <stx/panic.h>
#include <vulkan/vulkan.h>

struct AccelerationStructure {
    VkAccelerationStructureKHR handle;
    uint64_t device_address = 0;
    VkDeviceMemory memory;
    VkBuffer buffer;

    void create_buffer(VkDevice* p_logical_device,
                       VkPhysicalDevice* p_physical_device, VkDeviceSize size,
                       VkBufferUsageFlags usage_flags,
                       VkMemoryPropertyFlags property_flags, VkBuffer* p_buffer,
                       VkDeviceMemory* p_buffer_memory);
};
