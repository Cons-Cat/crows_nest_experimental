#pragma once

#include <stx/panic.h>
#include <vulkan/vulkan.h>

struct AccelerationStructure {
    VkAccelerationStructureKHR handle;
    uint64_t device_address = 0;
    VkDeviceMemory memory;
    VkBuffer buffer;

    void create_buffer(VkDevice& device, VkPhysicalDevice* p_physical_device,
                       VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo);
};
