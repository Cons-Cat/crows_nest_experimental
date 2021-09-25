#include "window.hpp"

#include <SDL.h>
#include <exception>
#include <iostream>
#include <stdexcept>

namespace crow {

void game::initialize() {
  try {
    window =
        SDL_CreateWindow(CMAKE_GAME_TITLE, SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_VULKAN);
    if (!window) {
      throw "Failed to create SDL2 window.";
    }
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
    this->vk_features = {
#ifdef DEBUG
        .robustBufferAccess = VK_TRUE,
        .vertexPipelineStoresAndAtomics = VK_TRUE,
        .fragmentStoresAndAtomics = VK_TRUE,
#endif
        .shaderSampledImageArrayDynamicIndexing = VK_TRUE,
        .shaderStorageBufferArrayDynamicIndexing = VK_TRUE,
    };
    VkPhysicalDeviceAccelerationStructureFeaturesKHR
        features_acceleration_structure = {
            .sType =
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
            .accelerationStructure = VK_TRUE,
            .descriptorBindingAccelerationStructureUpdateAfterBind = VK_TRUE,
        };
    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR
        features_buffer_device_address = {
            .sType =
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR,
            .bufferDeviceAddress = VK_TRUE,
        };

    VkPhysicalDeviceDescriptorIndexingFeaturesEXT features_descriptor_indexing = {
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

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR features_ray_tracing_pipeline = {
        .sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,
        .rayTracingPipeline = VK_TRUE,
        .rayTracingPipelineTraceRaysIndirect = VK_TRUE,
    };

    VkPhysicalDeviceScalarBlockLayoutFeaturesEXT features_scalar_block_layout =
        {
            .sType =
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES,
            .scalarBlockLayout = VK_TRUE,
        };

    features_acceleration_structure.pNext = &features_buffer_device_address;
    features_buffer_device_address.pNext = &features_descriptor_indexing;
    features_descriptor_indexing.pNext = &features_ray_tracing_pipeline;
    features_ray_tracing_pipeline.pNext = &features_scalar_block_layout;
    // TODO:
    // device_params.next = &features_acceleration_structure;

    vk::ApplicationInfo app_info(CMAKE_GAME_TITLE, 0, "", 0,
                                 VK_API_VERSION_1_2);
    std::vector<const char*> layer_names{
#ifdef DEBUG
        "VK_LAYER_LUNARG_standard_validation",
#endif
    };
    vk::InstanceCreateInfo info{};
    info.sType = vk::StructureType::eInstanceCreateInfo;
    info.pApplicationInfo = &app_info;
    info.enabledLayerCount = layer_names.size();
    info.ppEnabledLayerNames = layer_names.data();
    info.enabledExtensionCount = extension_names.size();
    info.ppEnabledExtensionNames = extension_names.data();
    this->vk_instance = vk::createInstance(info);

    SDL_Vulkan_CreateSurface(
        this->window, static_cast<VkInstance>(this->vk_instance),
        reinterpret_cast<VkSurfaceKHR*>(&this->vk_surface));
    if (!this->vk_surface) {
      throw "Failed to create an SDL2 Vulkan surface.";
    }
  } catch (std::exception& e) {
    // TODO: Set up fmt::
    std::cerr << e.what() << "\n";
    throw;
  }
}

void game::loop() {
  while (true) {
    SDL_Event event;
    if (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        break;
      }
    }
  }
}

// NOLINTNEXTLINE
void game::destroy() {
  this->vk_instance.destroy();
  SDL_DestroyWindow(this->window);
  SDL_Quit();
}

}  // namespace crow
