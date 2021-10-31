#pragma once
#include <GLFW/glfw3.h>
#include <stx/panic.h>
#include <vulkan/vulkan.h>

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

    // More complex things.
    // VkPhysicalDeviceProperties device_properties;
    // VkPhysicalDeviceFeatures device_features;
    // VkPhysicalDeviceMemoryProperties device_memory_properties;
    // VkPhysicalDeviceFeatures enabled_features{};
    // std::vector<const char*> enabled_device_extensions;
    // std::vector<const char*> enabled_instance_extensions;
    // void* device_createp_next_chain = nullptr;

    //  Raytracing
    VkBuffer index_buffer;
    VkDeviceMemory index_buffer_memory;

    VkBuffer vertex_position_buffer;
    VkDeviceMemory vertex_position_buffer_memory;

    VkBuffer material_index_buffer;
    VkDeviceMemory material_index_buffer_memory;

    VkBuffer material_buffer;
    VkDeviceMemory material_buffer_memory;

    VkAccelerationStructureKHR bottom_level_acceleration_structure;
    VkBuffer bottom_level_acceleration_structure_buffer;
    VkDeviceMemory bottom_level_acceleration_structure_buffer_memory;

    VkAccelerationStructureKHR top_level_acceleration_structure;
    VkBuffer top_level_acceleration_structure_buffer;
    VkDeviceMemory top_level_acceleration_structure_buffer_memory;

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
};
