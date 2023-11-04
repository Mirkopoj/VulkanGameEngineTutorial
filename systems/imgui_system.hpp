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
   ImGuiUi(ImGuiUi &&) = default;
   ImGuiUi(const ImGuiUi &) = default;
   ImGuiUi &operator=(ImGuiUi &&) = default;
   ImGuiUi &operator=(const ImGuiUi &) = default;
   ~ImGuiUi();

   void new_frame();
   void update();
   void render(VkCommandBuffer command_buffer);
};
