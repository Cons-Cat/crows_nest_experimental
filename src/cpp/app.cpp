#include "app.hpp"

#include <GLFW/glfw3.h>
#include <array>
#include <cstring>
#include <limits>
#include <vulkan/vulkan_core.h>

#include "memory.hpp"
#include "stx/panic.h"

static std::array<char, 500> key_down_index;

static void key_callback(GLFWwindow* /*p_window*/, int key, int /*scancode*/,
                         int action, int /*mods*/) {
    if (action == GLFW_PRESS) {
        key_down_index[key] = 1;
    }
    if (action == GLFW_RELEASE) {
        key_down_index[key] = 0;
    }
}

void App::create_surface() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    this->window =
        glfwCreateWindow(800, 600, "Raytracing Demo", nullptr, nullptr);
    glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(this->window, key_callback);

    uint32_t glfw_extension_count = 0;
    char const** p_glfw_extension_names =
        glfwGetRequiredInstanceExtensions(&glfw_extension_count);
    uint32_t const extension_count = glfw_extension_count + 1;
    char const** p_extension_names =
        new (std::nothrow) char const*[extension_count];
    for (uint32_t i = 0; i < glfw_extension_count; i++) {
        p_extension_names[i] = p_glfw_extension_names[i];
    }
    p_extension_names[glfw_extension_count] =
        "VK_KHR_get_physical_device_properties2";

    VkApplicationInfo application_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = VK_NULL_HANDLE,
        .pApplicationName = "Raytracing Demo",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_2,
    };

    constexpr uint32_t layer_count = 1;
    char const* layer_names[layer_count] = {
        "VK_LAYER_KHRONOS_validation",
    };

    VkInstanceCreateInfo instance_create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pApplicationInfo = &application_info,
        .enabledLayerCount = layer_count,
        .ppEnabledLayerNames = layer_names,
        .enabledExtensionCount = extension_count,
        .ppEnabledExtensionNames = p_extension_names,
    };

    if (vkCreateInstance(&instance_create_info, nullptr, &this->instance) !=
        VK_SUCCESS) {
        stx::panic("Failed to create instance!");
    }

    if (!glfwCreateWindowSurface(this->instance, this->window, nullptr,
                                 &this->surface)) {
    }

    delete[] p_extension_names;
}

void App::create_physical_device() {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(this->instance, &device_count, nullptr);
    VkPhysicalDevice* p_devices =
        new (std::nothrow) VkPhysicalDevice[device_count];
    vkEnumeratePhysicalDevices(this->instance, &device_count, p_devices);
    this->physical_device = p_devices[1];

    if (this->physical_device == VK_NULL_HANDLE) {
        stx::panic("Failed to select a physical device!");
    }

    vkGetPhysicalDeviceMemoryProperties(this->physical_device,
                                        &(this->memory_properties));

    delete[] p_devices;
}

