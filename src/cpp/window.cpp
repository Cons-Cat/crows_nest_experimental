#include "window.hpp"

#include <SDL.h>
#include <exception>
#include <iostream>
#include <stdexcept>

namespace crow {

void game::initialize() {
  try {
    window =
        SDL_CreateWindow(CMAKE_GAME_TITLE, SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_VULKAN);
    if (!window) {
      throw "Failed to create SDL2 window.";
    }
    uint32_t extension_count = 0;
    SDL_Vulkan_GetInstanceExtensions(window, &extension_count, nullptr);
    std::vector<const char*> extension_names(extension_count);
    SDL_Vulkan_GetInstanceExtensions(window, &extension_count,
                                     extension_names.data());
    // TODO: Set application version and engine version here via CMake.
    vk::ApplicationInfo app_info(CMAKE_GAME_TITLE, 0, "", 0,
                                 VK_API_VERSION_1_2);
    std::vector<const char*> layer_names{
#ifdef DEBUG
        "VK_LAYER_LUNARG_standard_validation",
#endif
    };
    vk::InstanceCreateInfo info{};
    info.sType = vk::StructureType::eInstanceCreateInfo;
    info.pApplicationInfo = &app_info;
    info.enabledLayerCount = layer_names.size();
    info.ppEnabledLayerNames = layer_names.data();
    info.enabledExtensionCount = extension_names.size();
    info.ppEnabledExtensionNames = extension_names.data();
    this->vk_instance = vk::createInstance(info);

    SDL_Vulkan_CreateSurface(
        this->window, static_cast<VkInstance>(this->vk_instance),
        reinterpret_cast<VkSurfaceKHR*>(&this->vk_surface));
    if (!this->vk_surface) {
      throw "Failed to create an SDL2 Vulkan surface.";
    }
  } catch (...) {
    // TODO: Set up fmt::
    // std::cerr << std::current_exception() << "\n";
    throw;
  }
}

void game::loop() {
  while (true) {
    SDL_Event event;
    if (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        break;
      }
    }
  }
}

// NOLINTNEXTLINE
void game::destroy() {
  this->vk_instance.destroy();
  SDL_DestroyWindow(this->window);
  SDL_Quit();
}

}  // namespace crow
