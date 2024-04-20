#include "compute_system.hpp"

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace lve {

ComputeSystem::ComputeSystem(
    LveDevice &device,
    const std::vector<VkDescriptorSetLayout> desc_layout,
    const std::string &compFilepath)
    : lveDevice(device),
      CmdBuffer{},
      submitInfo{
          .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
          .pNext = nullptr,
          .waitSemaphoreCount = 0,
          .pWaitSemaphores = nullptr,
          .pWaitDstStageMask = nullptr,
          .commandBufferCount = 1,
          .pCommandBuffers = &CmdBuffer,
          .signalSemaphoreCount = 0,
          .pSignalSemaphores = nullptr,
      } {
   createFence();
   createPipelineLayout(desc_layout);
   createShaderModule(compFilepath);
   createPipeline();
}

ComputeSystem::~ComputeSystem() {
   vkDestroyPipeline(lveDevice.device(), computePipeline, nullptr);
   vkDestroyShaderModule(lveDevice.device(), module, nullptr);
   vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
   vkDestroyFence(lveDevice.device(), Fence, nullptr);
}

void ComputeSystem::createFence() {
   VkFenceCreateInfo FenceCreateInfo = {
       .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
       .pNext = nullptr,
       .flags = 0,
   };

   if (vkCreateFence(lveDevice.device(), &FenceCreateInfo, nullptr,
                     &this->Fence)) {
      throw std::runtime_error("failed to create pipeline layout!");
   }
}

void ComputeSystem::createPipelineLayout(
    const std::vector<VkDescriptorSetLayout> desc_layout) {
   VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
   pipelineLayoutCreateInfo.sType =
       VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
   pipelineLayoutCreateInfo.pNext = nullptr;
   pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
   pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
   pipelineLayoutCreateInfo.setLayoutCount = desc_layout.size();
   pipelineLayoutCreateInfo.pSetLayouts = desc_layout.data();

   if (vkCreatePipelineLayout(lveDevice.device(),
                              &pipelineLayoutCreateInfo, nullptr,
                              &pipelineLayout) != VK_SUCCESS) {
      throw std::runtime_error("failed to create pipeline layout!");
   }
}

void ComputeSystem::createPipeline() {
   assert(pipelineLayout != nullptr &&
          "Cannot create pipeline before pipeline layout");
   VkPipelineShaderStageCreateInfo stageInfo = {};
   stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
   stageInfo.pNext = nullptr;
   stageInfo.flags = 0;
   stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
   stageInfo.module = module;
   stageInfo.pName = "main";
   stageInfo.pSpecializationInfo = nullptr;

   VkComputePipelineCreateInfo pipelineCreateInfo = {};
   pipelineCreateInfo.sType =
       VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
   pipelineCreateInfo.pNext = nullptr;
   pipelineCreateInfo.flags = 0;
   pipelineCreateInfo.stage = stageInfo;
   pipelineCreateInfo.layout = pipelineLayout;
   pipelineCreateInfo.basePipelineIndex = -1;
   pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

   if (vkCreateComputePipelines(lveDevice.device(), VK_NULL_HANDLE, 1,
                                &pipelineCreateInfo, nullptr,
                                &computePipeline) != VK_SUCCESS) {
      throw std::runtime_error("failed to create compute pipeline");
   }
}

void ComputeSystem::createShaderModule(const std::string &compFilepath) {
   auto compCode = readFile(compFilepath);
   VkShaderModuleCreateInfo createInfo{};
   createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
   createInfo.codeSize = compCode.size();
   createInfo.pCode = reinterpret_cast<const uint32_t *>(compCode.data());

   if (vkCreateShaderModule(lveDevice.device(), &createInfo, nullptr,
                            &module) != VK_SUCCESS) {
      throw std::runtime_error("failed to create shader module");
   }
}

std::vector<char> ComputeSystem::readFile(const std::string &filepath) {
   std::ifstream file{filepath, std::ios::ate | std::ios::binary};

   if (!file.is_open()) {
      throw std::runtime_error("failed to open file: " + filepath);
   }

   size_t fileSize = static_cast<size_t>(file.tellg());
   std::vector<char> buffer(fileSize);

   file.seekg(0);
   file.read(buffer.data(), fileSize);

   file.close();

   return buffer;
}

void ComputeSystem::dispatch(int width, int height, int channels,
                             VkDescriptorSet &DescriptorSet) {
   CmdBuffer = lveDevice.beginSingleTimeCommands();
   vkCmdBindPipeline(CmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                     this->computePipeline);
   vkCmdBindDescriptorSets(CmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                           this->pipelineLayout, 0, 1, &DescriptorSet, 0,
                           nullptr);
   vkCmdDispatch(CmdBuffer, width, height, channels);
   vkEndCommandBuffer(CmdBuffer);
}

void ComputeSystem::await() {
   VkQueue Queue = lveDevice.computeQueue();
   vkResetFences(lveDevice.device(), 1, &this->Fence);
   vkQueueSubmit(Queue, 1, &this->submitInfo, this->Fence);
   vkWaitForFences(lveDevice.device(), 1, &this->Fence, true,
                   uint64_t(-1));
}

void ComputeSystem::instant_dispatch(int width, int height, int channels,
                                     VkDescriptorSet &DescriptorSet) {
   dispatch(width, height, channels, DescriptorSet);
   await();
}

}  // namespace lve
