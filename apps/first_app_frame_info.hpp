#pragma once

#include <vulkan/vulkan.h>

#include <glm/fwd.hpp>
#include <glm/geometric.hpp>

#include "../lve/lve_camera.hpp"
#include "../lve/lve_game_object.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>


namespace lve {

#define MAX_LIGHTS 10

struct PointLight {
   glm::vec4 position{};
   glm::vec4 color{};
};

struct GlobalUbo {
   glm::mat4 projection{1.f};
   glm::mat4 view{1.f};
   glm::mat4 inverseView{1.f};
   glm::vec4 ambienLightColor{1.f, 1.f, 1.f, .02f};
   PointLight pointLights[MAX_LIGHTS];
   int numLights;
};

struct FrameInfo {
   int frameIndex;
   float frameTime;
   VkCommandBuffer commandBuffer;
   LveCamera &camera;
   VkDescriptorSet globalDescriptorSet;
   LveGameObject::Map &gameObjects;
};


}  // namespace lve
