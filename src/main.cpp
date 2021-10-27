#include <iostream>
#include <vulkan/vulkan.h>

#include "app.hpp"

struct RayTracingScratchBuffer {
    uint64_t deviceAddress = 0;
    VkBuffer handle = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
};

struct AccelerationStructure {
    VkAccelerationStructureKHR handle;
    uint64_t deviceAddress = 0;
    VkDeviceMemory memory;
    VkBuffer buffer;
};

auto main(int /*argc*/, char* /*argv*/[]) -> int {
    std::cout << "Hello, user!\n";
    App app;
    app.initialize();

    return EXIT_SUCCESS;
}
