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
  } catch (...) {
    std::exit(EXIT_FAILURE);
  };
  game.loop();
  game.destroy();
  std::exit(EXIT_SUCCESS);
}
