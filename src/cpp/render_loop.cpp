#include "render_loop.hpp"

#include "vk_globals.hpp"

namespace crow {

void render() {
    if (!global::prepared) {
        // Spin lock the render loop until a frame can be rendered.
        return;
    }
}

}  // namespace crow
