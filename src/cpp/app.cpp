#include "app.hpp"

#include <GLFW/glfw3.h>
#include <array>
#include <limits>
#include <vulkan/vulkan_core.h>

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
        stx::panic("Failed to select a physical device.");
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
    if (is_present_supported == VK_FALSE) {
        stx::panic("Surface presentation is not supported.");
    }

    constexpr uint32_t device_enabled_extension_count = 12;
    char const* device_enabled_extension_names[device_enabled_extension_count] =
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
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

    VkPhysicalDeviceBufferDeviceAddressFeaturesEXT
        buffer_device_address_features = {
            .sType =
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT,
            .pNext = nullptr,
            .bufferDeviceAddress = VK_TRUE,
            .bufferDeviceAddressCaptureReplay = VK_FALSE,
            .bufferDeviceAddressMultiDevice = VK_FALSE,
        };

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing_pipeline_features = {
        .sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,
        .pNext = &buffer_device_address_features,
        .rayTracingPipeline = VK_TRUE,
        .rayTracingPipelineShaderGroupHandleCaptureReplay = VK_FALSE,
        .rayTracingPipelineShaderGroupHandleCaptureReplayMixed = VK_FALSE,
        .rayTracingPipelineTraceRaysIndirect = VK_FALSE,
        .rayTraversalPrimitiveCulling = VK_FALSE,
    };

    VkPhysicalDeviceAccelerationStructureFeaturesKHR
        acceleration_structure_features = {
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
                     &(this->generic_queue));
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
        .imageFormat = surface_format.format,
        .imageColorSpace = surface_format.colorSpace,
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
    swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(this->logical_device, &swapchain_create_info,
                             nullptr, &this->swapchain) != VK_SUCCESS) {
        stx::panic("Failed to create a swapchain.");
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

void App::initialize() {
    this->create_surface();
    this->create_physical_device();
    this->create_logical_device();
    this->create_swapchain();
    this->render_loop();
}

void App::render_loop() {
    while (glfwWindowShouldClose(this->window) == 0) {
        glfwPollEvents();
    }
}

void App::free() {
    delete swapchain_images;
    vkDestroySwapchainKHR(this->logical_device, this->swapchain, nullptr);
    vkDestroyDevice(this->logical_device, nullptr);
    vkDestroySurfaceKHR(this->instance, this->surface, nullptr);
    vkDestroyInstance(this->instance, nullptr);
    glfwDestroyWindow(this->window);
    glfwTerminate();
}
