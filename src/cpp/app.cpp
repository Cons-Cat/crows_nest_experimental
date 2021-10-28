#include "app.hpp"

#include <cstring>
// #include <iostream>
#include <span>
#include <vulkan/vulkan_core.h>

// NOLINTNEXTLINE
static std::array<char, 500> key_down_index;

static void key_callback(GLFWwindow* /*p_window*/, int key, int /*scancode*/,
                         int action, int /*mods*/) {
    if (action == GLFW_PRESS) {
        key_down_index[key] = 1;  // NOLINT
    }
    if (action == GLFW_RELEASE) {
        key_down_index[key] = 0;  // NOLINT
    }
}

void App::create_surface() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    this->window =
        glfwCreateWindow(800, 600, "Raytracing Demo", nullptr, nullptr);
    glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(this->window, key_callback);

    uint32_t glfw_extension_count = 0;
    char const** p_glfw_extension_names =
        glfwGetRequiredInstanceExtensions(&glfw_extension_count);
    uint32_t extension_count = glfw_extension_count + 1;
    std::span<char const*> extension_names = {
        p_glfw_extension_names, sizeof(char const*) * extension_count};
    extension_names[glfw_extension_count] =
        "VK_KHR_get_physical_device_properties2";

    VkApplicationInfo application_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Raytracing Demo",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_2,
    };

    VkInstanceCreateInfo instance_create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .flags = 0,
        .pApplicationInfo = &application_info,
        .enabledExtensionCount = extension_count,
        .ppEnabledExtensionNames = extension_names.data(),
    };

    uint32_t layer_count = 1;
    char const** p_layer_names = new (std::nothrow) char const*;
    std::span<char const*> layer_names{p_layer_names,
                                       sizeof(char const*) * layer_count};
    layer_names[0] = "VK_LAYER_KHRONOS_validation";

    instance_create_info.enabledLayerCount = layer_count;
    instance_create_info.ppEnabledLayerNames = layer_names.data();

    if (vkCreateInstance(&instance_create_info, nullptr, &this->instance) ==
        VK_SUCCESS) {
    }

    if (glfwCreateWindowSurface(this->instance, this->window, nullptr,
                                &this->surface) == VK_SUCCESS) {
    }

    // std::cout << extension_names.size() << '\n';
    // std::cout << layer_names.size() << '\n';
}

void App::initialize() {
    this->create_surface();
    this->render_loop();
}

// NOLINTNEXTLINE this cannot be static.
void App::render_loop() {
    while (glfwWindowShouldClose(this->window) == 0) {
        glfwPollEvents();
    }
}
