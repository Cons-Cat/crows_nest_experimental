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
        window = SDL_CreateWindow(CMAKE_GAME_TITLE, SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED, 1280, 720,
                                  SDL_WINDOW_VULKAN);
        if (window == nullptr) {
            throw "Failed to create SDL2 window.";
        }
        crow::make_vk_features();  // This mutates global::device_features
        this->vk_features = global::device_features.features_basic;
        vk::InstanceCreateInfo info = crow::make_vk_create_info(window);
        this->vk_instance = vk::createInstance(info);

        SDL_Vulkan_CreateSurface(
            this->window, static_cast<VkInstance>(this->vk_instance),
            reinterpret_cast<VkSurfaceKHR*>(&this->vk_surface));
        if (!this->vk_surface) {
            throw "Failed to create an SDL2 Vulkan surface.";
        }
    } catch (std::exception& e) {
        // TODO: Set up fmt::
        std::cerr << e.what() << "\n";
        throw;
    }
}

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

// NOLINTNEXTLINE
void game::destroy() {
    this->vk_instance.destroy();
    SDL_DestroyWindow(this->window);
    SDL_Quit();
}

}  // namespace crow
