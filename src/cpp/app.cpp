#include "app.hpp"

#include <array>
#include <limits>

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
    constexpr uint32_t sentinel_queue =
        -1;  // std::numeric_limits<uint32_t>::max();
    this->graphics_queue_index = sentinel_queue;
    this->present_queue_index = sentinel_queue;
    this->compute_queue_index = sentinel_queue;

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

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(this->physical_device,
                                             &queue_family_count, nullptr);
    VkQueueFamilyProperties* p_queue_family_properties =
        static_cast<VkQueueFamilyProperties*>(
            malloc(sizeof(VkQueueFamilyProperties) * queue_family_count));
    vkGetPhysicalDeviceQueueFamilyProperties(
        this->physical_device, &queue_family_count, p_queue_family_properties);

    for (int x = 0; x < static_cast<int>(queue_family_count); x++) {
        if (this->graphics_queue_index == sentinel_queue &&
            p_queue_family_properties[x].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            this->graphics_queue_index = x;
        }

        if (this->compute_queue_index == sentinel_queue &&
            p_queue_family_properties[x].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            this->compute_queue_index = x;
        }

        VkBool32 is_present_supported = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(
            this->physical_device, x, this->surface, &is_present_supported);

        if (this->present_queue_index == sentinel_queue &&
            is_present_supported) {
            this->present_queue_index = x;
        }

        if (this->graphics_queue_index != sentinel_queue &&
            this->present_queue_index != sentinel_queue &&
            this->compute_queue_index != sentinel_queue) {
            break;
        }
    }

    float const queue_priority = 1.0f;
    constexpr uint32_t device_queue_create_info_count = 3;
    VkDeviceQueueCreateInfo
        device_queue_create_infos[device_queue_create_info_count] = {
            {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueFamilyIndex = this->graphics_queue_index,
                .queueCount = 1,
                .pQueuePriorities = &queue_priority,
            },
            {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueFamilyIndex = this->present_queue_index,
                .queueCount = 1,
                .pQueuePriorities = &queue_priority,
            },
            {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueFamilyIndex = this->compute_queue_index,
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

    vkGetDeviceQueue(this->logical_device, this->graphics_queue_index, 0,
                     &(this->graphics_queue));
    vkGetDeviceQueue(this->logical_device, this->present_queue_index, 0,
                     &(this->present_queue));
    vkGetDeviceQueue(this->logical_device, this->compute_queue_index, 0,
                     &(this->compute_queue));
}

void App::initialize() {
    this->create_surface();
    this->create_physical_device();
    this->create_logical_device();
    this->render_loop();
}

void App::render_loop() {
    while (glfwWindowShouldClose(this->window) == 0) {
        glfwPollEvents();
    }
}
