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

   SecondApp(int, int);
   ~SecondApp();

   SecondApp(const SecondApp &) = delete;
   SecondApp &operator=(const SecondApp &) = delete;

   void run();

  private:
   void loadGameObjects();

   LveWindow lveWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
   LveDevice lveDevice{lveWindow};
   LveRenderer lveRenderer{lveWindow, lveDevice};

   std::unique_ptr<LveDescriptorPool> globalPool{};
   std::unique_ptr<LveTerrain> terrain;
	int xn;
	int yn;
};
}  // namespace lve
