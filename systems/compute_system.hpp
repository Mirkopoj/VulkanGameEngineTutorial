#pragma once

#include <vulkan/vulkan_core.h>

#include <memory>
#include <vector>

#include "../lve/lve_device.hpp"
#include "../lve/lve_pipeline.hpp"

namespace lve {

class ComputeSystem {
  public:
   ComputeSystem(LveDevice &device);
   ComputeSystem(ComputeSystem &&) = delete;
   ComputeSystem(const ComputeSystem &) = delete;
   ComputeSystem &operator=(ComputeSystem &&) = delete;
   ComputeSystem &operator=(const ComputeSystem &) = delete;
   ~ComputeSystem();

  private:
   LveDevice &lveDevice;
	std::unique_ptr<LvePipeline> lvePipeline;
   VkPipelineLayout pipelineLayout;
   void createPipelineLayout();
   void createPipeline();
};

}  // namespace lve
