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
#include <iostream>
#include <iterator>
#include <limits>
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
#include "../lve/colormaps.hpp"
#include "../movement_controllers/terrain_movement_controller.hpp"
#include "../systems/gui_system.hpp"
#include "../systems/terrain_render_system.hpp"
#include "../systems/wind_render_system.hpp"
#include "second_app_frame_info.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace lve {

SecondApp::SecondApp() {
}

SecondApp::SecondApp(const char* path) {
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

   WindRenderSystem windRenderSystem{
       lveDevice, lveRenderer.getSwapChainRenderPass(),
       globalSetLayout->getDescriptorSetLayout(),
       "shaders/wind_shader.vert.spv", "shaders/wind_shader.frag.spv"};

   LveCamera camera{};

   float cameraHeight = 2.f;
   LveGameObject viewerObject = LveGameObject::createGameObject();
   fixViewer(viewerObject, cameraHeight);

   TerrainMovementController cameraController{};

   ImGuiGui myimgui(lveWindow.getGLFWwindow(), lveDevice, lveRenderer,
                    imguiPool->descriptor_pool());

   auto currentTime = std::chrono::high_resolution_clock::now();
   bool caminata = false;
   bool viento = false;

   std::string new_path = path;
   size_t pipeline = 0;
   int paleta_elegida = paleta_viento;

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
                             terrain,
                             wind};
         myimgui.new_frame();

         // update
         GlobalUbo ubo{};
         ubo.projection = camera.getProjection();
         ubo.view = camera.getView();
         ubo.cols = xn;
         ubo.time = currentTime.time_since_epoch().count() / 10000000L;
         uboBuffers[frameIndex]->writeToBuffer(&ubo);
         uboBuffers[frameIndex]->flush();
         myimgui.update(cameraController, caminata, new_path, maps, curr,
                        loadingTerrain, pipeline,
                        viewerObject.transform.translation, viento,
                        paleta_elegida, colormap::paletas());

         // render system
         lveRenderer.beginSwapChainRenderPass(commandBuffer);

         if (terrain) {
            terrainRenderSystem.renderTerrain(
                frameInfo,
                static_cast<TerrainRenderSystem::PipeLineType>(pipeline));
         }
         if (wind && viento) {
            windRenderSystem.renderWind(frameInfo);
         }
         myimgui.render(commandBuffer);

         lveRenderer.endSwapChainRenderPass(commandBuffer);
         lveRenderer.endFrame();
      }

      if (new_path != lastTryedPath && !loadingTerrain) {
         asyncLoadGameObjects(new_path.c_str());
      }
      if (paleta_viento != paleta_elegida && !loadingTerrain) {
         paleta_viento = paleta_elegida;
         asyncLoadGameObjects(new_path.c_str());
      }

      if (loadingState.valid() &&
          loadingState.wait_for(std::chrono::duration(
              std::chrono::seconds(0))) == std::future_status::ready) {
         loadingTerrain = false;
         try {
            NewMap newMap = loadingState.get();
            altitudeMap = newMap.altittudeMap;
            xn = newMap.xn;
            yn = newMap.yn;
            terrain = std::make_unique<LveTerrain>(lveDevice,
                                                   newMap.terrain_builder);
            fixViewer(viewerObject, cameraHeight);
            wind =
                std::make_unique<LveWind>(lveDevice, newMap.wind_builder);
         } catch (...) {
         }
      }
   }

   vkDeviceWaitIdle(lveDevice.device());
}

