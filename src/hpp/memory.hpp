#pragma once

#include <stx/panic.h>
#include <vulkan/vulkan.h>

auto find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties,
                      VkPhysicalDevice& physical_device) -> uint32_t;

void create_buffer(VkDevice& logical_device, VkPhysicalDevice& physical_device,
                   VkBufferUsageFlags usage_flags,
                   VkMemoryPropertyFlags memory_property_flags,
                   VkDeviceSize size, VkBuffer* p_buffer,
                   VkDeviceMemory* p_memory, void* p_data);

void copy_buffer(VkDevice& logical_device, VkCommandPool& cmd_pool,
                 VkBuffer* p_src_buffer, VkBuffer* p_dst_buffer,
                 VkDeviceSize size, VkCommandBuffer* p_cmd_buffer,
                 VkQueue& queue);
