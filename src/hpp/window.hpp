#pragma once

#include <vulkan/vulkan.hpp>

#include <SDL_vulkan.h>

namespace crow {

struct game {
    SDL_Window* window{};
    vk::Instance vk_instance;
    vk::SurfaceKHR vk_surface;
    vk::PhysicalDeviceFeatures vk_features;

    void initialize();
    void loop();
    void destroy();
};

}  // namespace crow
