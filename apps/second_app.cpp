#include "second_app.hpp"

#include <vulkan/vulkan_core.h>

#include <cstdlib>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <memory>
#include <vector>

#include "../keyboard_movement_controller.hpp"
#include "../lve/lve_buffer.hpp"
#include "../lve/lve_camera.hpp"
#include "../lve/lve_descriptors.hpp"
#include "../lve/lve_device.hpp"
#include "../lve/lve_game_object.hpp"
#include "../lve/lve_swap_chain.hpp"
#include "../lve/lve_terrain.hpp"
#include "../systems/terrain_render_system.hpp"
#include "second_app_frame_info.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <imgui.h>

#include <chrono>

namespace lve {

SecondApp::SecondApp() {
   globalPool = LveDescriptorPool::Builder(lveDevice)
                    .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
                    .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                 LveSwapChain::MAX_FRAMES_IN_FLIGHT)
                    .build();

   loadGameObjects();
}

SecondApp::~SecondApp() {
}

void SecondApp::run() {
   std::vector<std::unique_ptr<LveBuffer>> uboBuffers(
       LveSwapChain::MAX_FRAMES_IN_FLIGHT);
   for (int i = 0; i < uboBuffers.size(); i++) {
      uboBuffers[i] =
          std::make_unique<LveBuffer>(lveDevice, sizeof(GlobalUbo), 1,
                                      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
      uboBuffers[i]->map();
   }

   std::unique_ptr<LveDescriptorSetLayout> globalSetLayout =
       LveDescriptorSetLayout::Builder(lveDevice)
           .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                       VK_SHADER_STAGE_ALL_GRAPHICS)
           .build();

   std::vector<VkDescriptorSet> globalDescriptorSets(
       LveSwapChain::MAX_FRAMES_IN_FLIGHT);
   for (int i = 0; i < globalDescriptorSets.size(); i++) {
      auto bufferInfo = uboBuffers[i]->descriptorInfo();
      LveDescriptorWriter(*globalSetLayout, *globalPool)
          .writeBuffer(0, &bufferInfo)
          .build(globalDescriptorSets[i]);
   }

   TerrainRenderSystem terrainRenderSystem{
       lveDevice, lveRenderer.getSwapChainRenderPass(),
       globalSetLayout->getDescriptorSetLayout(),
       "shaders/terrain_shader.vert.spv",
       "shaders/terrain_shader.frag.spv"};

   LveCamera camera{};

   auto viewerObject = LveGameObject::createGameObject();
   viewerObject.transform.translation.z = -2.5f;
   KeyboardMovementController cameraController{};

   auto currentTime = std::chrono::high_resolution_clock::now();

   while (!lveWindow.shouldClose()) {
      glfwPollEvents();

      auto newTime = std::chrono::high_resolution_clock::now();
      float frameTime =
          std::chrono::duration<float, std::chrono::seconds::period>(
              newTime - currentTime)
              .count();
      currentTime = newTime;

      cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime,
                                     viewerObject);
      camera.setViewYXZ(viewerObject.transform.translation,
                        viewerObject.transform.rotation);

      float aspect = lveRenderer.getAspectRatio();
      camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f,
                                      100.f);

      if (auto commandBuffer = lveRenderer.beginFrame()) {
         int frameIndex = lveRenderer.getFrameIndex();
         FrameInfo frameInfo{frameIndex,
                             frameTime,
                             commandBuffer,
                             camera,
                             globalDescriptorSets[frameIndex],
                             *terrain};

         // update
         GlobalUbo ubo{};
         ubo.projection = camera.getProjection();
         ubo.view = camera.getView();
         ubo.inverseView = camera.getInverseView();
         ubo.cols = 5;
         uboBuffers[frameIndex]->writeToBuffer(&ubo);
         uboBuffers[frameIndex]->flush();

         // render system
         lveRenderer.beginSwapChainRenderPass(commandBuffer);

         terrainRenderSystem.renderTerrain(frameInfo);

         lveRenderer.endSwapChainRenderPass(commandBuffer);
         lveRenderer.endFrame();
      }
   }

   vkDeviceWaitIdle(lveDevice.device());
}

void SecondApp::loadGameObjects() {
   std::vector<std::vector<float>> altitudeMap = {};
   for (int x = 0; x < 5; ++x) {
      std::vector<float> aux = {};
      for (int y = 0; y < 5; ++y) {
         aux.push_back(static_cast<float>(rand() % 4) / 3.f);
      }
      altitudeMap.push_back(aux);
   }
   terrain = LveTerrain::createModelFromMesh(lveDevice, altitudeMap);
}

}  // namespace lve
