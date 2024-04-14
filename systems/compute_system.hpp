#pragma once

#include <vulkan/vulkan_core.h>

#include "../lve/lve_device.hpp"

namespace lve {

class ComputeSystem {
  public:
   ComputeSystem(LveDevice &device,
                 const std::vector<VkDescriptorSetLayout>,
                 const std::string &);
   ComputeSystem(ComputeSystem &&) = delete;
   ComputeSystem(const ComputeSystem &) = delete;
   ComputeSystem &operator=(ComputeSystem &&) = delete;
   ComputeSystem &operator=(const ComputeSystem &) = delete;
   ~ComputeSystem();

   VkPipeline computePipeline;
   VkPipelineLayout pipelineLayout;
  private:
   LveDevice &lveDevice;
   VkShaderModule module;
   void createPipelineLayout(const std::vector<VkDescriptorSetLayout>);
   void createPipeline();
   void createShaderModule(const std::string &);
   std::vector<char> readFile(const std::string &filepath);
};

}  // namespace lve
