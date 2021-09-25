#pragma once
#include <vulkan/vulkan.hpp>

namespace global {

// These declarations are `NOLINT`ed because global mutable variables are
// unsafe. These are written hastily, unfortunately.

struct vulkan_physical_device_features {
    vk::PhysicalDeviceFeatures features_basic;
    VkPhysicalDeviceAccelerationStructureFeaturesKHR
        features_acceleration_structure{};
    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR
        features_buffer_device_address{};
    VkPhysicalDeviceDescriptorIndexingFeaturesEXT
        features_descriptor_indexing{};
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR
        features_ray_tracing_pipeline{};
    VkPhysicalDeviceScalarBlockLayoutFeaturesEXT features_scalar_block_layout{};
};

extern vulkan_physical_device_features device_features;  // NOLINT
extern bool prepared;                                    // NOLINT

}  // namespace global
