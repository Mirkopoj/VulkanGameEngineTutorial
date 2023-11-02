#include "lve_imgui.hpp"

#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include <cstdio>
#include <imgui.h>

void ImGuiUi::init(GLFWwindow *window, ImGui_ImplVulkan_InitInfo *info,
                   VkRenderPass render_pass) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.Fonts->AddFontDefault();
  ImGui_ImplGlfw_InitForVulkan(window, true);
  ImGui_ImplVulkan_Init(info, render_pass);
  ImGui::StyleColorsDark();
}

void ImGuiUi::new_frame(VkCommandBuffer command_buffer) {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void ImGuiUi::update() {
  ImGui::Begin("Hola");
  ImGui::Text("Hello, world %d", 123);
  if (ImGui::Button("Save"))
	  printf("Boton\n");
  char buf[100]="";
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

void ImGuiUi::shutdown() {
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}
