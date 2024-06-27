#include "gui_system.hpp"

#include <imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <nfd.h>
#include <vulkan/vulkan_core.h>

#include <cstdio>
#include <set>
#include <stdexcept>

#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"
#include "lve/lve_renderer.hpp"

const char *vk_result_to_c_string(VkResult result);

static void check_vk_result(VkResult err);

ImGuiGui::ImGuiGui(GLFWwindow *window, lve::LveDevice &lveDevice,
                   lve::LveRenderer &lveRenderer,
                   VkDescriptorPool imguiPool) {
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
   info.CheckVkResultFn = check_vk_result;
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

void ImGuiGui::new_frame() {
   ImGui_ImplVulkan_NewFrame();
   ImGui_ImplGlfw_NewFrame();
   ImGui::NewFrame();
}

void ImGuiGui::update(lve::TerrainMovementController &cameraControler,
                      bool &caminata, std::string &path,
                      const std::set<std::string> &recent, int &curr,
                      bool &loadingState, size_t &pipeline,
                      glm::vec3 coord, bool &viento, int &paleta_viento,
                      const char *paletas) {
   ImGui::Begin("Sensibilidad");
   ImGui::SliderFloat("Velocidad minima", &cameraControler.moveSpeedMin,
                      0.1f, cameraControler.moveSpeedMax);
   ImGui::SliderFloat("Velocidad maxima", &cameraControler.moveSpeedMax,
                      cameraControler.moveSpeedMin, 500.f);
   ImGui::SliderFloat("Sensibilidad del mouse", &cameraControler.lookSpeed,
                      0.1f, 20.f);
   ImGui::Text("position: %f, %f, %f", coord.x, coord.y, coord.z);
   ImGui::End();

   int caminata_i = caminata;
   ImGui::Begin("Modo de movimiento");
   ImGui::RadioButton("Caminata", &caminata_i, 1);
   ImGui::SameLine();
   ImGui::RadioButton("Vuelo", &caminata_i, 0);
   ImGui::End();
   caminata = caminata_i;

   int pipeline_i = pipeline;
   ImGui::Begin("Modo de rederizado");
   ImGui::RadioButton("Normal", &pipeline_i, 0);
   ImGui::SameLine();
   ImGui::RadioButton("WireFrame", &pipeline_i, 1);
   ImGui::Checkbox("Ver viento", &viento);
   ImGui::Combo("Paleta viento", &paleta_viento, paletas);
   ImGui::End();
   pipeline = pipeline_i;

   ImGui::Begin("Proyect selector");
   if (!loadingState) {
      const char *list[recent.size()];
      int i = 0;
      for (const std::string &str : recent) {
         list[i] = str.c_str();
         ++i;
      }
      int prev = curr;
      if (ImGui::Button("Open new proyect")) {
         nfdu8char_t *outPath;
         nfdresult_t result = NFD_PickFolder(&outPath, NULL);
         if (result == NFD_OKAY) {
            path = outPath;
            NFD_FreePath(outPath);
         } else {
            throw new std::runtime_error(NFD_GetError());
         }
      }
      ImGui::ListBox("recent", &curr, list, recent.size());
      if (prev != curr) {
         path = list[curr];
      }
   } else {
      ImGui::Text("Loading terrain");
   }
   ImGui::End();
}

void ImGuiGui::render(VkCommandBuffer command_buffer) {
   ImGui::Render();
   ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);
}

ImGuiGui::~ImGuiGui() {
   ImGui_ImplVulkan_Shutdown();
   ImGui_ImplGlfw_Shutdown();
   ImGui::DestroyContext();
}

const char *vk_result_to_c_string(VkResult result) {
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
