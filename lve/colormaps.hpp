#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace colormap {

constexpr size_t color_points = 256;
typedef const glm::vec3 colormap_t[color_points];

const colormap_t* const colormap(const size_t);

const char* paletas();

}  // namespace colormaps
