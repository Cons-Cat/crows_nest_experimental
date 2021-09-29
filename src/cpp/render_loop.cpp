#include "render_loop.hpp"

#include "vk_globals.hpp"
#include "window.hpp"

namespace crow {

constexpr uint32_t fence_timeout_nanoseconds = 100;

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
    if (!global::prepared) {
        // Spin lock the render loop until a frame can be rendered.
        // return;
    }

    vk::Semaphore image_acquired_semaphore =
        this->logical_device.createSemaphore(
            vk::SemaphoreCreateInfo(vk::SemaphoreCreateFlags()));
    auto color =
        vk::ClearValue(std::array<float, 4>({{0.2f, 0.2f, 1.2f, 0.2f}}));
    for (int i = 0; i < this->framebuffers.size(); i++) {
        vk::CommandBuffer cmd_buf = this->cmd_buffers_rasterize[i];
        uint32_t current_buffer =
            this->logical_device
                .acquireNextImageKHR(this->swapchain, fence_timeout_nanoseconds,
                                     image_acquired_semaphore,
                                     // TODO: Fence
                                     nullptr)
                .value;

        cmd_buf.begin(
            vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlags()));

        vk::RenderPassBeginInfo render_pass_begin_info(
            this->render_pass, framebuffers[current_buffer],
            vk::Rect2D(vk::Offset2D(0, 0), this->window_extent), color);
        cmd_buf.beginRenderPass(render_pass_begin_info,
                                vk::SubpassContents::eInline);
        // cmd_buf.clearColorImage(
        //     this->framebuffers[current_buffer],
        //     vk::ImageLayout::eTransferDstOptimal, color,
        //     vk::Rect2D(vk::Offset2D(0, 0), this->window_extent), nullptr);
        cmd_buf.endRenderPass();
        cmd_buf.end();

        // vk::Fence draw_fence =
        //     this->logical_device.createFence(vk::FenceCreateInfo());
        vk::PipelineStageFlags wait_destination_stage_mask(
            vk::PipelineStageFlagBits::eColorAttachmentOutput);
        vk::SubmitInfo submit_info(image_acquired_semaphore,
                                   wait_destination_stage_mask, cmd_buf);
        vk::Fence draw_fence =
            this->logical_device.createFence(vk::FenceCreateInfo());
        vk::Queue graphics_queue = this->logical_device.getQueue(0, 0);
        vk::Queue present_queue = this->logical_device.getQueue(0, 0);
        graphics_queue.submit(submit_info, draw_fence);
        while (vk::Result::eTimeout ==
               this->logical_device.waitForFences(draw_fence, VK_TRUE,
                                                  fence_timeout_nanoseconds)) {
            // Spin while rendering.
        }

        vk::Result result = present_queue.presentKHR(
            vk::PresentInfoKHR({}, this->swapchain, current_buffer));
        this->logical_device.waitIdle();
    }
    // this->logical_device.destroySemaphore(image_acquired_semaphore);
}

}  // namespace crow
