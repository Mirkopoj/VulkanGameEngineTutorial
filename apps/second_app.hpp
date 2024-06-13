#pragma once

#include <vulkan/vulkan_core.h>

#include <filesystem>
#include <future>
#include <memory>
#include <set>

#include "../lve/lve_descriptors.hpp"
#include "../lve/lve_device.hpp"
#include "../lve/lve_game_object.hpp"
#include "../lve/lve_renderer.hpp"
#include "../lve/lve_terrain.hpp"
#include "../lve/lve_wind.hpp"
#include "../lve/lve_window.hpp"
namespace lve {

class SecondApp {
  public:
   static constexpr int WIDTH = 800;
   static constexpr int HEIGHT = 600;

   SecondApp(const char *);
   SecondApp();
   ~SecondApp();

   SecondApp(const SecondApp &) = delete;
   SecondApp &operator=(const SecondApp &) = delete;

   void run();

   void asyncLoadGameObjects(const std::filesystem::path &);

   struct NewMap {
      uint32_t yn;
      uint32_t xn;
      std::vector<std::vector<glm::float32>> altittudeMap;
      LveTerrain::Builder terrain_builder;
      LveWind::Builder wind_builder;
   };

  private:
   LveWindow lveWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
   LveDevice lveDevice{lveWindow};
   LveRenderer lveRenderer{lveWindow, lveDevice};

   std::unique_ptr<LveDescriptorPool> globalPool =
       LveDescriptorPool::Builder(lveDevice)
           .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
           .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        LveSwapChain::MAX_FRAMES_IN_FLIGHT)
           .build();
   std::unique_ptr<LveDescriptorPool> imguiPool =
       LveDescriptorPool::Builder(lveDevice)
           .setMaxSets(4)
           .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4)
           .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
           .build();

   std::filesystem::path path{""};
   std::filesystem::path lastTryedPath{""};

   std::unique_ptr<LveTerrain> terrain = nullptr;
   std::unique_ptr<LveWind> wind = nullptr;

   uint32_t xn = 0;
   uint32_t yn = 0;

   std::vector<std::vector<glm::float32>> altitudeMap = {};

   std::set<std::string> maps = {};
   int curr = 0;
   std::future<NewMap> loadingState;
   bool loadingTerrain = false;

   int paleta_viento = 10;
   const char *paletas[18] = {
       "spring",  "summer",  "autumn",  "winter", "bone",
       "cool",    "hot",     "copper",  "hsv",    "nipy_spectral",
       "jet",     "terrain", "seismic", "afmhot", "magma",
       "inferno", "plasma",  "viridis"};
   const char *paletas_s =
       "spring\0"
       "summer\0"
       "autumn\0"
       "winter\0"
       "bone\0"
       "cool\0"
       "hot\0"
       "copper\0"
       "hsv\0"
       "nipy_spectral\0"
       "jet\0"
       "terrain\0"
       "seismic\0"
       "afmhot\0"
       "magma\0"
       "inferno\0"
       "plasma\0"
       "viridis\0";

   NewMap loadGameObjects(const std::filesystem::path &);

   void fixViewer(LveGameObject &, float);
};
}  // namespace lve
