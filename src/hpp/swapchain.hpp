#pragma once
#include <vulkan/vulkan.hpp>

namespace crow {

struct vulkan_swapchain_format {
    vk::ColorSpaceKHR color_space;
    vk::Format color_format;
};

}  // namespace crow
