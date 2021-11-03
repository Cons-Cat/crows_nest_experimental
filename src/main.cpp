#include <iostream>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "app.hpp"

struct RayTracingScratchBuffer {
    uint64_t device_address = 0;
    VkBuffer handle = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
};

auto main(int /*argc*/, char* /*argv*/[]) -> int {
    std::cout << "Hello, user!\n";
    App app;
    app.initialize();
    app.render_loop();
    app.free();

    return EXIT_SUCCESS;
}