void App::create_logical_device() {
    this->generic_queue_index = 0;

    VkBool32 is_present_supported = 0;
    vkGetPhysicalDeviceSurfaceSupportKHR(this->physical_device,
                                         this->generic_queue_index,
                                         this->surface, &is_present_supported);
    if (is_present_supported != VK_TRUE) {
        stx::panic("Surface presentation is not supported!");
    }

    constexpr uint32_t device_enabled_extension_count = 12;
    char const* device_enabled_extension_names[device_enabled_extension_count] =
        {
            "VK_KHR_swapchain",
            "VK_KHR_ray_tracing_pipeline",
            "VK_KHR_acceleration_structure",
            "VK_KHR_spirv_1_4",
            "VK_KHR_shader_float_controls",
            "VK_KHR_get_memory_requirements2",
            "VK_EXT_descriptor_indexing",
            "VK_KHR_buffer_device_address",
            "VK_KHR_deferred_host_operations",
            "VK_KHR_pipeline_library",
            "VK_KHR_maintenance3",
            "VK_KHR_maintenance1",
        };

    float const queue_priority = 1.0f;
    constexpr uint32_t device_queue_create_info_count = 1;
    // TODO: Move to struct?
    VkDeviceQueueCreateInfo
        device_queue_create_infos[device_queue_create_info_count] = {
            {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueFamilyIndex = this->generic_queue_index,
                .queueCount = 1,
                .pQueuePriorities = &queue_priority,
            },
        };

    this->buffer_device_address_features = {
        .sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT,
        .pNext = nullptr,
        .bufferDeviceAddress = VK_TRUE,
        .bufferDeviceAddressCaptureReplay = VK_FALSE,
        .bufferDeviceAddressMultiDevice = VK_FALSE,
    };

    this->ray_tracing_pipeline_features = {
        .sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,
        .pNext = &buffer_device_address_features,
        .rayTracingPipeline = VK_TRUE,
        .rayTracingPipelineShaderGroupHandleCaptureReplay = VK_FALSE,
        .rayTracingPipelineShaderGroupHandleCaptureReplayMixed = VK_FALSE,
        .rayTracingPipelineTraceRaysIndirect = VK_FALSE,
        .rayTraversalPrimitiveCulling = VK_FALSE,
    };

    // VkPhysicalDeviceAccelerationStructureFeaturesKHR
    this->acceleration_structure_features = {
        .sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
        .pNext = &ray_tracing_pipeline_features,
        .accelerationStructure = VK_TRUE,
        .accelerationStructureCaptureReplay = VK_TRUE,
        .accelerationStructureIndirectBuild = VK_FALSE,
        .accelerationStructureHostCommands = VK_FALSE,
        .descriptorBindingAccelerationStructureUpdateAfterBind = VK_FALSE,
    };

    VkDeviceCreateInfo device_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &acceleration_structure_features,
        .flags = 0,
        .queueCreateInfoCount = device_queue_create_info_count,
        .pQueueCreateInfos = device_queue_create_infos,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = device_enabled_extension_count,
        .ppEnabledExtensionNames = device_enabled_extension_names,
        .pEnabledFeatures = nullptr,
    };

    if (vkCreateDevice(this->physical_device, &device_create_info, nullptr,
                       &(this->logical_device)) != VK_SUCCESS) {
        stx::panic("Failed to create logical device!");
    }

    vkGetDeviceQueue(this->logical_device, this->generic_queue_index, 0,
                     &this->present_queue);
    vkGetDeviceQueue(this->logical_device, this->generic_queue_index, 0,
                     &this->graphics_queue);
    vkGetDeviceQueue(this->logical_device, this->generic_queue_index, 0,
                     &this->compute_queue);
}

void App::create_swapchain() {
    VkSurfaceCapabilitiesKHR surface_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        this->physical_device, this->surface, &surface_capabilities);

    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(this->physical_device, this->surface,
                                         &format_count, nullptr);
    VkSurfaceFormatKHR* p_surface_formats =
        new (std::nothrow) VkSurfaceFormatKHR[format_count];

    vkGetPhysicalDeviceSurfaceFormatsKHR(this->physical_device, this->surface,
                                         &format_count, p_surface_formats);

    uint32_t present_mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        this->physical_device, this->surface, &present_mode_count, nullptr);
    VkPresentModeKHR* p_surface_present_modes =
        new (std::nothrow) VkPresentModeKHR[present_mode_count];

    vkGetPhysicalDeviceSurfacePresentModesKHR(
        this->physical_device, this->surface, &present_mode_count,
        p_surface_present_modes);

    VkSurfaceFormatKHR surface_format = p_surface_formats[0];
    VkPresentModeKHR present_mode = p_surface_present_modes[0];
    VkExtent2D extent = surface_capabilities.currentExtent;

    this->image_count = surface_capabilities.minImageCount + 1;
    if (surface_capabilities.maxImageCount > 0 &&
        this->image_count > surface_capabilities.maxImageCount) {
        this->image_count = surface_capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchain_create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = this->surface,
        .minImageCount = this->image_count,
        // TODO: UNORM might be better than SRGB.
        .imageFormat = VK_FORMAT_B8G8R8A8_SRGB,
        .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                      VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    };

    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = surface_capabilities.currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = present_mode;
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.oldSwapchain = nullptr;

    if (vkCreateSwapchainKHR(this->logical_device, &swapchain_create_info,
                             nullptr, &this->swapchain) != VK_SUCCESS) {
        stx::panic("Failed to create a swapchain!");
    }

    vkGetSwapchainImagesKHR(this->logical_device, this->swapchain,
                            &this->image_count, nullptr);
    this->swapchain_images = new (std::nothrow) VkImage[image_count];
    vkGetSwapchainImagesKHR(this->logical_device, this->swapchain,
                            &this->image_count, this->swapchain_images);

    this->swapchain_image_format = surface_format.format;

    delete[] p_surface_formats;
    delete[] p_surface_present_modes;
}

