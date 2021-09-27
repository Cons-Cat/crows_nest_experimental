#include "window.hpp"

#include <vulkan/vulkan.hpp>

#include <SDL.h>
#include <SDL_vulkan.h>
#include <algorithm>
#include <exception>
#include <iostream>
#include <stdexcept>

#include "render_loop.hpp"
#include "vk_globals.hpp"
#include "vk_init.hpp"

namespace crow {

void game::initialize() {
    try {
        p_window = SDL_CreateWindow(CMAKE_GAME_TITLE, SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED, 1280, 720,
                                    SDL_WINDOW_VULKAN);
        if (p_window == nullptr) {
            throw std::runtime_error(SDL_GetError());
        }
        // crow::make_vk_features();  // This mutates global::device_features
        // this->vk_features = global::device_features.features_basic;
        vk::ApplicationInfo app_info(CMAKE_GAME_TITLE, 0,
                                     "2108_GDBS_LogicVisions_GameEngine", 0,
                                     VK_API_VERSION_1_2);
        std::vector<char const*> instance_extensions =
            crow::make_vk_instance_extensions(p_window);
        std::vector<char const*> instance_layers = crow::make_vk_layer_names();
        vk::InstanceCreateInfo instance_info = crow::make_vk_instance_info(
            &instance_extensions, &instance_layers, &app_info);
        this->vk_instance = vk::createInstance(instance_info);

        VkSurfaceKHR p_temp_surface = nullptr;
        if (SDL_Vulkan_CreateSurface(this->p_window,
                                     static_cast<VkInstance>(this->vk_instance),
                                     &p_temp_surface) == SDL_FALSE) {
            throw std::runtime_error(SDL_GetError());
        }
        this->vk_surface = vk::SurfaceKHR(p_temp_surface);
        vk::PhysicalDevice vk_physical_device =
            crow::find_vk_physical_device(&this->vk_instance);
        std::vector<const char*> device_extensions =
            crow::make_vk_device_extensions();
        auto const& [rasterization_queue_index, presentation_queue_index,
                     compute_queue_index] =
            crow::make_vk_queue_indices(&vk_physical_device, &this->vk_surface);
        this->vk_logical_device = crow::make_vk_logical_device(
            &vk_physical_device, &vk_instance, &this->vk_surface,
            device_extensions, rasterization_queue_index, compute_queue_index);
        // TODO: Prove that BGRA-SRGB is supported.
        vk::Format format = vk::Format::eB8G8R8A8Srgb;
        vk::SurfaceCapabilitiesKHR surface_capabilities =
            vk_physical_device.getSurfaceCapabilitiesKHR(this->vk_surface);
        // TODO: FIFO is guaranteed by the spec to be available. But mailbox may
        // be preferable if it is present.
        vk::PresentModeKHR present_mode = vk::PresentModeKHR::eFifo;
        // TODO: Extract vk::Extent2D to a function.
        // This looks inefficient, but these data types are required by SDL2.
        vk::Extent2D swapchain_extent;
        int width = 0;
        int height = 0;
        SDL_Vulkan_GetDrawableSize(p_window, &width, &height);
        width = std::clamp(
            width, static_cast<int>(surface_capabilities.minImageExtent.width),
            static_cast<int>(surface_capabilities.maxImageExtent.width));
        height = std::clamp(
            height,
            static_cast<int>(surface_capabilities.minImageExtent.height),
            static_cast<int>(surface_capabilities.maxImageExtent.height));
        vk::Extent2D window_extent = {static_cast<uint32_t>(width),
                                      static_cast<uint32_t>(height)};

        vk::SwapchainKHR swapchain = crow::make_vk_swapchain(
            this->vk_logical_device, vk_physical_device, this->vk_surface,
            window_extent, format, present_mode, rasterization_queue_index,
            presentation_queue_index);
    } catch (std::exception& e) {
        // TODO: Set up fmt::
        std::cerr << e.what() << "\n";
        throw;
    }
}

// NOLINTNEXTLINE Remove when this function is clearly not static.
void game::loop() {
    while (true) {
        SDL_Event event;
        if (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                break;
            }
        }
    }
    crow::render();
}

void game::destroy() const {
    this->vk_logical_device.destroy();
    this->vk_instance.destroySurfaceKHR(this->vk_surface);
    this->vk_instance.destroy();
    SDL_DestroyWindow(this->p_window);
    SDL_Quit();
}

}  // namespace crow
