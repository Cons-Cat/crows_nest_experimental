#pragma once
#include <GLFW/glfw3.h>
#include <stx/panic.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

struct App {
    // Basic things.
    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkPhysicalDeviceMemoryProperties memory_properties;
    VkDevice logical_device;
    VkFormat depth_format;
    VkCommandPool cmd_pool;

    uint32_t generic_queue_index;
    VkQueue generic_queue;

    // Window.
    bool prepared = false;
    bool resized = false;
    uint32_t const width = 1280;
    uint32_t const height = 720;
    GLFWwindow* window;
    VkSurfaceKHR surface;

    uint32_t image_count;
    VkSwapchainKHR swapchain;
    VkImage* swapchain_images;
    VkFormat swapchain_image_format;

    struct StorageImage {
        VkDeviceMemory memory;
        VkImage image;
        VkImageView view;
        VkFormat format;
    } storage_image;

    VkSemaphore* image_available_semaphores;
    VkSemaphore* render_finished_semaphores;
    VkFence* in_flight_fences;
    VkFence* images_in_flight;
    uint32_t current_frame;

    //  Raytracing
    struct Vertex {
        float pos[3];
    };
    Vertex vertices[3] = {
        {1.0f, 1.0f, 0.0f},
        {-1.0f, 1.0f, 0.0f},
        {0.0f, -1.0f, 0.0f},
    };
    VkBuffer vertex_position_buffer;
    VkDeviceMemory vertex_position_buffer_memory;

    uint32_t indices[3] = {
        0,
        1,
        2,
    };
    VkBuffer index_buffer;
    VkDeviceMemory index_buffer_memory;

    VkBuffer material_index_buffer;
    VkDeviceMemory material_index_buffer_memory;

    VkBuffer material_buffer;
    VkDeviceMemory material_buffer_memory;

    VkPhysicalDeviceRayTracingPipelinePropertiesKHR
        ray_tracing_pipeline_properties;
    VkPhysicalDeviceAccelerationStructureFeaturesKHR
        acceleration_structure_features;
    VkPhysicalDeviceBufferDeviceAddressFeatures
        enabled_buffer_device_addres_features;
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR
        enabled_ray_tracing_pipeline_features;
    VkPhysicalDeviceAccelerationStructureFeaturesKHR
        enabled_acceleration_structure_features;

    VkAccelerationStructureKHR blas;
    uint64_t blas_address;
    VkBuffer blas_structure_buffer;
    VkDeviceMemory blas_buffer_memory;

    VkAccelerationStructureKHR tlas;
    uint64_t tlas_address;
    VkBuffer tlas_buffer;
    VkDeviceMemory tlas_buffer_memory;

    VkImageView ray_trace_image_view;
    VkImage ray_trace_image;
    VkDeviceMemory ray_trace_image_memory;

    VkBuffer uniform_buffer;
    VkDeviceMemory uniform_buffer_memory;

    VkDescriptorPool descriptor_pool;
    VkDescriptorSet ray_trace_descriptor_set;
    VkDescriptorSet material_descriptor_set;
    VkDescriptorSetLayout* ray_trace_descriptor_set_layouts;

    VkPipeline ray_trace_pipeline;
    VkPipelineLayout ray_trace_pipeline_layout;

    VkBuffer shader_binding_table_buffer;
    VkDeviceMemory shader_binding_table_buffer_memory;

    VkCommandBuffer* command_buffers;

    // Because these represent Vulkan functions, I will leave them in camelCase
    // for now.
    PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR;  // NOLINT
    PFN_vkCreateAccelerationStructureKHR
        vkCreateAccelerationStructureKHR;  // NOLINT
    PFN_vkDestroyAccelerationStructureKHR
        vkDestroyAccelerationStructureKHR;  // NOLINT
    PFN_vkGetAccelerationStructureBuildSizesKHR
        vkGetAccelerationStructureBuildSizesKHR;  // NOLINT
    PFN_vkGetAccelerationStructureDeviceAddressKHR
        vkGetAccelerationStructureDeviceAddressKHR;  // NOLINT
    PFN_vkCmdBuildAccelerationStructuresKHR
        vkCmdBuildAccelerationStructuresKHR;  // NOLINT
    PFN_vkBuildAccelerationStructuresKHR
        vkBuildAccelerationStructuresKHR;     // NOLINT
    PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;  // NOLINT
    PFN_vkGetRayTracingShaderGroupHandlesKHR
        vkGetRayTracingShaderGroupHandlesKHR;  // NOLINT
    PFN_vkCreateRayTracingPipelinesKHR
        vkCreateRayTracingPipelinesKHR;  // NOLINT

    // Methods
    void initialize();
    void render_loop();
    void free();

  private:
    void create_surface();
    void create_physical_device();
    void create_logical_device();
    void create_swapchain();
    void create_cmd_pool();
    void create_vertex_buffer();
    void create_index_buffer();
    void create_material_buffer();
    void create_storage_image();
    void create_textures();
    void create_cmd_buffers();
    void load_every_pfn();
    void create_tlas();
    void create_blas();
};
