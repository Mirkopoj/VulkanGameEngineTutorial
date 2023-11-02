#pragma once

#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

class ImGuiUi {
public:
	ImGuiUi() = default;
	ImGuiUi(ImGuiUi &&) = default;
	ImGuiUi(const ImGuiUi &) = default;
	ImGuiUi &operator=(ImGuiUi &&) = default;
	ImGuiUi &operator=(const ImGuiUi &) = default;
	~ImGuiUi() = default;

	void init(GLFWwindow *window, ImGui_ImplVulkan_InitInfo *info, VkRenderPass render_pass);
	void new_frame(VkCommandBuffer command_buffer);
	void update();
	void render(VkCommandBuffer command_buffer);
	void shutdown();
	
};


