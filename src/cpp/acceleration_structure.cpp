#include "acceleration_structure.hpp"

#include "memory.hpp"

void AccelerationStructure::create_buffer(VkDevice* p_logical_device,
                                          VkPhysicalDevice* p_physical_device,
                                          VkDeviceSize size,
                                          VkBufferUsageFlags usage_flags,
                                          VkMemoryPropertyFlags property_flags,
                                          VkBuffer* p_buffer,
                                          VkDeviceMemory* p_buffer_memory) {
}
