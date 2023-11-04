#pragma once

#include "../lve/lve_camera.hpp"
#include "../lve/lve_pipeline.hpp"
#include "../lve/lve_device.hpp"
#include "../lve/lve_game_object.hpp"
#include "../lve/lve_frame_info.hpp"

#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace lve {

	class PointLightSystem {
		public:

			PointLightSystem(
					LveDevice &device,
					VkRenderPass renderPass,
					VkDescriptorSetLayout globalSetLayout);
			~PointLightSystem();

			PointLightSystem(const PointLightSystem &) = delete;
			PointLightSystem &operator=(const PointLightSystem &) = delete;

			void update(FrameInfo &frameInfo, GlobalUbo &ubo);
			void render(FrameInfo &frameInfo);

		private:
			void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
			void createPipeline(VkRenderPass renderPass);

  LveDevice &lveDevice;

  std::unique_ptr<LvePipeline> lvePipeline;
  VkPipelineLayout pipelineLayout;
};
} // namespace lve
