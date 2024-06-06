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
#include <filesystem>
#include <future>
#include <glm/common.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

#include "../asc_process/Lexer.hpp"
#include "../lve/lve_buffer.hpp"
#include "../lve/lve_camera.hpp"
#include "../lve/lve_descriptors.hpp"
#include "../lve/lve_device.hpp"
#include "../lve/lve_swap_chain.hpp"
#include "../lve/lve_terrain.hpp"
#include "../movement_controllers/terrain_movement_controller.hpp"
#include "../systems/gui_system.hpp"
#include "../systems/terrain_render_system.hpp"
#include "second_app_frame_info.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace lve {

SecondApp::SecondApp()
    : globalPool(LveDescriptorPool::Builder(lveDevice)
                     .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
                     .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                  LveSwapChain::MAX_FRAMES_IN_FLIGHT)
                     .build()),
      imguiPool(
          LveDescriptorPool::Builder(lveDevice)
              .setMaxSets(4)
              .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4)
              .setPoolFlags(
                  VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
              .build()) {
}

SecondApp::SecondApp(const char* path)
    : globalPool(LveDescriptorPool::Builder(lveDevice)
                     .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
                     .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                  LveSwapChain::MAX_FRAMES_IN_FLIGHT)
                     .build()),
      imguiPool(
          LveDescriptorPool::Builder(lveDevice)
              .setMaxSets(4)
              .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4)
              .setPoolFlags(
                  VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
              .build()) {
   asyncLoadGameObjects(path);
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

   float cameraHeight = 2.f;
   LveGameObject viewerObject = LveGameObject::createGameObject();
   fixViewer(viewerObject, cameraHeight);

   TerrainMovementController cameraController{};

   ImGuiGui myimgui(lveWindow.getGLFWwindow(), lveDevice, lveRenderer,
                    imguiPool->descriptor_pool());

   auto currentTime = std::chrono::high_resolution_clock::now();
   bool caminata = false;

   std::string new_path = path;
   size_t pipeline = 0;

   while (!lveWindow.shouldClose()) {
      glfwPollEvents();

      auto newTime = std::chrono::high_resolution_clock::now();
      float frameTime =
          std::chrono::duration<float, std::chrono::seconds::period>(
              newTime - currentTime)
              .count();
      currentTime = newTime;

      cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime,
                                     viewerObject, altitudeMap,
                                     cameraHeight, caminata);

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
                             terrain};
         myimgui.new_frame();

         // update
         GlobalUbo ubo{};
         ubo.projection = camera.getProjection();
         ubo.view = camera.getView();
         ubo.inverseView = camera.getInverseView();
         ubo.cols = xn;
         uboBuffers[frameIndex]->writeToBuffer(&ubo);
         uboBuffers[frameIndex]->flush();
         myimgui.update(cameraController, caminata, new_path, maps, curr,
                        loadingTerrain, pipeline);

         // render system
         lveRenderer.beginSwapChainRenderPass(commandBuffer);

         if (terrain) {
            terrainRenderSystem.renderTerrain(
                frameInfo,
                static_cast<TerrainRenderSystem::PipeLineType>(pipeline));
         }
         myimgui.render(commandBuffer);

         lveRenderer.endSwapChainRenderPass(commandBuffer);
         lveRenderer.endFrame();
      }

      if (new_path != lastTryedPath && !loadingTerrain) {
         asyncLoadGameObjects(new_path.c_str());
      }

      if (loadingState.valid() &&
          loadingState.wait_for(std::chrono::duration(
              std::chrono::seconds(0))) == std::future_status::ready) {
         loadingTerrain = false;
         try {
            NewMap newMap = loadingState.get();
            altitudeMap = newMap.altitudeMap;
            xn = newMap.xn;
            yn = newMap.yn;
            terrain =
                std::make_unique<LveTerrain>(lveDevice, newMap.builder);
            fixViewer(viewerObject, cameraHeight);
         } catch (...) {
         }
      }
   }

   vkDeviceWaitIdle(lveDevice.device());
}

SecondApp::NewMap SecondApp::loadGameObjects(const char* new_path) {
   Lexer::Config config(new_path);
   NewMap newMap;

   newMap.yn = std::stoi(config.value("ROWS"));
   newMap.xn = std::stoi(config.value("COLS"));
   Lexer::Ascf altitudeAsc = Lexer::loadf(
       (config.get_path() + config.value("ELEV_MAP")).c_str());
   newMap.altitudeMap = altitudeAsc.body;
   glm::float32 min = NAN;
   for (std::vector<glm::float32>& row : newMap.altitudeMap) {
      for (glm::float32& cell : row) {
         if (cell == altitudeAsc.NODATA_value) {
            cell = 0;
         }
         glm::float32 alt{cell / altitudeAsc.cellsize};
         cell = alt;
         min = min != NAN && min < cell ? min : cell;
      }
   }
   for (std::vector<glm::float32>& row : newMap.altitudeMap) {
      for (glm::float32& cell : row) {
         cell -= min;
      }
   }
   Lexer::Asci vegetationAsc = Lexer::loadi(
       (config.get_path() + config.value("VEGETATION_MAP")).c_str());
   std::vector<std::vector<glm::int32>> vegetationMap = vegetationAsc.body;
   std::vector<std::vector<glm::vec3>> colorMap;
   Lexer::PaletDB paletDb{
       (config.get_path() + config.value("PALETA")).c_str()};
   for (std::vector<glm::int32> row : vegetationMap) {
      std::vector<glm::vec3> aux;
      for (glm::int32 cell : row) {
         Lexer::PaletDB::Color color = paletDb.color(cell);
         aux.push_back(color.color);
      }
      colorMap.push_back(aux);
   }

   newMap.builder.generateMesh(newMap.altitudeMap, colorMap);

   path = new_path;
   std::pair<std::set<std::string>::iterator, bool> insert_result =
       maps.insert(path);
   if (insert_result.second) {
      curr = std::distance(maps.begin(), insert_result.first);
   }
   return newMap;
}

void SecondApp::asyncLoadGameObjects(const char* new_path) {
   lastTryedPath = new_path;
   if (std::filesystem::exists(
           std::filesystem::path(std::string(new_path) + "config.txt"))) {
      loadingState = std::async(
          std::launch::async, &SecondApp::loadGameObjects, this, new_path);
      loadingTerrain = true;
   }
}

void SecondApp::fixViewer(LveGameObject& viewerObject,
                          float cameraHeight) {
   viewerObject.transform.translation.x = static_cast<float>(xn - 1) / 2.f;
   viewerObject.transform.translation.z = static_cast<float>(yn - 1) / 2.f;
   uint32_t x = glm::clamp(
       xn - (uint32_t)roundf(viewerObject.transform.translation.x),
       (uint32_t)0, xn - 1);
   uint32_t y =
       glm::clamp((uint32_t)roundf(viewerObject.transform.translation.z),
                  (uint32_t)0, yn - 1);
   if (xn && yn) {
      viewerObject.transform.translation.y =
          -cameraHeight - altitudeMap[y][x];
   }
}

}  // namespace lve
