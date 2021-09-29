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

        // TODO: More precise device features query.
        // crow::make_vk_features();  // This mutates global::device_features
        // this->vk_features = global::device_features.features_basic;
        auto supported_features = vk_physical_device.getFeatures2<
            vk::PhysicalDeviceFeatures2,
            vk::PhysicalDeviceAccelerationStructureFeaturesKHR,
            vk::PhysicalDeviceBufferDeviceAddressFeaturesKHR,
            vk::PhysicalDeviceDescriptorIndexingFeaturesEXT,
            vk::PhysicalDeviceRayTracingPipelineFeaturesKHR,
            vk::PhysicalDeviceScalarBlockLayoutFeaturesEXT>();

        std::vector<const char*> device_extensions =
            crow::make_vk_device_extensions();
        auto const& [rasterization_queue_index, presentation_queue_index,
                     compute_queue_index] =
            crow::make_vk_queue_indices(&vk_physical_device, &this->vk_surface);
        this->vk_logical_device = crow::make_vk_logical_device(
            &vk_physical_device, &vk_instance, &this->vk_surface,
            device_extensions, rasterization_queue_index, compute_queue_index);
        // TODO: Prove that BGRA-SRGB is supported.
        vk::Format color_format = vk::Format::eB8G8R8A8Srgb;
        vk::Format depth_format = vk::Format::eD16Unorm;
        vk::SurfaceCapabilitiesKHR surface_capabilities =
            vk_physical_device.getSurfaceCapabilitiesKHR(this->vk_surface);
        // TODO: FIFO is guaranteed by the spec to be available. But mailbox may
        // be preferable if it is present.
        vk::PresentModeKHR present_mode = vk::PresentModeKHR::eFifo;
        // TODO: Extract creation of vk::Extent2D to a function.
        // This looks inefficient, but these data types are required by SDL2.
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
        this->window_extent = vk::Extent2D{static_cast<uint32_t>(width),
                                           static_cast<uint32_t>(height)};

        this->vk_swapchain = crow::make_vk_swapchain(
            this->vk_logical_device, vk_physical_device, this->vk_surface,
            window_extent, color_format, present_mode,
            rasterization_queue_index, presentation_queue_index);

        this->vk_swapchain_images =
            this->vk_logical_device.getSwapchainImagesKHR(this->vk_swapchain);
        this->vk_image_views = crow::make_image_views(
            &this->vk_logical_device, &this->vk_swapchain_images, color_format);
        this->vk_swapchain_fences = crow::make_swapchain_fences(
            &this->vk_logical_device, &this->vk_swapchain_images);

        this->vk_cmd_pool_compute = crow::make_command_pool(
            &this->vk_logical_device, compute_queue_index);
        this->vk_cmd_buffer_compute = crow::alloc_command_buffer(
            &this->vk_logical_device, &this->vk_cmd_pool_compute);
        this->vk_cmd_pool_rasterize = crow::make_command_pool(
            &this->vk_logical_device, rasterization_queue_index);
        this->vk_cmd_buffer_rasterize = crow::alloc_command_buffer(
            &this->vk_logical_device, &this->vk_cmd_pool_rasterize);

        this->render_pass = crow::make_render_pass(&this->vk_logical_device,
                                                   color_format, depth_format);
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
    for (auto const& fence : this->vk_swapchain_fences) {
        this->vk_logical_device.destroy(fence);
    }
    this->vk_logical_device.destroy(this->vk_cmd_pool_compute);
    for (auto const& image_view : this->vk_image_views) {
        this->vk_logical_device.destroyImageView(image_view);
    }
    this->vk_logical_device.destroySwapchainKHR();
    this->vk_logical_device.destroyRenderPass(this->render_pass);
    this->vk_logical_device.destroy();
    this->vk_instance.destroySurfaceKHR(this->vk_surface);
    this->vk_instance.destroy();
    SDL_DestroyWindow(this->p_window);
    SDL_Quit();
}

void game::record_clear_raster(vk::CommandBuffer* p_cmd_buffer) {
}

}  // namespace crow