SecondApp::NewMap SecondApp::loadGameObjects(
    const std::filesystem::path& new_path) {
   Lexer::Config config(new_path);
   NewMap newMap;

   newMap.yn = std::stoi(config.value("ROWS"));
   newMap.xn = std::stoi(config.value("COLS"));
   auto altittude_join =
       std::async(std::launch::async, [&config, &newMap] {
          Lexer::Ascf altitudeAsc = Lexer::loadf(
              (config.get_path() / config.value("ELEV_MAP")).c_str());
          newMap.altittudeMap = altitudeAsc.body;
          glm::float32 min = NAN;
          for (std::vector<glm::float32>& row : newMap.altittudeMap) {
             for (glm::float32& cell : row) {
                if (cell == altitudeAsc.NODATA_value) {
                   cell = 0;
                }
                glm::float32 alt{cell / altitudeAsc.cellsize};
                cell = alt;
                min = min != NAN && min < cell ? min : cell;
             }
          }
          for (std::vector<glm::float32>& row : newMap.altittudeMap) {
             for (glm::float32& cell : row) {
                cell -= min;
             }
          }
       });
   auto terrain_join = std::async(std::launch::async, [&config, &newMap,
                                                       &altittude_join] {
      auto beginTime = std::chrono::high_resolution_clock::now();
      auto vege_join = std::async(std::launch::async, [&config] {
         Lexer::Asci vegetationAsc = Lexer::loadi(
             (config.get_path() / config.value("VEGETATION_MAP")).c_str());
         return vegetationAsc.body;
      });
      std::vector<std::vector<glm::vec3>> colorMap;
      auto paleta_join = std::async(std::launch::async, [&config] {
         Lexer::PaletDB paletDb{
             (config.get_path() / config.value("PALETA")).c_str()};
         return paletDb;
      });
      std::vector<std::vector<glm::int32>> vegetationMap = vege_join.get();
      Lexer::PaletDB paletDb = paleta_join.get();
      for (std::vector<glm::int32> row : vegetationMap) {
         std::vector<glm::vec3> aux;
         for (glm::int32 cell : row) {
            Lexer::PaletDB::Color color = paletDb.color(cell);
            aux.push_back(color.color);
         }
         colorMap.push_back(aux);
      }
      altittude_join.wait();
      newMap.terrain_builder.generateMesh(newMap.altittudeMap, colorMap);
      auto endTime = std::chrono::high_resolution_clock::now();
      float time =
          std::chrono::duration<float, std::chrono::seconds::period>(
              endTime - beginTime)
              .count();
      std::cout << "Terrain time: " << time << "\n";
   });

   auto wind_join = std::async(std::launch::async, [&config, &newMap,
                                                    &altittude_join,
                                                    this] {
      auto beginTime = std::chrono::high_resolution_clock::now();
      auto dir_join = std::async(std::launch::async, [&config] {
         return Lexer::loadi(
                    (config.get_path() / config.value("WIND_MAP")).c_str())
             .body;
      });
      auto vel_join = std::async(std::launch::async, [&config] {
         return Lexer::loadf(
                    (config.get_path() / config.value("INT_WIND")).c_str())
             .body;
      });

      std::vector<std::vector<glm::int32>> dirViento = dir_join.get();
      std::vector<std::vector<glm::float32>> velViento = vel_join.get();
      std::vector<std::vector<glm::vec2>> windSpeed;
      float min = std::numeric_limits<float>::max();
      float max = std::numeric_limits<float>::min();
      for (size_t y = 0; y < dirViento.size(); ++y) {
         std::vector<glm::vec2> row;
         for (size_t x = 0; x < dirViento[0].size(); ++x) {
            float angulo = dirViento[y][x] * glm::two_pi<float>() / 360.f;
            row.push_back(glm::vec2(glm::cos(angulo), glm::sin(angulo)) *
                          velViento[y][x]);
            min = glm::min(min, velViento[y][x]);
            max = glm::max(max, velViento[y][x]);
         }
         windSpeed.push_back(row);
      }
      altittude_join.wait();
      newMap.wind_builder.generateMesh(newMap.altittudeMap, windSpeed, min,
                                       max, paleta_viento);
      auto endTime = std::chrono::high_resolution_clock::now();
      float time =
          std::chrono::duration<float, std::chrono::seconds::period>(
              endTime - beginTime)
              .count();
      std::cout << "Wind time: " << time << "\n";
   });

   path = new_path;
   std::pair<std::set<std::string>::iterator, bool> insert_result =
       maps.insert(path);
   if (insert_result.second) {
      curr = std::distance(maps.begin(), insert_result.first);
   }

   terrain_join.wait();
   wind_join.wait();

   return newMap;
}

void SecondApp::asyncLoadGameObjects(
    const std::filesystem::path& new_path) {
   lastTryedPath = new_path;
   if (std::filesystem::exists(new_path / "config.txt")) {
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
