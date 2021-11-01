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