void App::create_cmd_pool() {
    VkCommandPoolCreateInfo command_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = this->generic_queue_index,
    };

    if (vkCreateCommandPool(this->logical_device, &command_pool_create_info,
                            NULL, &this->cmd_pool) != VK_SUCCESS) {
        stx::panic("Failed to create a command pool!");
    }
}

void App::create_storage_image() {
    VkImageCreateInfo image = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = this->swapchain_image_format,
        .extent =
            {
                .width = this->width,
                .height = this->height,
                .depth = 1,
            },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    if (vkCreateImage(this->logical_device, &image, nullptr,
                      &this->storage_image.image) != VK_SUCCESS) {
        stx::panic("Failed to create image!");
    }

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(
        this->logical_device, this->storage_image.image, &memory_requirements);

    VkMemoryAllocateInfo memory_allocate_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex = find_memory_type(memory_requirements.memoryTypeBits,
                                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                            this->physical_device),
    };

    if (vkAllocateMemory(this->logical_device, &memory_allocate_info, nullptr,
                         &this->storage_image.memory) != VK_SUCCESS) {
        stx::panic("Faile to allocate image memory!");
    }

    if (vkBindImageMemory(this->logical_device, this->storage_image.image,
                          this->storage_image.memory, 0) != VK_SUCCESS) {
        stx::panic("Failed to bind image memory!");
    }

    VkImageViewCreateInfo color_image_view = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = this->storage_image.image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = this->swapchain_image_format,
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };

    if (vkCreateImageView(this->logical_device, &color_image_view, nullptr,
                          &this->storage_image.view) != VK_SUCCESS) {
        stx::panic("Failed to create an image view");
    }
}

void App::load_every_pfn() {
    vkGetBufferDeviceAddressKHR =
        reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(
            this->logical_device, "vkGetBufferDeviceAddressKHR"));
    vkCmdBuildAccelerationStructuresKHR =
        reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(
            vkGetDeviceProcAddr(this->logical_device,
                                "vkCmdBuildAccelerationStructuresKHR"));
    vkBuildAccelerationStructuresKHR =
        reinterpret_cast<PFN_vkBuildAccelerationStructuresKHR>(
            vkGetDeviceProcAddr(this->logical_device,
                                "vkBuildAccelerationStructuresKHR"));
    vkCreateAccelerationStructureKHR =
        reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(
            vkGetDeviceProcAddr(this->logical_device,
                                "vkCreateAccelerationStructureKHR"));
    vkDestroyAccelerationStructureKHR =
        reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(
            vkGetDeviceProcAddr(this->logical_device,
                                "vkDestroyAccelerationStructureKHR"));
    vkGetAccelerationStructureBuildSizesKHR =
        reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(
            vkGetDeviceProcAddr(this->logical_device,
                                "vkGetAccelerationStructureBuildSizesKHR"));
    vkGetAccelerationStructureDeviceAddressKHR =
        reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(
            vkGetDeviceProcAddr(this->logical_device,
                                "vkGetAccelerationStructureDeviceAddressKHR"));
    vkCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(
        vkGetDeviceProcAddr(this->logical_device, "vkCmdTraceRaysKHR"));
    vkGetRayTracingShaderGroupHandlesKHR =
        reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(
            vkGetDeviceProcAddr(this->logical_device,
                                "vkGetRayTracingShaderGroupHandlesKHR"));
    vkCreateRayTracingPipelinesKHR =
        reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(
            vkGetDeviceProcAddr(this->logical_device,
                                "vkCreateRayTracingPipelinesKHR"));
}

