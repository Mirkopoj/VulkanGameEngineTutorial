#include "imgui_system.hpp"

#include "imgui.h"
#include "imgui/misc/cpp/imgui_stdlib.h"
#include <vulkan/vulkan_core.h>

#include <cstdio>

#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"
#include "lve/lve_renderer.hpp"

const char *VkResultToCString(VkResult result);

static void CheckVkResult(VkResult err);

ImGuiUi::ImGuiUi(GLFWwindow *window, lve::LveDevice &lveDevice,
                 lve::LveRenderer &lveRenderer, VkDescriptorPool imguiPool)
    : state({
          .buf = "",
          .f = 0.0,
      }) {
   ImGui_ImplVulkan_InitInfo info = {};
   info.Instance = lveDevice.get_instance();
   info.PhysicalDevice = lveDevice.physical_device();
   info.Device = lveDevice.device();
   info.QueueFamily = lveDevice.findPhysicalQueueFamilies().presentFamily;
   info.Queue = lveDevice.presentQueue();
   info.PipelineCache = VK_NULL_HANDLE;
   info.DescriptorPool = imguiPool;
   info.Subpass = 0;
   info.MinImageCount = lveRenderer.getSwapChainImageCount();
   info.ImageCount = lveRenderer.getSwapChainImageCount();
   info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
   info.Allocator = nullptr;
   info.CheckVkResultFn = CheckVkResult;
	info.RenderPass = lveRenderer.getSwapChainRenderPass();

   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGuiIO &io = ImGui::GetIO();
   io.Fonts->AddFontDefault();
   ImGui_ImplGlfw_InitForVulkan(window, true);
   ImGui_ImplVulkan_Init(&info);
   ImGui_ImplVulkan_CreateFontsTexture();
   ImGui_ImplVulkan_DestroyFontsTexture();
   ImGui::StyleColorsDark();
}

void ImGuiUi::new_frame() {
   ImGui_ImplVulkan_NewFrame();
   ImGui_ImplGlfw_NewFrame();
   ImGui::NewFrame();
}

