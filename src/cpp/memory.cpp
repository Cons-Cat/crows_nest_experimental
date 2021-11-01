#include "memory.hpp"

#include <cstring>
#include <vulkan/vulkan_core.h>

auto find_memory_type(uint32_t memory_type_index,
                      VkMemoryPropertyFlags properties,
                      VkPhysicalDevice* p_physical_device) -> uint32_t {
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(*p_physical_device, &mem_properties);

    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
        if ((memory_type_index & (1 << i)) &&
            (mem_properties.memoryTypes[i].propertyFlags & properties) ==
                properties) {
            return i;
        }
    }
    stx::panic("Failed to find a memory type!");
}

void create_buffer(VkDevice* p_logical_device,
                   VkPhysicalDevice* p_physical_device,
                   VkBufferUsageFlags usage_flags,
                   VkMemoryPropertyFlags memory_property_flags,
                   VkDeviceSize size, VkBuffer* p_buffer,
                   VkDeviceMemory* p_memory, void* p_data) {
    VkBufferCreateInfo buffer_create_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage_flags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    if (vkCreateBuffer(*p_logical_device, &buffer_create_info, nullptr,
                       p_buffer) != VK_SUCCESS) {
        stx::panic("Failed to create buffer!");
    }

    // Create the memory backing up the buffer handle
    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(*p_logical_device, *p_buffer,
                                  &memory_requirements);

    VkMemoryAllocateInfo mem_alloc = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex =
            find_memory_type(memory_requirements.memoryTypeBits,
                             memory_property_flags, p_physical_device),
    };

    // TODO: Do I need this?
    // VkMemoryAllocateFlagsInfoKHR alloc_flags_info{};
    // if (usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
    //     alloc_flags_info.sType =
    //         VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
    //     alloc_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    //     mem_alloc.pNext = &alloc_flags_info;
    // }

    if (vkAllocateMemory(*p_logical_device, &mem_alloc, nullptr, p_memory) !=
        VK_SUCCESS) {
        stx::panic("Failed to allocate buffer memory!");
    }

    // TODO: Do I need this?
    // if (p_data != nullptr) {
    //     void* p_mapped;
    //     if (vkMapMemory(*p_logical_device, *p_memory, 0, size, 0, &p_mapped)
    //     !=
    //         VK_SUCCESS) {
    //         stx::panic("Failed to map buffer memory!");
    //     }
    //     memcpy(p_mapped, p_data, size);
    //     if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) ==
    //     0) {
    //         VkMappedMemoryRange mapped_range = {
    //             .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
    //             .memory = *p_memory,
    //             .offset = 0,
    //             .size = size,
    //         };
    //         vkFlushMappedMemoryRanges(*p_logical_device, 1, &mapped_range);
    //     }
    //     vkUnmapMemory(*p_logical_device, *p_memory);
    // }

    // TODO: Check result.
    vkBindBufferMemory(*p_logical_device, *p_buffer, *p_memory, 0);
}

// TODO: Clean up.
void copy_buffer(VkDevice* p_logical_device, VkCommandPool* p_cmd_pool,
                 VkBuffer p_src_buffer, VkBuffer p_dst_buffer,
                 VkDeviceSize size, VkCommandBuffer* p_cmd_buffer,
                 VkQueue* p_queue) {
    VkCommandBufferAllocateInfo buffer_allocate_info = {};
    buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    buffer_allocate_info.commandPool = *p_cmd_pool;
    buffer_allocate_info.commandBufferCount = 1;

    VkCommandBuffer p_commandBuffer;
    vkAllocateCommandBuffers(*p_logical_device, &buffer_allocate_info,
                             p_cmd_buffer);

    VkCommandBufferBeginInfo command_buffer_begin_info = {};
    command_buffer_begin_info.sType =
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.flags =
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(p_commandBuffer, &command_buffer_begin_info);
    VkBufferCopy buffer_copy = {};
    buffer_copy.size = size;
    vkCmdCopyBuffer(p_commandBuffer, p_src_buffer, p_dst_buffer, 1,
                    &buffer_copy);
    vkEndCommandBuffer(p_commandBuffer);

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &p_commandBuffer;

    vkQueueSubmit(*p_queue, 1, &submit_info, nullptr);
    vkQueueWaitIdle(*p_queue);

    vkFreeCommandBuffers(*p_logical_device, *p_cmd_pool, 1, p_cmd_buffer);
}