void App::create_vertex_buffer() {
    // TODO: Make this take a real mesh instead of a hard-coded triangle.
    VkDeviceSize position_buffer_size = sizeof(App::Vertex) * 3;

    VkBuffer p_position_staging_buffer;
    VkDeviceMemory p_position_staging_buffer_memory;

    create_buffer(this->logical_device, this->physical_device,
                  VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  position_buffer_size, p_position_staging_buffer,
                  &p_position_staging_buffer_memory);

    void* p_position_data;
    vkMapMemory(this->logical_device, p_position_staging_buffer_memory, 0,
                position_buffer_size, 0, &p_position_data);
    memcpy(p_position_data, &this->vertices, position_buffer_size);
    vkUnmapMemory(this->logical_device, p_position_staging_buffer_memory);

    create_buffer(
        this->logical_device, this->physical_device,
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, position_buffer_size,
        vertex_position_buffer, &this->vertex_position_buffer_memory);

    copy_buffer(this->logical_device, this->cmd_pool, p_position_staging_buffer,
                this->vertex_position_buffer, position_buffer_size,
                this->graphics_queue);

    vkDestroyBuffer(this->logical_device, p_position_staging_buffer, nullptr);
    vkFreeMemory(this->logical_device, p_position_staging_buffer_memory,
                 nullptr);
}

void App::create_index_buffer() {
    // TODO: Take arbitrary mesh instead of hard-coded triangle.
    uint32_t num_faces = 1;
    VkDeviceSize buffer_size = sizeof(uint32_t) * num_faces;
    uint32_t* p_position_indices = new (std::nothrow) uint32_t[3];
    p_position_indices[0] = 0;
    p_position_indices[1] = 1;
    p_position_indices[2] = 2;

    VkBuffer p_staging_buffer;
    VkDeviceMemory p_staging_buffer_memory;
    create_buffer(this->logical_device, this->physical_device,
                  VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  buffer_size, p_staging_buffer, &p_staging_buffer_memory);

    void* p_data;
    vkMapMemory(this->logical_device, p_staging_buffer_memory, 0, buffer_size,
                0, &p_data);
    memcpy(p_data, p_position_indices, buffer_size);
    vkUnmapMemory(this->logical_device, p_staging_buffer_memory);

    create_buffer(
        this->logical_device, this->physical_device,
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer_size, this->index_buffer,
        &this->index_buffer_memory);

    copy_buffer(this->logical_device, this->cmd_pool, p_staging_buffer,
                this->index_buffer, buffer_size, this->graphics_queue);

    vkDestroyBuffer(this->logical_device, p_staging_buffer, nullptr);
    vkFreeMemory(this->logical_device, p_staging_buffer_memory, nullptr);

    delete[] p_position_indices;
}

