#pragma once

#include <vulkan/vulkan.h>

#include <glm/fwd.hpp>
#include <glm/geometric.hpp>

#include "../lve/lve_camera.hpp"
#include "../lve/lve_terrain.hpp"
#include "../lve/lve_wind.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace lve {

struct GlobalUbo {
   glm::mat4 projection{1.f};
   glm::mat4 view{1.f};
   glm::vec4 ambienLightColor{0.9921568627f, 0.9843137255f, 0.8274509804f,
                              .02f};
   glm::vec3 lightPosition{1.5f, -3.5f, -2.5f};
   glm::uint cols{5};
};

struct FrameInfo {
   int frameIndex;
   float frameTime;
   VkCommandBuffer commandBuffer;
   LveCamera &camera;
   VkDescriptorSet globalDescriptorSet;
   std::unique_ptr<LveTerrain> &terrain;
   std::unique_ptr<LveWind> &wind;
};

}  // namespace lve
