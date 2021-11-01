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
    // Create the buffer handle
    VkBufferCreateInfo buffer_create_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    // TODO: Check result.
    vkCreateBuffer(*p_logical_device, &buffer_create_info, nullptr, p_buffer);

    // Create the memory backing up the buffer handle
    VkMemoryRequirements mem_reqs;
    VkMemoryAllocateInfo mem_alloc = vks::initializers::memoryAllocateInfo();
    vkGetBufferMemoryRequirements(*p_logical_device, *p_buffer, &mem_reqs);
    mem_alloc.allocationSize = mem_reqs.size;
    // Find a memory type index that fits the properties of the buffer
    mem_alloc.memoryTypeIndex = find_memory_type(
        mem_reqs.memoryTypeBits, memoryPropertyFlags, p_physical_device);
    // If the buffer has VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT set we also
    // need to enable the appropriate flag during allocation
    VkMemoryAllocateFlagsInfoKHR alloc_flags_info{};
    if (usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        alloc_flags_info.sType =
            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
        alloc_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        mem_alloc.pNext = &alloc_flags_info;
    }
    // TODO: Check result.
    vkAllocateMemory(*p_logical_device, &mem_alloc, nullptr, p_memory);

    // If a pointer to the buffer data has been passed, map the buffer and copy
    // over the data
    if (p_data != nullptr) {
        void* p_mapped;
        // TODO: Check result.
        vkMapMemory(*p_logical_device, *p_memory, 0, size, 0, &p_mapped);
        memcpy(p_mapped, p_data, size);

        // If host coherency hasn't been requested, do a manual flush to make
        // writes visible
        if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
            VkMappedMemoryRange mapped_range =
                vks::initializers::mappedMemoryRange();
            mapped_range.memory = *p_memory;
            mapped_range.offset = 0;
            mapped_range.size = size;
            vkFlushMappedMemoryRanges(*p_logical_device, 1, &mapped_range);
        }
        vkUnmapMemory(*p_logical_device, *p_memory);
    }

    // Attach the memory to the buffer object
    // TODO: Check result.
    vkBindBufferMemory(*p_logical_device, *p_buffer, *p_memory, 0);
}