void App::create_blas() {
    // Setup identity transform matrix
    VkTransformMatrixKHR transform_matrix = {
        1.0f, 0.0f, 0.0f, 0.0f,  //
        0.0f, 1.0f, 0.0f, 0.0f,  //
        0.0f, 0.0f, 1.0f, 0.0f,  //
    };

    VkBufferDeviceAddressInfo vertex_buffer_device_address_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = this->vertex_position_buffer,
    };

    VkDeviceAddress vertex_buffer_address = vkGetBufferDeviceAddressKHR(
        this->logical_device, &vertex_buffer_device_address_info);

    VkDeviceOrHostAddressConstKHR vertex_device_or_host_address_const = {
        .deviceAddress = vertex_buffer_address,
    };

    VkBufferDeviceAddressInfo index_buffer_device_address_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = this->index_buffer,
    };

    VkDeviceAddress index_buffer_address = vkGetBufferDeviceAddressKHR(
        this->logical_device, &index_buffer_device_address_info);

    VkDeviceOrHostAddressConstKHR index_device_or_host_address_const = {
        .deviceAddress = index_buffer_address,
    };

    VkAccelerationStructureGeometryTrianglesDataKHR
        acceleration_structure_geometry_triangles_data = {
            .sType =
                VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
            .pNext = nullptr,
            .vertexFormat = VK_FORMAT_R32G32B32_SFLOAT,
            .vertexData = vertex_device_or_host_address_const,
            .vertexStride = sizeof(Vertex),
            .maxVertex = 3,  // TODO: Change this to mesh vertex count.
            .indexType = VK_INDEX_TYPE_UINT32,
            .indexData = index_device_or_host_address_const,
            .transformData = (VkDeviceOrHostAddressConstKHR){},
        };

    VkAccelerationStructureGeometryDataKHR
        acceleration_structure_geometry_data = {
            .triangles = acceleration_structure_geometry_triangles_data,
        };

    VkAccelerationStructureGeometryKHR acceleration_structure_geometry = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
        .pNext = nullptr,
        .geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
        .geometry = acceleration_structure_geometry_data,
        .flags = VK_GEOMETRY_OPAQUE_BIT_KHR,
    };

    VkAccelerationStructureBuildGeometryInfoKHR
        acceleration_structure_build_geometry_info = {
            .sType =
                VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
            .pNext = nullptr,
            .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
            .flags = 0,
            .mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
            .srcAccelerationStructure = VK_NULL_HANDLE,
            .dstAccelerationStructure = VK_NULL_HANDLE,
            .geometryCount = 1,
            .pGeometries = &acceleration_structure_geometry,
            .ppGeometries = nullptr,
            .scratchData = {},
        };

    VkAccelerationStructureBuildSizesInfoKHR
        acceleration_structure_build_sizes_info = {
            .sType =
                VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
            .pNext = nullptr,
            .accelerationStructureSize = 0,
            .updateScratchSize = 0,
            .buildScratchSize = 0,
        };

    vkGetAccelerationStructureBuildSizesKHR(
        this->logical_device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_KHR,
        &acceleration_structure_build_geometry_info,
        &acceleration_structure_build_geometry_info.geometryCount,
        &acceleration_structure_build_sizes_info);

    create_buffer(
        this->logical_device, this->physical_device,
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        acceleration_structure_build_sizes_info.accelerationStructureSize,
        this->blas_buffer, &this->blas_buffer_memory);

    VkBuffer p_scratch_buffer;
    VkDeviceMemory p_scratch_buffer_memory;
    create_buffer(
        this->logical_device, this->physical_device,
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        acceleration_structure_build_sizes_info.buildScratchSize,
        p_scratch_buffer, &p_scratch_buffer_memory);

    VkBufferDeviceAddressInfo scratch_buffer_device_address_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = p_scratch_buffer,
    };

    VkDeviceAddress scratch_buffer_address = vkGetBufferDeviceAddressKHR(
        this->logical_device, &scratch_buffer_device_address_info);

    VkDeviceOrHostAddressKHR scratch_device_or_host_address = {
        .deviceAddress = scratch_buffer_address,
    };

    acceleration_structure_build_geometry_info.scratchData =
        scratch_device_or_host_address;

    VkAccelerationStructureCreateInfoKHR acceleration_structure_create_info = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .createFlags = 0,
        .buffer = this->blas_buffer,
        .offset = 0,
        .size =
            acceleration_structure_build_sizes_info.accelerationStructureSize,
        .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
        .deviceAddress = 0,
    };

    vkCreateAccelerationStructureKHR(this->logical_device,
                                     &acceleration_structure_create_info,
                                     nullptr, &this->blas);

    acceleration_structure_build_geometry_info.dstAccelerationStructure =
        this->blas;

    const VkAccelerationStructureBuildRangeInfoKHR*
        p_acceleration_structure_build_range_info =
            new (std::nothrow) VkAccelerationStructureBuildRangeInfoKHR{
                .primitiveCount = 1,  // TODO: Number of faces.
                .primitiveOffset = 0,
                .firstVertex = 0,
                .transformOffset = 0,
            };
    const VkAccelerationStructureBuildRangeInfoKHR**
        p_acceleration_structure_build_range_infos =
            &p_acceleration_structure_build_range_info;

    VkCommandBufferAllocateInfo buffer_allocate_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = this->cmd_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkCommandBuffer p_commandBuffer;
    vkAllocateCommandBuffers(this->logical_device, &buffer_allocate_info,
                             &p_commandBuffer);

    VkCommandBufferBeginInfo command_buffer_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    if (vkBeginCommandBuffer(p_commandBuffer, &command_buffer_begin_info) !=
        VK_SUCCESS) {
        stx::panic("Failed to begin blas build command buffer!");
    }
    vkCmdBuildAccelerationStructuresKHR(
        p_commandBuffer, 1, &acceleration_structure_build_geometry_info,
        p_acceleration_structure_build_range_infos);
    if (vkEndCommandBuffer(p_commandBuffer) != VK_SUCCESS) {
        stx::panic("Failed to end blas build command buffer!");
    }

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &p_commandBuffer,
    };

    if (vkQueueSubmit(this->compute_queue, 1, &submit_info, nullptr) !=
        VK_SUCCESS) {
        stx::panic("Failed to submit blas build command queue!");
    }
    if (vkQueueWaitIdle(this->compute_queue) != VK_SUCCESS) {
        stx::panic("Failed to wait on blas build command queue!");
    }

    vkFreeCommandBuffers(this->logical_device, this->cmd_pool, 1,
                         &p_commandBuffer);

    vkDestroyBuffer(this->logical_device, p_scratch_buffer, nullptr);
    vkFreeMemory(this->logical_device, p_scratch_buffer_memory, nullptr);
    delete p_acceleration_structure_build_range_info;
}

