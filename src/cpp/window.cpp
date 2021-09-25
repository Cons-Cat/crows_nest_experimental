#include "window.hpp"

#include <vulkan/vulkan.hpp>

#include <SDL.h>
#include <SDL_vulkan.h>
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
            throw std::runtime_error("Failed to create SDL2 window.");
        }
        // crow::make_vk_features();  // This mutates global::device_features
        // this->vk_features = global::device_features.features_basic;
        vk::ApplicationInfo app_info(CMAKE_GAME_TITLE, 0,
                                     "2108_GDBS_LogicVisions_GameEngine", 0,
                                     VK_API_VERSION_1_2);
        std::vector<char const*> extension_names =
            crow::make_vk_extensions(p_window);
        std::vector<char const*> layer_names = crow::make_vk_layer_names();
        vk::InstanceCreateInfo instance_info = crow::make_vk_instance_info(
            &extension_names, &layer_names, &app_info);
        this->vk_instance = vk::createInstance(instance_info);
        this->vk_device = crow::make_vk_logical_device(&vk_instance);

        SDL_Vulkan_CreateSurface(
            this->p_window, static_cast<VkInstance>(this->vk_instance),
            reinterpret_cast<VkSurfaceKHR*>(&this->vk_surface));
        if (!this->vk_surface) {
            throw std::runtime_error(
                "Failed to create an SDL2 Vulkan surface.");
        }
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
    this->vk_device.destroy();
    this->vk_instance.destroySurfaceKHR(this->vk_surface);
    this->vk_instance.destroy();
    SDL_DestroyWindow(this->p_window);
    SDL_Quit();
}

}  // namespace crow
