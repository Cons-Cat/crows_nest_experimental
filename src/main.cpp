#include <vulkan/vulkan.hpp>

#include <SDL_vulkan.h>
#include <exception>
#include <iostream>
#include <stdexcept>

#include "hpp/window.hpp"

auto main(int /*argc*/, char* /*argv*/[]) -> int {
    // TODO: Set up fmt::
    std::cout << "Hello, player!\n";
    crow::game game;
    try {
        game.initialize();
        game.loop();
        game.destroy();
    } catch (std::exception& e) {
        std::cerr << e.what() << "\n";
        std::exit(EXIT_FAILURE);
    };
    std::exit(EXIT_SUCCESS);
}
