#pragma once

#include <vulkan/vulkan_core.h>

#include <future>
#include <memory>
#include <set>

#include "../asc_process/Lexer.hpp"
#include "../lve/lve_descriptors.hpp"
#include "../lve/lve_device.hpp"
#include "../lve/lve_renderer.hpp"
#include "../lve/lve_terrain.hpp"
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

   void asyncLoadGameObjects(const char *);

   struct NewMap {
      uint32_t yn;
      uint32_t xn;
      std::vector<std::vector<glm::float32>> altitudeMap;
      std::vector<std::vector<glm::vec3>> colorMap;
   };

  private:
   LveWindow lveWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
   LveDevice lveDevice{lveWindow};
   LveRenderer lveRenderer{lveWindow, lveDevice};

   std::unique_ptr<LveDescriptorPool> globalPool{};
   std::unique_ptr<LveDescriptorPool> imguiPool{};

   std::string path = "";
   std::string lastTryedPath = "";

   std::unique_ptr<LveTerrain> terrain = nullptr;

   uint32_t xn = 0;
   uint32_t yn = 0;

   std::vector<std::vector<glm::float32>> altitudeMap = {};

   std::set<std::string> maps = {};
   int curr = 0;
   std::future<NewMap> loadingState;
   bool loadingTerrain = false;

   NewMap loadGameObjects(Lexer::Config);
};
}  // namespace lve
