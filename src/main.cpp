#include <iostream>
#include <vulkan/vulkan.h>

#include "../ext/Vulkan-WSIWindow/WSIWindow/CDevices.h"
#include "../ext/Vulkan-WSIWindow/WSIWindow/WSIWindow.h"

class MyWindow : public WSIWindow {};

auto main(int /*argc*/, char* /*argv*/[]) -> int {
    std::cout << "Hello, user!\n";

    CInstance instance(true);
    instance.DebugReport.SetFlags(8);  // Minimal debug information.
    MyWindow window;
    window.SetTitle("");
    window.SetWinSize(640, 480);
    VkSurfaceKHR p_surface = window.GetSurface(instance);

    CPhysicalDevices gpus(instance);  // Enumerate GPUs and their properties.
    CPhysicalDevice* p_gpu = gpus.FindPresentable(p_surface);

    CDevice device(*p_gpu);  // Create logical device on selected GPU.
    CQueue* p_queue = device.AddQueue(VK_QUEUE_COMPUTE_BIT, p_surface);

    // Main event loop, runs until window is closed.
    while (window.ProcessEvents()) {
    }

    return EXIT_SUCCESS;
}
