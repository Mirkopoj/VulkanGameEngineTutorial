#pragma once

#include <vulkan/vulkan_core.h>

#include <memory>

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

   void loadGameObjects(const char *);

  private:
   LveWindow lveWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
   LveDevice lveDevice{lveWindow};
   LveRenderer lveRenderer{lveWindow, lveDevice};

   std::unique_ptr<LveDescriptorPool> globalPool{};
   std::unique_ptr<LveDescriptorPool> imguiPool{};

   std::string path = "";

   std::unique_ptr<LveTerrain> terrain = nullptr;

   uint32_t xn = 0;
   uint32_t yn = 0;

   std::vector<std::vector<glm::float32>> altitudeMap = {};
};
}  // namespace lve
