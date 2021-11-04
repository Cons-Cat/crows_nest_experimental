#include "memory.hpp"

#include <cstring>
#include <vulkan/vulkan_core.h>

auto find_memory_type(uint32_t memory_type_index,
                      VkMemoryPropertyFlags properties,
                      VkPhysicalDevice& physical_device) -> uint32_t {
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
        if ((memory_type_index & (1 << i)) &&
            (mem_properties.memoryTypes[i].propertyFlags & properties) ==
                properties) {
            return i;
        }
    }
    stx::panic("Failed to find a memory type!");
}

void create_buffer(VkDevice& logical_device, VkPhysicalDevice& physical_device,
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
    if (vkCreateBuffer(logical_device, &buffer_create_info, nullptr,
                       p_buffer) != VK_SUCCESS) {
        stx::panic("Failed to create buffer!");
    }
    // Create the memory backing up the buffer handle
    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(logical_device, *p_buffer,
                                  &memory_requirements);

    VkMemoryAllocateInfo mem_alloc = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex =
            find_memory_type(memory_requirements.memoryTypeBits,
                             memory_property_flags, physical_device),
    };
    if (vkAllocateMemory(logical_device, &mem_alloc, nullptr, p_memory) !=
        VK_SUCCESS) {
        stx::panic("Failed to allocate buffer memory!");
    }
    if (vkBindBufferMemory(logical_device, *p_buffer, *p_memory, 0) !=
        VK_SUCCESS) {
        stx::panic("Failed to bind buffer memory!");
    }
}

void copy_buffer(VkDevice& logical_device, VkCommandPool& cmd_pool,
                 VkBuffer* p_src_buffer, VkBuffer* p_dst_buffer,
                 VkDeviceSize size, VkCommandBuffer* p_cmd_buffer,
                 VkQueue& queue) {
    // Submit a copy command to a one-time use buffer, and block until it
    // completes.
    VkCommandBufferAllocateInfo buffer_allocate_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = cmd_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    VkCommandBuffer p_command_buffer;
    if (vkAllocateCommandBuffers(logical_device, &buffer_allocate_info,
                                 p_cmd_buffer) != VK_SUCCESS) {
        stx::panic("Failed to allocate a command buffer for copy operation!");
    }
    VkCommandBufferBeginInfo command_buffer_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    if (vkBeginCommandBuffer(p_command_buffer, &command_buffer_begin_info) !=
        VK_SUCCESS) {
        stx::panic("Failed to begin a command buffer for copy operation!");
    }
    VkBufferCopy buffer_copy = {
        .size = size,
    };
    vkCmdCopyBuffer(*p_cmd_buffer, *p_src_buffer, *p_dst_buffer, 1,
                    &buffer_copy);
    if (vkEndCommandBuffer(p_command_buffer) != VK_SUCCESS) {
        stx::panic("Failed to end the command buffer for copy operation!");
    }
    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &p_command_buffer,
    };
    if (vkQueueSubmit(queue, 1, &submit_info, nullptr) != VK_SUCCESS) {
        stx::panic("Failed to submit a command queue for copy operation!");
    }
    if (vkQueueWaitIdle(queue) != VK_SUCCESS) {
        stx::panic("Failed to wait on the command queue for copy operation!");
    }
    vkFreeCommandBuffers(logical_device, cmd_pool, 1, p_cmd_buffer);
}
