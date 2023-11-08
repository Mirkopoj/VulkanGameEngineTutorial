#pragma once

#include <vector>

#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_vulkan.h"
#include "../lve/lve_device.hpp"
#include "../lve/lve_renderer.hpp"

class ImGuiUi {
  public:
   ImGuiUi(GLFWwindow *window, lve::LveDevice &lveDevice,
           lve::LveRenderer &lveRenderer, VkDescriptorPool imguiPool);
   ImGuiUi(ImGuiUi &&) = delete;
   ImGuiUi(const ImGuiUi &) = delete;
   ImGuiUi &operator=(ImGuiUi &&) = delete;
   ImGuiUi &operator=(const ImGuiUi &) = delete;
   ~ImGuiUi();

   void new_frame();
   void update();
   void render(VkCommandBuffer command_buffer);
};
