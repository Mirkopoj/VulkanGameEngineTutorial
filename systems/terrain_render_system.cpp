#include "terrain_render_system.hpp"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <glm/fwd.hpp>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <stdexcept>

namespace lve {

struct SimplePushConstantData {
   glm::mat4 modelMatrix{1.f};
   glm::mat4 normalMatrix{1.f};
};

TerrainRenderSystem::TerrainRenderSystem(
    LveDevice &device, VkRenderPass renderPass,
    VkDescriptorSetLayout globalSetLayout, const std::string &vertFilepath,
    const std::string &fragFilepath)
    : lveDevice{device} {
   createPipelineLayout(globalSetLayout);
   createPipeline(renderPass, vertFilepath, fragFilepath);
}

TerrainRenderSystem::~TerrainRenderSystem() {
   vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
}

void TerrainRenderSystem::createPipelineLayout(
    VkDescriptorSetLayout globalSetLayout) {
   VkPushConstantRange pushConstantRange{};
   pushConstantRange.stageFlags =
       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
   pushConstantRange.offset = 0;
   pushConstantRange.size = sizeof(SimplePushConstantData);

   std::vector<VkDescriptorSetLayout> descriptoSetLayouts{globalSetLayout};

   VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
   pipelineLayoutInfo.sType =
       VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
   pipelineLayoutInfo.setLayoutCount =
       static_cast<uint32_t>(descriptoSetLayouts.size());
   pipelineLayoutInfo.pSetLayouts = descriptoSetLayouts.data();
   pipelineLayoutInfo.pushConstantRangeCount = 1;
   pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

   if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo,
                              nullptr, &pipelineLayout) != VK_SUCCESS) {
      throw std::runtime_error("failed to create pipeline layout!");
   }
}

void TerrainRenderSystem::createPipeline(VkRenderPass renderPass,
                                         const std::string &vertFilepath,
                                         const std::string &fragFilepath) {
   assert(pipelineLayout != nullptr &&
          "Cannot create pipeline before pipeline layout");

   PipelineConfigInfo pipelineConfig{};
   LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
   pipelineConfig.inputAssemblyInfo.topology =
       VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
   pipelineConfig.bindingDescriptions =
       LveTerrain::Vertex::getBindingDescriptions();
   pipelineConfig.attributeDescriptions =
       LveTerrain::Vertex::getAttributeDescriptions();
   pipelineConfig.renderPass = renderPass;
   pipelineConfig.pipelineLayout = pipelineLayout;
   lvePipeline = std::make_unique<LvePipeline>(
       lveDevice, vertFilepath, fragFilepath, pipelineConfig);
}

void TerrainRenderSystem::renderTerrain(FrameInfo &frameInfo) {
   lvePipeline->bind(frameInfo.commandBuffer);

   vkCmdBindDescriptorSets(
       frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
       pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);

   SimplePushConstantData push{};
   const float c3 = glm::cos(.0f);
   const float s3 = glm::sin(.0f);
   const float c2 = glm::cos(.0f);
   const float s2 = glm::sin(.0f);
   const float c1 = glm::cos(.0f);
   const float s1 = glm::sin(.0f);
   push.modelMatrix = glm::mat4{{
                                    (c1 * c3 + s1 * s2 * s3),
                                    (c2 * s3),
                                    (c1 * s2 * s3 - c3 * s1),
                                    0.0f,
                                },
                                {
                                    (c3 * s1 * s2 - c1 * s3),
                                    (c2 * c3),
                                    (c1 * c3 * s2 + s1 * s3),
                                    0.0f,
                                },
                                {
                                    (c2 * s1),
                                    (-s2),
                                    (c1 * c2),
                                    0.0f,
                                },
                                {.0f, .0f, .0f, 1.0f}};
   ;
   push.normalMatrix = glm::mat3{
       {
           (c1 * c3 + s1 * s2 * s3),
           (c2 * s3),
           (c1 * s2 * s3 - c3 * s1),
       },
       {
           (c3 * s1 * s2 - c1 * s3),
           (c2 * c3),
           (c1 * c3 * s2 + s1 * s3),
       },
       {
           (c2 * s1),
           (-s2),
           (c1 * c2),
       },
   };

   vkCmdPushConstants(
       frameInfo.commandBuffer, pipelineLayout,
       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
       sizeof(SimplePushConstantData), &push);

   frameInfo.terrain.bind(frameInfo.commandBuffer);
   frameInfo.terrain.draw(frameInfo.commandBuffer);
}

}  // namespace lve
