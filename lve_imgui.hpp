#pragma once

#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "lve_device.hpp"
#include "lve_renderer.hpp"
#include <vector>

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
