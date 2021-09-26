#pragma once
#include <vulkan/vulkan.hpp>

#include <SDL.h>
#include <SDL_vulkan.h>
#include <assert.h>
#include <iostream>
#include <optional>

#include "vk_globals.hpp"

namespace crow {

inline auto make_vk_extensions(SDL_Window* p_window)
    -> std::vector<char const*> {
    uint32_t extension_count = 0;
    SDL_Vulkan_GetInstanceExtensions(p_window, &extension_count, nullptr);
    std::vector<char const*> extension_names;
    SDL_Vulkan_GetInstanceExtensions(p_window, &extension_count,
                                     extension_names.data());

    extension_names.push_back("VK_KHR_surface");
    return extension_names;
}

inline auto make_vk_features() {
    global::device_features.features_basic = {
#ifdef DEBUG
        .fragmentStoresAndAtomics = VK_TRUE,
#endif
        .shaderSampledImageArrayDynamicIndexing = VK_TRUE,
        .shaderStorageBufferArrayDynamicIndexing = VK_TRUE,
    };

    global::device_features.features_acceleration_structure = {
        .sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
        .accelerationStructure = VK_TRUE,
        .descriptorBindingAccelerationStructureUpdateAfterBind = VK_TRUE,
    };

    global::device_features.features_buffer_device_address = {
        .sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR,
        .bufferDeviceAddress = VK_TRUE,
    };

    global::device_features.features_descriptor_indexing = {
        .sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT,
        .shaderUniformTexelBufferArrayDynamicIndexing = VK_TRUE,
        .shaderStorageTexelBufferArrayDynamicIndexing = VK_TRUE,
        .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
        .shaderStorageBufferArrayNonUniformIndexing = VK_TRUE,
        .shaderUniformTexelBufferArrayNonUniformIndexing = VK_TRUE,
        .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
        .descriptorBindingStorageImageUpdateAfterBind = VK_TRUE,
        .descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE,
        .descriptorBindingUniformTexelBufferUpdateAfterBind = VK_TRUE,
        .descriptorBindingStorageTexelBufferUpdateAfterBind = VK_TRUE,
        .descriptorBindingUpdateUnusedWhilePending = VK_TRUE,
        .descriptorBindingPartiallyBound = VK_TRUE,
        .runtimeDescriptorArray = VK_TRUE,
    };

    global::device_features.features_ray_tracing_pipeline = {
        .sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,
        .rayTracingPipeline = VK_TRUE,
        .rayTracingPipelineTraceRaysIndirect = VK_TRUE,
    };

    global::device_features.features_scalar_block_layout = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES,
        .scalarBlockLayout = VK_TRUE,
    };

    // global::device_features.features_basic.pNext = &global::;

    global::device_features.features_acceleration_structure.pNext =
        &global::device_features.features_buffer_device_address;
    global::device_features.features_buffer_device_address.pNext =
        &global::device_features.features_descriptor_indexing;
    global::device_features.features_descriptor_indexing.pNext =
        &global::device_features.features_ray_tracing_pipeline;
    global::device_features.features_ray_tracing_pipeline.pNext =
        &global::device_features.features_scalar_block_layout;
    // TODO:
    // device_params.next = &features_acceleration_structure;
}

inline auto make_vk_layer_names() -> std::vector<char const*> {
    std::vector<const char*> layer_names{
#ifdef CMAKE_DEBUG
        "VK_LAYER_KHRONOS_validation",
#endif
    };
    return layer_names;
}

inline auto make_vk_instance_info(
    std::vector<char const*> const* p_extension_names,
    std::vector<char const*> const* p_layer_names,
    vk::ApplicationInfo const* p_app_info) -> vk::InstanceCreateInfo {
    vk::InstanceCreateInfo instance_info{};
    instance_info.sType = vk::StructureType::eInstanceCreateInfo;
    instance_info.pApplicationInfo = p_app_info;
    instance_info.enabledLayerCount = p_layer_names->size();
    instance_info.ppEnabledLayerNames = p_layer_names->data();
    instance_info.enabledExtensionCount = p_extension_names->size();
    instance_info.ppEnabledExtensionNames = p_extension_names->data();
    return instance_info;
}

inline auto is_device_capable(vk::PhysicalDevice device)
    -> std::optional<size_t> {
    vk::PhysicalDeviceProperties2 device_properties = device.getProperties2();

    // TODO: This might not be reliable.
    // Find a video card that can support raytracing and blitting.
    std::vector<vk::QueueFamilyProperties> queue_family_properties =
        device.getQueueFamilyProperties();
    auto property_iterator = std::find_if(
        queue_family_properties.begin(), queue_family_properties.end(),
        [](vk::QueueFamilyProperties const& qfp) {
            // return qfp.queueFlags & vk::QueueFlagBits::eCompute;
            return qfp.queueFlags & vk::QueueFlagBits::eCompute;
        });
    size_t compute_queue_family_index =
        std::distance(queue_family_properties.begin(), property_iterator);
    if (compute_queue_family_index < queue_family_properties.size()) {
        return std::optional<size_t>(compute_queue_family_index);
    }
    return std::nullopt;
}

inline void get_vk_features() {
    // TODO: Get the features needed for rendering this game, to verify that a
    // given physical device is viable.
}

inline auto make_device_extensions() -> std::vector<char const*> {
    std::vector<char const*> extension_names;
    extension_names.insert(extension_names.end(),
                           {
                               VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
                               VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
                               VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
                               VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
                               VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
                               VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
                               VK_KHR_SPIRV_1_4_EXTENSION_NAME,
                               VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
                               VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME,
                           });
    return extension_names;
}

inline auto make_vk_logical_device(vk::Instance* p_instance,
                                   std::vector<char const*>& device_extensions)
    -> vk::Device {
    // TODO: Check all video cards and find ideal one.
    vk::PhysicalDevice capable_physical_device = VK_NULL_HANDLE;
    std::vector<vk::PhysicalDevice> physical_devices =
        p_instance->enumeratePhysicalDevices();
    if (physical_devices.empty()) {
        throw std::runtime_error("Failed to find a Vulkan-capable GPU.");
    }
    size_t compute_queue_family_index = 0;
    uint32_t device_index = 0;

    for (int i = 0; i < physical_devices.size(); i++) {
        auto device = physical_devices[i];
        device_index = i;
        std::optional<size_t> maybe_index = is_device_capable(device);
        if (maybe_index.has_value()) {
            compute_queue_family_index = maybe_index.value();
            capable_physical_device = device;
            break;
        }
    }
    if (!capable_physical_device) {
        throw std::runtime_error("Failed to find a GPU for raytracing.");
    }
#ifdef CMAKE_DEBUG
    std::cout << "Selected device: [" << device_index << "]\n";
#endif

    float queue_priority = 0.f;
    vk::DeviceQueueCreateInfo device_queue_create_info(
        vk::DeviceQueueCreateFlags(), compute_queue_family_index, 1,
        &queue_priority);
    // vk::PhysicalDeviceFeatures physical_device_features{};

    vk::DeviceCreateInfo device_create_info(
        vk::DeviceCreateFlags(), device_queue_create_info, {},
        device_extensions, {}  // &physical_device_features
    );
    vk::Device logical_device =
        capable_physical_device.createDevice(device_create_info);
    // vk::Device logical_device =
    //     capable_physical_device.createDevice(vk::DeviceCreateInfo(
    //         vk::DeviceCreateFlags(), device_queue_create_info));
    return logical_device;
}

}  // namespace crow
