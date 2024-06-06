#pragma once

#include <vulkan/vulkan_core.h>

#include <memory>

#include "../apps/second_app_frame_info.hpp"
#include "../lve/lve_device.hpp"
#include "../lve/lve_pipeline.hpp"

namespace lve {

class TerrainRenderSystem {
  public:
   enum class PipeLineType {
      Normal,
      WireFrame,
   };

   TerrainRenderSystem(LveDevice &device, VkRenderPass renderPass,
                       VkDescriptorSetLayout globalSetLayout,
                       const std::string &vertFilepath,
                       const std::string &fragFilepath);
   ~TerrainRenderSystem();

   TerrainRenderSystem(const TerrainRenderSystem &) = delete;
   TerrainRenderSystem &operator=(const TerrainRenderSystem &) = delete;

   void renderTerrain(FrameInfo &frameInfo, PipeLineType pipeline);

  private:
   void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
   void createPipeline(VkRenderPass renderPass,
                       const std::string &vertFilepath,
                       const std::string &fragFilepath,
                       PipeLineType pipeline);

   LveDevice &lveDevice;

   std::unique_ptr<LvePipeline> lvePipeline[2];
   VkPipelineLayout pipelineLayout;
};
}  // namespace lve
