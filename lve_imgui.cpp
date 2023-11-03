#include "lve_imgui.hpp"

#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "lve_renderer.hpp"
#include "lve_swap_chain.hpp"
#include <cstdio>
#include <imgui.h>
#include <vector>
#include <vulkan/vulkan_core.h>

ImGuiUi::ImGuiUi(GLFWwindow *window, ImGui_ImplVulkan_InitInfo *info,
                   VkRenderPass render_pass, lve::LveRenderer &lveRenderer) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.Fonts->AddFontDefault();
  ImGui_ImplGlfw_InitForVulkan(window, true);
  ImGui_ImplVulkan_Init(info, render_pass);
  for (int i = 0; i < lve::LveSwapChain::MAX_FRAMES_IN_FLIGHT; ++i) {
    if (auto commandBuffer = lveRenderer.beginFrame()) {
      ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    }
    lveRenderer.endFrame();
  }
  ImGui_ImplVulkan_DestroyFontUploadObjects();
  ImGui::StyleColorsDark();
}

void ImGuiUi::new_frame() {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void ImGuiUi::update() {
  ImGui::Begin("Hola");
  ImGui::Text("Hello, world %d", 123);
  if (ImGui::Button("Save"))
    printf("Boton\n");
  char buf[100] = "";
  ImGui::InputText("string", buf, IM_ARRAYSIZE(buf));
  float f;
  ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
  ImGui::Text("%f, %s", f, buf);
  ImGui::End();
}

void ImGuiUi::render(VkCommandBuffer command_buffer) {
  ImGui::Render();
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);
}

ImGuiUi::~ImGuiUi() {
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}
