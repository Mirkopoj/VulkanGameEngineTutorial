#pragma once

#include <vulkan/vulkan_core.h>

#include <memory>

#include "../apps/second_app_frame_info.hpp"
#include "../lve/lve_device.hpp"
#include "../lve/lve_pipeline.hpp"

namespace lve {

class WindRenderSystem {
  public:
   WindRenderSystem(LveDevice &device, VkRenderPass renderPass,
                    VkDescriptorSetLayout globalSetLayout,
                    const std::string &vertFilepath,
                    const std::string &fragFilepath);
   ~WindRenderSystem();

   WindRenderSystem(const WindRenderSystem &) = delete;
   WindRenderSystem &operator=(const WindRenderSystem &) = delete;

   void renderWind(FrameInfo &frameInfo);

  private:
   void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
   void createPipeline(VkRenderPass renderPass,
                       const std::string &vertFilepath,
                       const std::string &fragFilepath);

   LveDevice &lveDevice;

   std::unique_ptr<LvePipeline> lvePipeline;
   VkPipelineLayout pipelineLayout;
};
}  // namespace lve
