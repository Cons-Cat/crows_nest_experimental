#include <GLFW/glfw3.h>
#include <iostream>
#include <vulkan/vulkan.h>

#include "../ext/Vulkan-WSIWindow/WSIWindow/CDevices.h"
#include "../ext/Vulkan-WSIWindow/WSIWindow/WSIWindow.h"

class MyWindow : public WSIWindow {};

auto main(int /*argc*/, char* /*argv*/[]) -> int {
    std::cout << "Hello, user!\n";
    CLayers layers;
    layers.Pick({
        "VK_LAYER_KHRONOS_validation",
    });
    CInstance instance(true);
    CExtensions extensions;
    extensions.PickAll();

    MyWindow window;
    window.SetTitle("");
    window.SetWinSize(640, 480);
    VkSurfaceKHR p_surface = window.GetSurface(instance);

    CPhysicalDevices gpus(instance);  // Enumerate GPUs and their properties.
    CPhysicalDevice* p_gpu = gpus.FindPresentable(p_surface);

    CDevice device(*p_gpu);
    CQueue* p_queue = device.AddQueue(VK_QUEUE_COMPUTE_BIT, p_surface);

    // layers.Print();
    // extensions.Print();

    // Main event loop, runs until window is closed.
    while (window.ProcessEvents()) {
    }

    return EXIT_SUCCESS;
}
