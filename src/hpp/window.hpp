#pragma once

#include <vulkan/vulkan.hpp>

#include <SDL_vulkan.h>

namespace crow {

struct game {
    // TODO: Move these to private and make getters.
    SDL_Window* p_window{};
    vk::Instance vk_instance;
    vk::Device vk_logical_device;
    vk::SurfaceKHR vk_surface;
    vk::PhysicalDeviceFeatures2 vk_features;
    vk::SwapchainKHR vk_swapchain;
    std::vector<vk::ImageView> vk_image_views;

    void initialize();
    void loop();
    void destroy() const;

   private:
    uint32_t window_width;
    uint32_t window_height;
};

}  // namespace crow
