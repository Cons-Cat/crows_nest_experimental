#include "memory.hpp"

#include <cstring>
#include <vulkan/vulkan_core.h>

auto find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties,
                      VkPhysicalDevice* p_physical_device) -> uint32_t {
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(*p_physical_device, &mem_properties);

    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) &&
            (mem_properties.memoryTypes[i].propertyFlags & properties) ==
                properties) {
            return i;
        }
    }
    stx::panic("Failed to find a memory type!");
}

void create_buffer(VkDevice* p_logical_device,
                   VkPhysicalDevice* p_physical_device,
                   VkBufferUsageFlags usageFlags,
                   VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size,
                   VkBuffer* p_buffer, VkDeviceMemory* p_memory, void* p_data) {
    VkBufferCreateInfo buffer_create_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    if (vkCreateBuffer(*p_logical_device, &buffer_create_info, nullptr,
                       p_buffer) != VK_SUCCESS) {
        stx::panic("Failed to create buffer!");
    }

    // Create the memory backing up the buffer handle
    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(*p_logical_device, *p_buffer, &mem_reqs);

    VkMemoryAllocateInfo mem_alloc = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
        .allocationSize = mem_reqs.size,
        .memoryTypeIndex = find_memory_type(
            mem_reqs.memoryTypeBits, memoryPropertyFlags, p_physical_device),
    };

    VkMemoryAllocateFlagsInfoKHR alloc_flags_info{};
    if (usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        alloc_flags_info.sType =
            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
        alloc_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        mem_alloc.pNext = &alloc_flags_info;
    }

    if (vkAllocateMemory(*p_logical_device, &mem_alloc, nullptr, p_memory) !=
        VK_SUCCESS) {
        stx::panic("Failed to allocat buffer memory!");
    }

    if (p_data != nullptr) {
        void* p_mapped;
        if (vkMapMemory(*p_logical_device, *p_memory, 0, size, 0, &p_mapped) !=
            VK_SUCCESS) {
            stx::panic("Failed to map buffer memory!");
        }
        memcpy(p_mapped, p_data, size);

        if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
            VkMappedMemoryRange mapped_range = {
                .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
                .memory = *p_memory,
                .offset = 0,
                .size = size,
            };
            vkFlushMappedMemoryRanges(*p_logical_device, 1, &mapped_range);
        }
        vkUnmapMemory(*p_logical_device, *p_memory);
    }

    // TODO: Check result.
    vkBindBufferMemory(*p_logical_device, *p_buffer, *p_memory, 0);
}
