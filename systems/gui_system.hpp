#pragma once

#include <cstring>
#include <set>

#include "../lve/lve_device.hpp"
#include "../lve/lve_renderer.hpp"
#include "../movement_controllers/terrain_movement_controller.hpp"

const char *vk_result_to_c_string(VkResult result);

static void check_vk_result(VkResult err) {
   if (err == 0) return;
   fprintf(stderr, "[vulkan] Error: VkResult = %s\n",
           vk_result_to_c_string(err));
   if (err < 0) abort();
}

class ImGuiGui {
  public:
   ImGuiGui(GLFWwindow *window, lve::LveDevice &lveDevice,
            lve::LveRenderer &lveRenderer, VkDescriptorPool imguiPool);
   ImGuiGui(ImGuiGui &&) = delete;
   ImGuiGui(const ImGuiGui &) = delete;
   ImGuiGui &operator=(ImGuiGui &&) = delete;
   ImGuiGui &operator=(const ImGuiGui &) = delete;
   ~ImGuiGui();

   void new_frame();
   void update(lve::TerrainMovementController &, bool &, std::string &,
               const std::set<std::string> &, int &, bool &, size_t &,
               glm::vec3, bool &);
   void render(VkCommandBuffer command_buffer);
};