void ImGuiUi::update(MyTextureData *i_img, MyTextureData *o_img) {
   ImGui::ShowDemoWindow();

   ImGui::Begin("Hola");
   ImGui::Text("Hello, world %d", 123);
   if (ImGui::Button("Save")) printf("Boton\n");
   ImGui::InputText("string", &state.buf);
   ImGui::SliderFloat("float", &state.f, 0.0f, 1.0f);
   ImGui::Text("%f, %s", state.f, state.buf.c_str());
   ImGui::End();

   ImGui::Begin("Antes");
   ImGui::Image((ImTextureID)i_img->DS,
                ImVec2(i_img->Width, i_img->Height));
   ImGui::End();

   ImGui::Begin("Despues");
   ImGui::Image((ImTextureID)o_img->DS,
                ImVec2(o_img->Width, o_img->Height));
   ImGui::End();

   ImGui::Begin("Shaders");
   ImGui::Text("Cantidad");
   ImGui::RadioButton("0", &state.shader_count, 0);
   ImGui::SameLine();
   ImGui::RadioButton("1", &state.shader_count, 1);
   ImGui::SameLine();
   ImGui::RadioButton("2", &state.shader_count, 2);
   if (state.shader_count > 0) {
      ImGui::Text("Primero");
      ImGui::RadioButton("Bordes", &state.first_shader, 0);
      ImGui::SameLine();
      ImGui::RadioButton("Borroso", &state.first_shader, 1);
   }
   if (state.shader_count > 1) {
      ImGui::Text("Segundo");
		ImGui::PushID(0);
      ImGui::RadioButton("Bordes", &state.second_shader, 0);
      ImGui::SameLine();
      ImGui::RadioButton("Borroso", &state.second_shader, 1);
		ImGui::PopID();
   }
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

const char *VkResultToCString(VkResult result) {
   switch (result) {
      case VK_SUCCESS:
         return "VK_SUCCESS";
      case VK_NOT_READY:
         return "VK_NOT_READY";
      case VK_TIMEOUT:
         return "VK_TIMEOUT";
      case VK_EVENT_SET:
         return "VK_EVENT_SET";
      case VK_EVENT_RESET:
         return "VK_EVENT_RESET";
      case VK_INCOMPLETE:
         return "VK_INCOMPLETE";
      case VK_ERROR_OUT_OF_HOST_MEMORY:
         return "VK_ERROR_OUT_OF_HOST_MEMORY";
      case VK_ERROR_OUT_OF_DEVICE_MEMORY:
         return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
      case VK_ERROR_INITIALIZATION_FAILED:
         return "VK_ERROR_INITIALIZATION_FAILED";
      case VK_ERROR_DEVICE_LOST:
         return "VK_ERROR_DEVICE_LOST";
      case VK_ERROR_MEMORY_MAP_FAILED:
         return "VK_ERROR_MEMORY_MAP_FAILED";
      case VK_ERROR_LAYER_NOT_PRESENT:
         return "VK_ERROR_LAYER_NOT_PRESENT";
      case VK_ERROR_EXTENSION_NOT_PRESENT:
         return "VK_ERROR_EXTENSION_NOT_PRESENT";
      case VK_ERROR_FEATURE_NOT_PRESENT:
         return "VK_ERROR_FEATURE_NOT_PRESENT";
      case VK_ERROR_INCOMPATIBLE_DRIVER:
         return "VK_ERROR_INCOMPATIBLE_DRIVER";
      case VK_ERROR_TOO_MANY_OBJECTS:
         return "VK_ERROR_TOO_MANY_OBJECTS";
      case VK_ERROR_FORMAT_NOT_SUPPORTED:
         return "VK_ERROR_FORMAT_NOT_SUPPORTED";
      case VK_ERROR_FRAGMENTED_POOL:
         return "VK_ERROR_FRAGMENTED_POOL";
      case VK_ERROR_UNKNOWN:
         return "VK_ERROR_UNKNOWN";
      case VK_ERROR_OUT_OF_POOL_MEMORY:
         return "VK_ERROR_OUT_OF_POOL_MEMORY";
      case VK_ERROR_INVALID_EXTERNAL_HANDLE:
         return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
      case VK_ERROR_FRAGMENTATION:
         return "VK_ERROR_FRAGMENTATION";
      case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
         return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
      case VK_PIPELINE_COMPILE_REQUIRED:
         return "VK_PIPELINE_COMPILE_REQUIRED";
      case VK_ERROR_SURFACE_LOST_KHR:
         return "VK_ERROR_SURFACE_LOST_KHR";
      case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
         return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
      case VK_SUBOPTIMAL_KHR:
         return "VK_SUBOPTIMAL_KHR";
      case VK_ERROR_OUT_OF_DATE_KHR:
         return "VK_ERROR_OUT_OF_DATE_KHR";
      case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
         return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
      case VK_ERROR_VALIDATION_FAILED_EXT:
         return "VK_ERROR_VALIDATION_FAILED_EXT";
      case VK_ERROR_INVALID_SHADER_NV:
         return "VK_ERROR_INVALID_SHADER_NV";
#ifdef VK_ENABLE_BETA_EXTENSIONS
      case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR:
         return "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
      case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR:
         return "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
      case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR:
         return "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
      case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR:
         return "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
      case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR:
         return "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
      case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR:
         return "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";
#endif
      case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
         return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
      case VK_ERROR_NOT_PERMITTED_KHR:
         return "VK_ERROR_NOT_PERMITTED_KHR";
      case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
         return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
      case VK_THREAD_IDLE_KHR:
         return "VK_THREAD_IDLE_KHR";
      case VK_THREAD_DONE_KHR:
         return "VK_THREAD_DONE_KHR";
      case VK_OPERATION_DEFERRED_KHR:
         return "VK_OPERATION_DEFERRED_KHR";
      case VK_OPERATION_NOT_DEFERRED_KHR:
         return "VK_OPERATION_NOT_DEFERRED_KHR";
      case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:
         return "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
      case VK_RESULT_MAX_ENUM:
         return "VK_RESULT_MAX_ENUM";
      default:
         return "??????";
   }
}
