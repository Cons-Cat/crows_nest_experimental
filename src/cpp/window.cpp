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
        this->surface = vk::SurfaceKHR(p_temp_surface);
        this->physical_device = crow::find_physical_device(&this->vk_instance);

        // TODO: More precise device features query.
        // crow::make_vk_features();  // This mutates global::device_features
        // this->vk_features = global::device_features.features_basic;
        auto supported_features = this->physical_device.getFeatures2<
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
            crow::make_vk_queue_indices(&this->physical_device, &this->surface);
        this->logical_device = crow::make_vk_logical_device(
            &this->physical_device, &vk_instance, &this->surface,
            device_extensions, rasterization_queue_index, compute_queue_index);
        // TODO: Prove that BGRA-SRGB is supported.
        vk::Format color_format = vk::Format::eB8G8R8A8Srgb;
        vk::Format depth_format = vk::Format::eD16Unorm;
        vk::SurfaceCapabilitiesKHR surface_capabilities =
            this->physical_device.getSurfaceCapabilitiesKHR(this->surface);
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

        this->swapchain = crow::make_vk_swapchain(
            this->logical_device, this->physical_device, this->surface,
            window_extent, color_format, present_mode,
            rasterization_queue_index, presentation_queue_index);

        this->swapchain_images =
            this->logical_device.getSwapchainImagesKHR(this->swapchain);
        this->swapchain_image_views = crow::make_image_views(
            &this->logical_device, &this->swapchain_images, color_format);
        this->swapchain_fences = crow::make_swapchain_fences(
            &this->logical_device, &this->swapchain_images);

        this->cmd_pool_compute =
            crow::make_cmd_pool(&this->logical_device, compute_queue_index);
        this->cmd_buffers_compute = crow::alloc_cmd_buffers(
            &this->logical_device, this->swapchain_images.size(),
            &this->cmd_pool_compute);
        this->cmd_pool_rasterize = crow::make_cmd_pool(
            &this->logical_device, rasterization_queue_index);
        this->cmd_buffers_rasterize = crow::alloc_cmd_buffers(
            &this->logical_device, this->swapchain_images.size(),
            &this->cmd_pool_rasterize);

        this->render_pass = crow::make_render_pass(&this->logical_device,
                                                   color_format, depth_format);
        this->framebuffers = crow::make_framebuffers(
            &this->logical_device, &this->render_pass, window_extent,
            &this->swapchain_image_views, &this->attachments);
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

    // uint32_t temp_buffer_data = 8;
    // vk::Buffer temp_buffer = this->logical_device.createBuffer(
    //     vk::BufferCreateInfo(vk::BufferCreateFlags(),
    //     sizeof(temp_buffer_data),
    //                          vk::BufferUsageFlagBits::eStorageBuffer));
    // vk::MemoryRequirements memory_requirements =
    //     this->logical_device.getBufferMemoryRequirements(temp_buffer);
    // /*
    //  There are several types of memory that can be allocated, and we must
    //  choose a memory type that:

    //  1) Satisfies the memory requirements(memoryRequirements.memoryTypeBits).

    //  2) Satifies our own usage requirements. We want to be able to read the
    //  buffer memory from the GPU to the CPU with vkMapMemory, so we set
    //  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT.

    //  Also, by setting VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memory written by
    //  the device(GPU) will be easily visible to the host(CPU), without having
    //  to call any extra flushing commands. So mainly for convenience, we set
    //  this flag.
    //  */
    // uint32_t memory_type_index =
    //     crow::find_memory_type(this->physical_device.getMemoryProperties(),
    //                            memory_requirements.memoryTypeBits,
    //                            vk::MemoryPropertyFlagBits::eHostVisible |
    //                                vk::MemoryPropertyFlagBits::eHostCoherent);
    // vk::DeviceMemory device_memory = this->logical_device.allocateMemory(
    //     vk::MemoryAllocateInfo(memory_requirements.size, memory_type_index));
    // uint8_t* p_data = static_cast<uint8_t*>(this->logical_device.mapMemory(
    //     device_memory, 0, memory_requirements.size));
    // memcpy(p_data, &temp_buffer_data, sizeof(temp_buffer_data));
    // this->logical_device.unmapMemory(device_memory);
    // this->logical_device.bindBufferMemory(temp_buffer, device_memory, 0);

    this->render();
}

void game::destroy() const {
    for (auto const& fence : this->swapchain_fences) {
        this->logical_device.destroyFence(fence);
    }

    // Destroying command pools also destroys the command buffers they
    // allocated.
    this->logical_device.destroyCommandPool(this->cmd_pool_compute);
    this->logical_device.destroyCommandPool(this->cmd_pool_rasterize);

    for (auto const& image_view : this->swapchain_image_views) {
        this->logical_device.destroyImageView(image_view);
    }
    for (auto const& framebuffer : this->framebuffers) {
        this->logical_device.destroyFramebuffer(framebuffer);
    }
    this->logical_device.destroyRenderPass(this->render_pass);
    this->logical_device.destroySwapchainKHR();
    this->logical_device.destroy();
    this->vk_instance.destroySurfaceKHR(this->surface);
    this->vk_instance.destroy();
    SDL_DestroyWindow(this->p_window);
    SDL_Quit();
}

void game::record_clear_raster(vk::CommandBuffer* p_cmd_buffer) {
}

}  // namespace crow