void App::create_tlas() {
    VkTransformMatrixKHR transform_matrix = {
        1, 0, 0, 0,  //
        0, 1, 0, 0,  //
        0, 0, 1, 0,  //
    };

    VkAccelerationStructureDeviceAddressInfoKHR
        acceleration_structure_device_address_info = {};
    acceleration_structure_device_address_info.sType =
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
    acceleration_structure_device_address_info.accelerationStructure =
        this->blas;

    VkDeviceAddress acceleration_structure_device_address =
        vkGetAccelerationStructureDeviceAddressKHR(
            this->logical_device, &acceleration_structure_device_address_info);

    VkAccelerationStructureInstanceKHR geometry_instance = {};
    geometry_instance.transform = transform_matrix;
    geometry_instance.instanceCustomIndex = 0;
    geometry_instance.mask = 0xFF;
    geometry_instance.instanceShaderBindingTableRecordOffset = 0;
    geometry_instance.flags =
        VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
    geometry_instance.accelerationStructureReference =
        acceleration_structure_device_address;

    VkDeviceSize geometry_instance_buffer_size =
        sizeof(VkAccelerationStructureInstanceKHR);

    VkBuffer p_geometry_instance_staging_buffer;
    VkDeviceMemory p_geometry_instance_staging_buffer_memory;
    create_buffer(this->logical_device, this->physical_device,
                  VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  geometry_instance_buffer_size,
                  p_geometry_instance_staging_buffer,
                  &p_geometry_instance_staging_buffer_memory);

    void* p_geometry_instance_data;
    vkMapMemory(this->logical_device, p_geometry_instance_staging_buffer_memory,
                0, geometry_instance_buffer_size, 0, &p_geometry_instance_data);
    memcpy(p_geometry_instance_data, &geometry_instance,
           geometry_instance_buffer_size);
    vkUnmapMemory(this->logical_device,
                  p_geometry_instance_staging_buffer_memory);

    VkBuffer p_geometry_instance_buffer;
    VkDeviceMemory p_geometry_instance_buffer_memory;
    create_buffer(
        this->logical_device, this->physical_device,
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, geometry_instance_buffer_size,
        p_geometry_instance_buffer, &p_geometry_instance_buffer_memory);

    copy_buffer(this->logical_device, this->cmd_pool,
                p_geometry_instance_staging_buffer, p_geometry_instance_buffer,
                geometry_instance_buffer_size, this->graphics_queue);

    vkDestroyBuffer(this->logical_device, p_geometry_instance_staging_buffer,
                    nullptr);
    vkFreeMemory(this->logical_device,
                 p_geometry_instance_staging_buffer_memory, nullptr);

    VkBufferDeviceAddressInfo geometry_instance_buffer_device_address_info = {};
    geometry_instance_buffer_device_address_info.sType =
        VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    geometry_instance_buffer_device_address_info.buffer =
        p_geometry_instance_buffer;

    VkDeviceAddress geometry_instance_buffer_address =
        vkGetBufferDeviceAddressKHR(
            this->logical_device,
            &geometry_instance_buffer_device_address_info);

    VkDeviceOrHostAddressConstKHR
        geometry_instance_device_or_host_address_const = {
            .deviceAddress = geometry_instance_buffer_address,
        };

    VkAccelerationStructureGeometryInstancesDataKHR
        acceleration_structure_geometry_instances_data = {
            .sType =
                VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
            .pNext = nullptr,
            .arrayOfPointers = VK_FALSE,
            .data = geometry_instance_device_or_host_address_const,
        };

    VkAccelerationStructureGeometryDataKHR
        acceleration_structure_geometry_data = {
            .instances = acceleration_structure_geometry_instances_data,
        };

    VkAccelerationStructureGeometryKHR acceleration_structure_geometry = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
        .pNext = nullptr,
        .geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
        .geometry = acceleration_structure_geometry_data,
        .flags = VK_GEOMETRY_OPAQUE_BIT_KHR,
    };

    VkAccelerationStructureBuildGeometryInfoKHR
        acceleration_structure_build_geometry_info = {
            .sType =
                VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
            .pNext = nullptr,
            .type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
            .flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR,
            .mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
            .srcAccelerationStructure = nullptr,
            .dstAccelerationStructure = nullptr,
            .geometryCount = 1,
            .pGeometries = &acceleration_structure_geometry,
            .ppGeometries = nullptr,
            .scratchData = {},
        };

    VkAccelerationStructureBuildSizesInfoKHR
        acceleration_structure_build_sizes_info = {
            .sType =
                VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
            .pNext = nullptr,
            .accelerationStructureSize = 0,
            .updateScratchSize = 0,
            .buildScratchSize = 0,
        };

    vkGetAccelerationStructureBuildSizesKHR(
        this->logical_device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_KHR,
        &acceleration_structure_build_geometry_info,
        &acceleration_structure_build_geometry_info.geometryCount,
        &acceleration_structure_build_sizes_info);

    create_buffer(
        this->logical_device, this->physical_device,
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        acceleration_structure_build_sizes_info.accelerationStructureSize,
        this->tlas_buffer, &this->tlas_buffer_memory);

    VkBuffer p_scratch_buffer;
    VkDeviceMemory p_scratch_buffer_memory;
    create_buffer(
        this->logical_device, this->physical_device,
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        acceleration_structure_build_sizes_info.buildScratchSize,
        p_scratch_buffer, &p_scratch_buffer_memory);

    VkBufferDeviceAddressInfo scratch_buffer_device_address_info = {};
    scratch_buffer_device_address_info.sType =
        VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    scratch_buffer_device_address_info.buffer = p_scratch_buffer;

    VkDeviceAddress scratch_buffer_address = vkGetBufferDeviceAddressKHR(
        this->logical_device, &scratch_buffer_device_address_info);

    VkDeviceOrHostAddressKHR scratch_device_or_host_address = {};
    scratch_device_or_host_address.deviceAddress = scratch_buffer_address;

    acceleration_structure_build_geometry_info.scratchData =
        scratch_device_or_host_address;

    VkAccelerationStructureCreateInfoKHR acceleration_structure_create_info = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .createFlags = 0,
        .buffer = this->tlas_buffer,
        .offset = 0,
        .size =
            acceleration_structure_build_sizes_info.accelerationStructureSize,
        .type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
        .deviceAddress = 0,
    };

    vkCreateAccelerationStructureKHR(this->logical_device,
                                     &acceleration_structure_create_info,
                                     nullptr, &this->tlas);

    acceleration_structure_build_geometry_info.dstAccelerationStructure =
        this->tlas;

    const VkAccelerationStructureBuildRangeInfoKHR*
        p_acceleration_structure_build_range_info =
            new (std::nothrow) VkAccelerationStructureBuildRangeInfoKHR{
                .primitiveCount = 1,
                .primitiveOffset = 0,
                .firstVertex = 0,
                .transformOffset = 0,
            };
    const VkAccelerationStructureBuildRangeInfoKHR**
        p_acceleration_structure_build_range_infos =
            &p_acceleration_structure_build_range_info;

    VkCommandBufferAllocateInfo buffer_allocate_info = {};
    buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    buffer_allocate_info.commandPool = this->cmd_pool;
    buffer_allocate_info.commandBufferCount = 1;

    VkCommandBuffer p_command_buffer;
    vkAllocateCommandBuffers(this->logical_device, &buffer_allocate_info,
                             &p_command_buffer);

    VkCommandBufferBeginInfo command_buffer_begin_info = {};
    command_buffer_begin_info.sType =
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.flags =
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(p_command_buffer, &command_buffer_begin_info) !=
        VK_SUCCESS) {
        stx::panic("Failed to begin tlas build command buffer!");
    }
    vkCmdBuildAccelerationStructuresKHR(
        p_command_buffer, 1, &acceleration_structure_build_geometry_info,
        p_acceleration_structure_build_range_infos);
    if (vkEndCommandBuffer(p_command_buffer) != VK_SUCCESS) {
        stx::panic("Failed to end tlas build command buffer!");
    }

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &p_command_buffer;

    if (vkQueueSubmit(this->compute_queue, 1, &submit_info, nullptr) !=
        VK_SUCCESS) {
        stx::panic("Failed to submit tlas build command queue!");
    }
    if (vkQueueWaitIdle(this->compute_queue) != VK_SUCCESS) {
        stx::panic("Failed to wait on tlas build command queue!");
    }

    vkFreeCommandBuffers(this->logical_device, this->cmd_pool, 1,
                         &p_command_buffer);

    vkDestroyBuffer(this->logical_device, p_scratch_buffer, nullptr);
    vkFreeMemory(this->logical_device, p_scratch_buffer_memory, nullptr);
    delete p_acceleration_structure_build_range_info;
}

