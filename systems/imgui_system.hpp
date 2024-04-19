#pragma once

#include <cstring>

#include "../lve/lve_device.hpp"
#include "../lve/lve_renderer.hpp"

struct MyTextureData {
   VkDescriptorSet
       DS;  // Descriptor set: this is what you'll pass to Image()
   int Width;
   int Height;
   int Channels;

   // Need to keep track of these to properly cleanup
   VkImageView ImageView;
   VkImage Image;
   VkDeviceMemory ImageMemory;
   VkSampler Sampler;
   VkBuffer UploadBuffer;
   VkDeviceMemory UploadBufferMemory;

   MyTextureData() {
      memset(this, 0, sizeof(*this));
   }
};

const char *VkResultToCString(VkResult result);

static void check_vk_result(VkResult err) {
   if (err == 0) return;
   fprintf(stderr, "[vulkan] Error: VkResult = %s\n",
           VkResultToCString(err));
   if (err < 0) abort();
}

bool LoadTextureFromFile(const char *filename, MyTextureData *tex_data,
                         lve::LveDevice &device);
void RemoveTexture(MyTextureData *tex_data, lve::LveDevice &device);

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
   void update(MyTextureData *i_img, MyTextureData *o_img);
   void render(VkCommandBuffer command_buffer);

   int get_shader_count() {
      return state.shader_count;
   }
   int get_first_shader() {
      return state.first_shader;
   }
   int get_second_shader() {
      return state.second_shader;
   }

  private:
   typedef struct {
      std::string buf;
      float f;
      int shader_count;
      int first_shader;
      int second_shader;
   } State;

   State state;
};
