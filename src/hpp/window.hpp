#pragma once

#include <vulkan/vulkan.hpp>

#include <SDL_vulkan.h>

namespace crow {

struct game {
    void initialize();
    void loop();
    void destroy() const;

   private:
    // TODO: Move these to private and make getters.
    SDL_Window* p_window{};
    vk::Instance vk_instance;
    vk::Device logical_device;
    vk::SurfaceKHR surface;
    vk::PhysicalDeviceFeatures2 physical_features;
    uint32_t window_width;
    uint32_t window_height;
    vk::Extent2D window_extent;
    vk::SwapchainKHR swapchain;
    std::vector<vk::Image> swapchain_images;
    std::vector<vk::ImageView> image_views;
    std::vector<vk::Fence> swapchain_fences;
    std::vector<vk::Framebuffer> framebuffers;
    std::array<vk::ImageView, 1> attachments;
    vk::RenderPass render_pass;
    vk::CommandPool cmd_pool_compute;
    vk::CommandPool cmd_pool_rasterize;
    vk::CommandBuffer cmd_buffer_compute;
    vk::CommandBuffer cmd_buffer_rasterize;

    enum struct queue : uint32_t {
        compute,
        ui,
        COUNT,
    };
    std::array<vk::Queue, static_cast<size_t>(queue::COUNT)> vk_cmd_queues;

    void render();
    void record_clear_raster(vk::CommandBuffer* p_cmd_buffer);
};

}  // namespace crow
