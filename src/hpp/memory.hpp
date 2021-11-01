#pragma once

#include <stx/panic.h>
#include <vulkan/vulkan.h>

auto find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties,
                      VkPhysicalDevice* p_physical_device) -> uint32_t;

void create_buffer(VkDevice* p_logical_device,
                   VkPhysicalDevice* p_physical_device,
                   VkBufferUsageFlags usageFlags,
                   VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size,
                   VkBuffer* p_buffer, VkDeviceMemory* p_memory, void* p_data);

void copy_buffer(VkDevice* p_logical_device, VkCommandPool* p_cmd_pool,
                 VkBuffer p_src_buffer, VkBuffer p_dst_buffer,
                 VkDeviceSize size, VkCommandBuffer* p_cmd_buffer,
                 VkQueue* p_queue);
