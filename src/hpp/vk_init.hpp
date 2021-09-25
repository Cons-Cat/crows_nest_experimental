#pragma once
#include <vulkan/vulkan.hpp>

#include <SDL.h>
#include <SDL_vulkan.h>

#include "vk_globals.hpp"

namespace crow {

inline auto make_vk_extensions(SDL_Window* window) -> std::vector<const char*> {
    uint32_t extension_count = 0;
    SDL_Vulkan_GetInstanceExtensions(window, &extension_count, nullptr);
    std::vector<const char*> extension_names(extension_count);
    SDL_Vulkan_GetInstanceExtensions(window, &extension_count,
                                     extension_names.data());
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
#ifdef DEBUG
        "VK_LAYER_LUNARG_standard_validation",
#endif
    };
    return layer_names;
}

inline auto make_vk_create_info(SDL_Window* window) -> vk::InstanceCreateInfo {
    std::vector<const char*> extension_names = crow::make_vk_extensions(window);
    std::vector<const char*> layer_names = crow::make_vk_layer_names();

    vk::ApplicationInfo app_info(CMAKE_GAME_TITLE, 0,
                                 "2108_GDBS_LogicVisions_GameEngine", 0,
                                 VK_API_VERSION_1_2);

    vk::InstanceCreateInfo info{};
    info.sType = vk::StructureType::eInstanceCreateInfo;
    info.pApplicationInfo = &app_info;
    info.enabledLayerCount = layer_names.size();
    info.ppEnabledLayerNames = layer_names.data();
    info.enabledExtensionCount = extension_names.size();
    info.ppEnabledExtensionNames = extension_names.data();
    return info;
}

}  // namespace crow
