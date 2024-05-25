#include "second_app.hpp"

#include <pthread.h>
#include <vulkan/vulkan_core.h>

// std
#include <imgui.h>

#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <glm/common.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <memory>
#include <vector>

#include "../asc_process/Lexer.hpp"
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

namespace lve {

SecondApp::SecondApp(const char* map) {
   globalPool = LveDescriptorPool::Builder(lveDevice)
                    .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
                    .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                 LveSwapChain::MAX_FRAMES_IN_FLIGHT)
                    .build();

   loadGameObjects(map);
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
   viewerObject.transform.translation.x = static_cast<float>(xn - 1) / 2.f;
   viewerObject.transform.translation.z = static_cast<float>(yn - 1) / 2.f;
   KeyboardMovementController cameraController{};
   float cameraHeight = 2.f;

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
      uint32_t x = glm::clamp(
          xn - (uint32_t)roundf(viewerObject.transform.translation.x),
          (uint32_t)0, xn - 1);
      uint32_t y = glm::clamp(
          (uint32_t)roundf(viewerObject.transform.translation.z),
          (uint32_t)0, yn - 1);
      viewerObject.transform.translation =
          glm::clamp(viewerObject.transform.translation,
                     glm::vec3{0.f, -fmin(xn, yn), 0.f},
                     glm::vec3{xn, -cameraHeight - altitudeMap[y][x], yn});
      camera.setViewYXZ(viewerObject.transform.translation,
                        viewerObject.transform.rotation);

      float aspect = lveRenderer.getAspectRatio();
      camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f,
                                      fmax(xn, yn) * 1.8);

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
         ubo.cols = xn;
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

void SecondApp::loadGameObjects(const char* map) {
   std::ifstream ifile(map);
   int32_t NODATA_value;
   int cellsize;
   const int meta_data_lines = 6;
   std::string line;
   for (int i = 0; i < meta_data_lines; ++i) {
      std::getline(ifile, line);
      Lexer::Line pair = Lexer::parse(line);
      if (!std::strcmp(pair.name.c_str(), "ncols")) {
         xn = pair.value;
         continue;
      }
      if (!std::strcmp(pair.name.c_str(), "nrows")) {
         yn = pair.value;
         continue;
      }
      if (!std::strcmp(pair.name.c_str(), "NODATA_value")) {
         NODATA_value = pair.value;
         continue;
      }
      if (!std::strcmp(pair.name.c_str(), "cellsize")) {
         cellsize = pair.value;
         continue;
      }
   }
   altitudeMap = {};
   glm::float32 min = NAN;
   for (std::string line; std::getline(ifile, line);) {
      std::vector<std::string> altitudeMapIn = Lexer::tokenize(line);
      std::vector<glm::float32> aux = {};
      for (int x = 0; x < xn; ++x) {
         int val = std::stoi(altitudeMapIn[x]);
         if (val == NODATA_value) {
            val = 0;
         }
         glm::float32 alt{static_cast<float>(val) / cellsize};
         min = min != NAN && min < alt ? min : alt;
         aux.push_back(alt);
      }
      altitudeMap.push_back(aux);
   }
   for (std::vector<glm::float32>& row : altitudeMap) {
      for (glm::float32& cell : row) {
         cell -= min;
      }
   }

   terrain = LveTerrain::createModelFromMesh(lveDevice, altitudeMap);
}

}  // namespace lve
