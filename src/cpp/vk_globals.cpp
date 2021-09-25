#include "vk_globals.hpp"

namespace global {

// These declarations are `NOLINT`ed because global mutable variables are
// unsafe. These are written hastily, unfortunately.

vulkan_physical_device_features device_features;  // NOLINT
bool prepared = false;                            // NOLINT

}  // namespace global
