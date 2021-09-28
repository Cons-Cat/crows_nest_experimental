#include "vk_globals.hpp"

namespace global {

// These declarations are `NOLINT`ed because global mutable variables are
// unsafe. These are written hastily, unfortunately.

bool prepared = false;  // NOLINT

}  // namespace global
