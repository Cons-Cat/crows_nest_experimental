#pragma once
#include <vulkan/vulkan.hpp>

namespace global {

// These declarations are `NOLINT`ed because global mutable variables are
// unsafe. These are written hastily, unfortunately.

extern bool prepared;  // NOLINT

}  // namespace global
