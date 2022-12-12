#pragma once

#include "lve_window.hpp"
#include "lve_device.hpp"
#include "lve_swap_chain.hpp"

#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <cassert>

namespace lve {

	class LveRenderer {
		public:

			LveRenderer(LveWindow &window, LveDevice &device);
			~LveRenderer();

			LveRenderer(const LveRenderer &) = delete;
			LveRenderer &operator=(const LveRenderer &) = delete;

			VkRenderPass getSwapChainRenderPass() const { return lveSwapChain->getRenderPass(); }
			bool isFrameInProgress() const { return isFrameStarted; }

			VkCommandBuffer getCurrentCommandBuffert() const { 
				assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
				return commandBuffers[currentImageIndex];
			}

			VkCommandBuffer beginFrame();
			void endFrame();
			void beginSwapChainRenmderPass(VkCommandBuffer commandBuffer);
			void endSwapChainRenmderPass(VkCommandBuffer commandBuffer);

		private:
			void createCommandBuffers();
			void freeCommandBuffers();
			void recreateSwapChain();

			LveWindow& lveWindow;
			LveDevice& lveDevice;
			std::unique_ptr<LveSwapChain> lveSwapChain;
			std::vector<VkCommandBuffer> commandBuffers;

			uint32_t currentImageIndex;
			bool isFrameStarted;

	};
}
