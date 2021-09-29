#include "render_loop.hpp"

#include "vk_globals.hpp"
#include "window.hpp"

namespace crow {

// NOLINTNEXTLINE Remove when this function is clearly not static.
void game::render() {
    if (!global::prepared) {
        // Spin lock the render loop until a frame can be rendered.
        return;
    }
}

}  // namespace crow
