#pragma once
#include <vulkan/vulkan.hpp>

#include <SDL.h>
#include <SDL_vulkan.h>
#include <iostream>
#include <optional>
#include <string>

#include "vk_globals.hpp"

namespace crow {

// TODO: Move these into `vk_init.cpp`

inline auto make_vk_instance_extensions(SDL_Window* p_window)
    -> std::vector<char const*> {
    uint32_t extension_count = 0;
    SDL_Vulkan_GetInstanceExtensions(p_window, &extension_count, nullptr);
    std::vector<char const*> extension_names(extension_count);
    SDL_Vulkan_GetInstanceExtensions(p_window, &extension_count,
                                     extension_names.data());
    extension_names.push_back("VK_KHR_surface");
    return extension_names;
}

inline auto make_vk_features() {
}

inline auto make_vk_layer_names() -> std::vector<char const*> {
    std::vector<const char*> layer_names{
#ifdef CMAKE_DEBUG
        "VK_LAYER_KHRONOS_validation",
#endif
    };
    return layer_names;
}

inline auto make_vk_instance_info(
    std::vector<char const*> const* p_extension_names,
    std::vector<char const*> const* p_layer_names,
    vk::ApplicationInfo const* p_app_info) -> vk::InstanceCreateInfo {
    vk::InstanceCreateInfo instance_info{};
    instance_info.sType = vk::StructureType::eInstanceCreateInfo;
    instance_info.pApplicationInfo = p_app_info;
    instance_info.enabledLayerCount = p_layer_names->size();
    instance_info.ppEnabledLayerNames = p_layer_names->data();
    instance_info.enabledExtensionCount = p_extension_names->size();
    instance_info.ppEnabledExtensionNames = p_extension_names->data();
    return instance_info;
}

inline auto is_device_capable(vk::PhysicalDevice device)
    -> std::optional<size_t> {
    vk::PhysicalDeviceProperties2 device_properties = device.getProperties2();

    // TODO: This might not be reliable.
    // Find a video card that can support raytracing and blitting.
    std::vector<vk::QueueFamilyProperties> queue_family_properties =
        device.getQueueFamilyProperties();
    auto property_iterator = std::find_if(
        queue_family_properties.begin(), queue_family_properties.end(),
        [](vk::QueueFamilyProperties const& qfp) {
            return qfp.queueFlags & vk::QueueFlagBits::eCompute;
        });
    size_t compute_queue_family_index =
        std::distance(queue_family_properties.begin(), property_iterator);
    if (compute_queue_family_index < queue_family_properties.size()) {
        return compute_queue_family_index;
    }
    return std::nullopt;
}

inline void get_vk_features() {
    // TODO: Get the features needed for rendering this game, to verify that a
    // given physical device is viable.
}

inline auto make_vk_device_extensions() -> std::vector<char const*> {
    std::vector<char const*> extension_names{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
        VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
        VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
        VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
        VK_KHR_SPIRV_1_4_EXTENSION_NAME,
        VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
        VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME,
    };
    return extension_names;
}

inline auto find_vk_physical_device(vk::Instance* p_instance)
    -> vk::PhysicalDevice {
    // TODO: Check all video cards and find ideal one.
    vk::PhysicalDevice capable_physical_device = VK_NULL_HANDLE;
    std::vector<vk::PhysicalDevice> physical_devices =
        p_instance->enumeratePhysicalDevices();
    if (physical_devices.empty()) {
        throw std::runtime_error("Failed to find a Vulkan-capable GPU.");
    }
    size_t compute_queue_family_index = 0;
    uint32_t device_index = 0;

    for (int i = 0; i < physical_devices.size(); i++) {
        auto device = physical_devices[i];
        device_index = i;
        std::optional<size_t> maybe_index = is_device_capable(device);
        if (maybe_index.has_value()) {
            compute_queue_family_index = maybe_index.value();
            capable_physical_device = device;
            break;
        }
    }
    if (!capable_physical_device) {
        throw std::runtime_error("Failed to find a GPU for raytracing.");
    }
#ifdef CMAKE_DEBUG
    std::cout << "Selected device: [" << device_index << "]\n";
#endif
    return capable_physical_device;
}

inline auto find_queue_family_index(
    std::vector<vk::QueueFamilyProperties> const& queue_family_properties,
    vk::QueueFlagBits queue_flag) -> uint32_t {
    for (int i = 0; i < queue_family_properties.size(); i++) {
        auto qfp = queue_family_properties[i];
        if (qfp.queueFlags & queue_flag) {
            return i;
        }
    }
    throw std::runtime_error("Failed to find a queue family that supports " +
                             to_string(queue_flag));
    // TODO: On my video card, and many others, there are two compute queue
    // families. One is consolidated into the graphics queue family, and another
    // is dedicated to compute. It would be best to utilize the dedicated
    // compute queue for compute kernels, if possible on a given video card, so
    // that game UI can render in parallel to raytracing.
}

inline auto make_vk_queue_indices(vk::PhysicalDevice* p_device,
                                  vk::SurfaceKHR* p_surface)
    -> std::tuple<uint32_t,  // Graphics queue family index.
                  uint32_t,  // Presentation queue family index.
                  uint32_t>  // Compute queue family index.
{
    std::vector<vk::QueueFamilyProperties> queue_family_properties =
        p_device->getQueueFamilyProperties();
#ifdef CMAKE_DEBUG
    for (auto& queue_family : queue_family_properties) {
        std::cout << "Queue number: " + std::to_string(queue_family.queueCount)
                  << '\n';
        std::cout << "Queue flags: " + to_string(queue_family.queueFlags)
                  << '\n';
    }
#endif

    auto rasterization_queue_family_index = find_queue_family_index(
        queue_family_properties, vk::QueueFlagBits::eGraphics);
    auto compute_queue_family_index = find_queue_family_index(
        queue_family_properties, vk::QueueFlagBits::eCompute);
#ifdef CMAKE_DEBUG
    std::cout << "Found rasterization queue family "
                 "index: "
              << rasterization_queue_family_index << '\n';
    std::cout << "Found compute queue family index: "
              << compute_queue_family_index << '\n';
#endif

    uint32_t present_queue_family_index =
        p_device->getSurfaceSupportKHR(rasterization_queue_family_index,
                                       *p_surface) != 0
            ? rasterization_queue_family_index
            : queue_family_properties.size();

    // TODO: A rasterization queue is not guaranteed to support presentation.
    // Some video cards have multiple rasterization queues, so selecting one
    // that does support presentation is necessary.
    // TODO: This should be extracted to a separate function.
    /*    if (present_queue_family_index == queue_family_properties.size()) {
            for (size_t i = 0; i < queue_family_properties.size(); i++) {
                if ((queue_family_properties[i].queueFlags &
                     vk::QueueFlagBits::eGraphics) &&
                    (p_device->getSurfaceSupportKHR(static_cast<uint32_t>(i),
                                                    *p_surface) != 0)) {
                    rasterization_queue_family_index = i;
                    present_queue_family_index = i;
                    break;
                }
            }
            if (present_queue_family_index == queue_family_properties.size()) {
                for (size_t i = 0; i < queue_family_properties.size(); i++) {
                    if (p_device->getSurfaceSupportKHR(static_cast<uint32_t>(i),
                                                       *p_surface) != 0) {
                        present_queue_family_index = i;
                        break;
                    }
                }
            }
        }
        if ((rasterization_queue_family_index == queue_family_properties.size())
       || (present_queue_family_index == queue_family_properties.size())) {
            throw std::runtime_error(
                "Could not find a queue for graphics or present ->
       terminating");
        }
    */

    return std::make_tuple(present_queue_family_index,
                           present_queue_family_index,
                           compute_queue_family_index);
}

inline auto make_vk_logical_device(vk::PhysicalDevice* p_physical_device,
                                   vk::Instance* /*p_instance*/,
                                   vk::SurfaceKHR* /*p_surface*/,
                                   std::vector<char const*>& device_extensions,
                                   uint32_t compute_queue_family_index,
                                   uint32_t rasterization_queue_family_index)
    -> vk::Device {
    float rasterization_queue_priority = 1.f;
    float compute_queue_priority = 1.f;
    vk::DeviceQueueCreateInfo device_rasterization_queue_create_info(
        vk::DeviceQueueCreateFlags(), rasterization_queue_family_index, 1,
        &rasterization_queue_priority);
    vk::DeviceQueueCreateInfo device_compute_queue_create_info(
        vk::DeviceQueueCreateFlags(), compute_queue_family_index, 1,
        &compute_queue_priority);
    // vk::PhysicalDeviceFeatures physical_device_features{};
    std::vector<vk::DeviceQueueCreateInfo> device_queues_infos = {
        device_rasterization_queue_create_info,
        device_compute_queue_create_info};

    vk::DeviceCreateInfo device_create_info(
        vk::DeviceCreateFlags(), device_queues_infos, {}, device_extensions, {}
        // &physical_device_features
    );

    vk::Device logical_device =
        p_physical_device->createDevice(device_create_info);

    return logical_device;
}

// TODO: Pass by pointer?
inline auto make_vk_swapchain(
    vk::Device& logical_device, vk::PhysicalDevice& physical_device,
    vk::SurfaceKHR& surface, vk::Extent2D& extent, vk::Format& format,
    vk::PresentModeKHR& present_mode, uint32_t rasterization_queue_family_index,
    uint32_t present_queue_family_index) -> vk::SwapchainKHR {
    vk::SurfaceCapabilitiesKHR surface_capabilities =
        physical_device.getSurfaceCapabilitiesKHR(surface);
    uint32_t frame_count = std::clamp(std::numeric_limits<uint32_t>::max(),
                                      surface_capabilities.minImageCount,
                                      surface_capabilities.maxImageCount);

    vk::SurfaceTransformFlagBitsKHR pre_transform =
        surface_capabilities.currentTransform;
    // TODO: Look into this more for optimizations.
    vk::CompositeAlphaFlagBitsKHR composite_alpha =
        vk::CompositeAlphaFlagBitsKHR::eOpaque;

    vk::SwapchainCreateInfoKHR swapchain_create_info(
        vk::SwapchainCreateFlagsKHR(), surface, frame_count, format,
        vk::ColorSpaceKHR::eSrgbNonlinear, extent, 1u,
        vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive,
        {}, pre_transform, composite_alpha, present_mode, VK_TRUE,
        VK_NULL_HANDLE);

    std::array<uint32_t, 2> queue_family_indices = {
        static_cast<uint32_t>(rasterization_queue_family_index),
        static_cast<uint32_t>(present_queue_family_index),
    };
    if (rasterization_queue_family_index != present_queue_family_index) {
        // If the rasterization and presentation queues are from different queue
        // families, create the swapchain with `imageSharingMode` as
        // `vk::SharingMode::eConcurrent`.
        swapchain_create_info.imageSharingMode = vk::SharingMode::eConcurrent;
        swapchain_create_info.queueFamilyIndexCount = 2;
        swapchain_create_info.pQueueFamilyIndices = queue_family_indices.data();
    }

    vk::SwapchainKHR swapchain =
        logical_device.createSwapchainKHR(swapchain_create_info);
    return swapchain;
}

inline auto make_image_views(vk::Device* p_logical_device,
                             std::vector<vk::Image>* p_swapchain_images,
                             vk::Format format) {
    std::vector<vk::ImageView> image_views;
    image_views.reserve(p_swapchain_images->size());
    vk::ComponentMapping component_mapping(
        vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG,
        vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA);
    vk::ImageSubresourceRange sub_resource_range(
        vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
    for (auto image : *p_swapchain_images) {
        vk::ImageViewCreateInfo image_view_create_info(
            vk::ImageViewCreateFlags(), image, vk::ImageViewType::e2D, format,
            component_mapping, sub_resource_range);
        image_views.push_back(
            p_logical_device->createImageView(image_view_create_info));
    }
    return image_views;
}

// TODO: This needs be modernized.
inline auto find_supported_format(vk::PhysicalDevice* p_physical_device,
                                  std::vector<VkFormat> const& candidates,
                                  VkImageTiling tiling,
                                  VkFormatFeatureFlags features) -> vk::Format {
    for (auto const& format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(
            static_cast<VkPhysicalDevice>(*p_physical_device),
            static_cast<VkFormat>(format), &props);

        if (static_cast<VkImageTiling>(tiling) == VK_IMAGE_TILING_LINEAR &&
            (props.linearTilingFeatures & features) == features) {
            return static_cast<vk::Format>(format);
        }
        if (static_cast<VkImageTiling>(tiling) == VK_IMAGE_TILING_OPTIMAL &&
            (props.optimalTilingFeatures & features) == features) {
            return static_cast<vk::Format>(format);
        }
    }

    throw std::runtime_error("Failed to find a supported format");
}

inline auto find_depth_format(vk::PhysicalDevice* p_physical_device)
    -> vk::Format {
    return find_supported_format(
        p_physical_device,
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
         VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

inline auto make_swapchain_fences(vk::Device* p_logical_device,
                                  std::vector<vk::Image>* p_swapchain_images)
    -> std::vector<vk::Fence> {
    vk::FenceCreateInfo fence_create_info(vk::FenceCreateFlagBits::eSignaled);
    std::vector<vk::Fence> fences;
    fences.resize(p_swapchain_images->size());
    for (auto& fence : fences) {
        fence = p_logical_device->createFence(fence_create_info);
    }
    return fences;
}

inline auto make_render_pass(vk::Device* p_logical_device,
                             vk::Format color_format, vk::Format depth_format)
    -> vk::RenderPass {
    std::array<vk::AttachmentDescription, 2> attachment_descriptions;
    attachment_descriptions[0] = vk::AttachmentDescription(
        vk::AttachmentDescriptionFlags(), color_format,
        vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined,
        vk::ImageLayout::ePresentSrcKHR);
    attachment_descriptions[1] = vk::AttachmentDescription(
        vk::AttachmentDescriptionFlags(), depth_format,
        vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eDontCare, vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined,
        vk::ImageLayout::eDepthStencilAttachmentOptimal);

    vk::AttachmentReference color_reference(
        0, vk::ImageLayout::eColorAttachmentOptimal);
    vk::AttachmentReference depth_reference(
        1, vk::ImageLayout::eDepthStencilAttachmentOptimal);

    vk::SubpassDescription subpass(vk::SubpassDescriptionFlags(),
                                   vk::PipelineBindPoint::eGraphics, {},
                                   color_reference, {}, &depth_reference);

    return p_logical_device->createRenderPass(vk::RenderPassCreateInfo(
        vk::RenderPassCreateFlags(), attachment_descriptions, subpass));
}

inline void update_render_pass(vk::Device* p_logical_device,
                               vk::RenderPass* p_render_pass,
                               vk::Format color_format,
                               vk::Format depth_format) {
    p_logical_device->destroyRenderPass(*p_render_pass);
    *p_render_pass =
        crow::make_render_pass(p_logical_device, color_format, depth_format);
}

inline auto make_framebuffers(
    vk::Device* /*p_logical_device*/,
    std::vector<vk::ImageView>* p_swapchain_image_views)
    -> std::vector<vk::Framebuffer> {
    std::vector<vk::Framebuffer> framebuffers;
    framebuffers.resize(p_swapchain_image_views->size());

    std::vector<vk::ImageView> attachments{};
    attachments.reserve(framebuffers.size());
    for (int i = 0; i < framebuffers.size(); i++) {
        attachments.emplace_back((*p_swapchain_image_views)[i]);
    }
    // vk::FramebufferCreateInfo framebuffer_info(vk::FramebufferCreateFlags(),
    //                                            *p_render_pass, );
    return framebuffers;
}

inline auto make_command_pool(vk::Device* p_logical_device,
                              uint32_t queue_family_index) -> vk::CommandPool {
    vk::CommandPoolCreateInfo cmd_pool_info(vk::CommandPoolCreateFlags(),
                                            queue_family_index);
    return p_logical_device->createCommandPool(cmd_pool_info);
}

inline auto alloc_command_buffer(vk::Device* p_logical_device,
                                 vk::CommandPool* p_cmd_pool)
    -> vk::CommandBuffer {
    return p_logical_device
        ->allocateCommandBuffers(vk::CommandBufferAllocateInfo(
            *p_cmd_pool, vk::CommandBufferLevel::ePrimary, 1))
        .front();
}

}  // namespace crow
