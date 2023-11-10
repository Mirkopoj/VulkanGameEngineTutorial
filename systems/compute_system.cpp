#include "compute_system.hpp"
#include <cassert>
#include <stdexcept>

namespace lve {

ComputeSystem::ComputeSystem(LveDevice &device) : lveDevice(device) {
	createPipelineLayout();
   createPipeline();

}

ComputeSystem::~ComputeSystem() {
   vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
}

void ComputeSystem::createPipelineLayout() {
   VkPushConstantRange pushConstantRange{};
   pushConstantRange.stageFlags =
       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
   pushConstantRange.offset = 0;
   //pushConstantRange.size = sizeof(PointLightPushConstants);

   //std::vector<VkDescriptorSetLayout> descriptoSetLayouts{globalSetLayout};

   VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
   pipelineLayoutInfo.sType =
       VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
   pipelineLayoutInfo.setLayoutCount =
       //static_cast<uint32_t>(descriptoSetLayouts.size());
   //pipelineLayoutInfo.pSetLayouts = descriptoSetLayouts.data();
   pipelineLayoutInfo.pushConstantRangeCount = 1;
   pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

   if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo,
                              nullptr, &pipelineLayout) != VK_SUCCESS) {
      throw std::runtime_error("failed to create pipeline layout!");
   }
}

void ComputeSystem::createPipeline() {
   assert(pipelineLayout != nullptr &&
          "Cannot create pipeline before pipeline layout");
   PipelineConfigInfo pipelineConfig{};
   LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
   LvePipeline::enableAlphaBlending(pipelineConfig);
   pipelineConfig.attributeDescriptions.clear();
   pipelineConfig.bindingDescriptions.clear();
   //pipelineConfig.renderPass = renderPass;
   pipelineConfig.pipelineLayout = pipelineLayout;
   lvePipeline = std::make_unique<LvePipeline>(
       lveDevice, "shaders/point_light.vert.spv",
       "shaders/point_light.frag.spv", pipelineConfig);
}

}  // namespace lve