void App::initialize() {
    this->create_surface();
    this->create_physical_device();
    this->create_logical_device();
    this->load_every_pfn();

    this->create_swapchain();
    this->create_cmd_pool();
    this->create_storage_image();
    this->create_vertex_buffer();
    this->create_index_buffer();
    this->create_blas();
    this->create_tlas();
}

void App::render_loop() {
    while (glfwWindowShouldClose(this->window) == 0) {
        glfwPollEvents();
    }
}

void App::free() {
    // vkFreeCommandBuffers(this->logical_device, this->cmd_pool,
    //                      this->image_count, this->p_command_buffers);
    // delete this->p_command_buffers;

    // Free acceleration structure.
    vkDestroyAccelerationStructureKHR(this->logical_device, this->tlas,
                                      nullptr);
    vkDestroyBuffer(this->logical_device, this->tlas_buffer, nullptr);
    vkFreeMemory(this->logical_device, this->tlas_buffer_memory, nullptr);

    vkDestroyAccelerationStructureKHR(this->logical_device, this->blas,
                                      nullptr);
    vkDestroyBuffer(this->logical_device, this->blas_buffer, nullptr);
    vkFreeMemory(this->logical_device, this->blas_buffer_memory, nullptr);

    // Free mesh data.
    vkDestroyBuffer(this->logical_device, this->vertex_position_buffer,
                    nullptr);
    vkFreeMemory(this->logical_device, this->vertex_position_buffer_memory,
                 nullptr);
    vkDestroyBuffer(this->logical_device, this->index_buffer, nullptr);
    vkFreeMemory(this->logical_device, this->index_buffer_memory, nullptr);

    // Free swapchain.
    vkDestroyImageView(this->logical_device, this->storage_image.view, nullptr);
    vkDestroyImage(this->logical_device, this->storage_image.image, nullptr);
    vkFreeMemory(this->logical_device, this->storage_image.memory, nullptr);
    delete swapchain_images;

    // Free other things.
    vkDestroyCommandPool(this->logical_device, this->cmd_pool, nullptr);
    vkDestroySwapchainKHR(this->logical_device, this->swapchain, nullptr);
    vkDestroyDevice(this->logical_device, nullptr);

    vkDestroySurfaceKHR(this->instance, this->surface, nullptr);
    vkDestroyInstance(this->instance, nullptr);
    glfwDestroyWindow(this->window);
    glfwTerminate();
}
