#include "render_loop.hpp"

#include <limits>

#include "synchro.hpp"
#include "vk_globals.hpp"
#include "window.hpp"

namespace crow {

constexpr uint64_t fence_timeout_nanoseconds =
    std::numeric_limits<uint64_t>::max();

void submit_and_wait(vk::Device const& device, vk::Queue const& queue,
                     vk::CommandBuffer const& commandBuffer) {
    vk::Fence fence = device.createFence(vk::FenceCreateInfo());
    queue.submit(vk::SubmitInfo(0, nullptr, nullptr, 1, &commandBuffer), fence);
    while (vk::Result::eTimeout ==
           device.waitForFences(fence, VK_TRUE, fence_timeout_nanoseconds)) {
        // Spinlock.
    }
    device.destroyFence(fence);
}

// NOLINTNEXTLINE Remove when this function is clearly not static.
void game::render() {
    auto color =
        vk::ClearValue(std::array<float, 4>({{0.2f, 0.2f, 1.2f, 0.2f}}));

    if (!global::prepared) {
        // Spin lock the render loop until a frame can be rendered.
        // return;
    }

    vk::Semaphore image_available_semaphore;
    std::array<vk::Semaphore const, 1> wait_semaphores = {
        image_available_semaphore};

    // for (int i = 0; i < this->framebuffers.size(); i++) {
    uint32_t current_framebuffer_index =
        this->logical_device
            .acquireNextImageKHR(this->swapchain, fence_timeout_nanoseconds,
                                 image_available_semaphore,
                                 // TODO: Fence
                                 VK_NULL_HANDLE)
            .value;

    vk::CommandBuffer cmd_buf =
        this->cmd_buffers_rasterize[current_framebuffer_index];
    cmd_buf.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlags()));

    vk::PipelineStageFlags wait_destination_stage_mask(
        vk::PipelineStageFlagBits::eColorAttachmentOutput);
    vk::SubmitInfo submit_info(wait_semaphores, wait_destination_stage_mask,
                               cmd_buf);
    // TODO: 0 is a hard-coded rasterization queue family:
    vk::Queue rasterization_queue = this->logical_device.getQueue(0, 0);
    vk::Fence draw_fence =
        this->logical_device.createFence(vk::FenceCreateInfo());
    rasterization_queue.submit(submit_info, draw_fence);

    while (vk::Result::eTimeout ==
           this->logical_device.waitForFences(draw_fence, VK_TRUE,
                                              fence_timeout_nanoseconds)) {
        // Spin while rendering.
    }

    // TODO: 0 is a hard-coded presentation queue family:
    vk::Queue present_queue = this->logical_device.getQueue(0, 0);
    present_queue.presentKHR(
        vk::PresentInfoKHR({}, swapchain, current_framebuffer_index));

    // vk::RenderPassBeginInfo render_pass_begin_info(
    //     this->render_pass, this->framebuffers[current_framebuffer_index],
    //     vk::Rect2D(vk::Offset2D(0, 0), this->window_extent), color);
    // cmd_buf.beginRenderPass(render_pass_begin_info,
    //                         vk::SubpassContents::eInline);

    // cmd_buf.endRenderPass();
    // cmd_buf.end();

    vk::Result result = present_queue.presentKHR(
        vk::PresentInfoKHR({}, this->swapchain, current_framebuffer_index));
    this->logical_device.waitIdle();
    // }

    // logical_device.destroySemaphore(binary_semaphore);
}

}  // namespace crow
