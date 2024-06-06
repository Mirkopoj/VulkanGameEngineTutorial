#pragma once

#include <vulkan/vulkan_core.h>

#include <memory>

#include "../lve/lve_device.hpp"
#include "../apps/first_app_frame_info.hpp"
#include "../lve/lve_pipeline.hpp"

namespace lve {

class SimpleRenderSystem {
  public:
   SimpleRenderSystem(LveDevice &device, VkRenderPass renderPass,
                      VkDescriptorSetLayout globalSetLayout);
   ~SimpleRenderSystem();

   SimpleRenderSystem(const SimpleRenderSystem &) = delete;
   SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

   void renderGameObjects(FrameInfo &frameInfo);

  private:
   void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
   void createPipeline(VkRenderPass renderPass);

   LveDevice &lveDevice;

   std::unique_ptr<LvePipeline> lvePipeline;
   VkPipelineLayout pipelineLayout;
};
}  // namespace lve
