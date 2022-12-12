#include "first_app.hpp"
#include "lve_model.hpp"
#include "lve_pipeline.hpp"
#include "lve_swap_chain.hpp"
#include <GLFW/glfw3.h>
#include <array>
#include <cassert>
#include <cstdint>
#include <memory>
#include <sys/types.h>
#include <vector>
#include <vulkan/vulkan_core.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <stdexcept>

std::vector<lve::LveModel::Vertex> shierpinsk(uint iter, std::vector<lve::LveModel::Vertex> inicial) {
	if (iter == 0) return inicial;
	lve::LveModel::Vertex A = inicial[0];
	lve::LveModel::Vertex B = inicial[1];
	lve::LveModel::Vertex C = inicial[2];
	lve::LveModel::Vertex D;
	lve::LveModel::Vertex E;
	lve::LveModel::Vertex F;
	D.position = (A.position+B.position);
	E.position = (A.position+C.position);
	F.position = (C.position+B.position);
	D.position /=2;
	E.position /=2;
	F.position /=2;

	std::vector<lve::LveModel::Vertex> t1;
	std::vector<lve::LveModel::Vertex> t2;
	std::vector<lve::LveModel::Vertex> t3;
	D.color = {0.0f, 1.0f, 0.0f};
	E.color = {0.0f, 0.0f, 1.0f};
	t1.push_back(A);
	t1.push_back(D);
	t1.push_back(E);
	D.color = {1.0f, 0.0f, 0.0f};
	F.color = {0.0f, 0.0f, 1.0f};
	t2.push_back(D);
	t2.push_back(B);
	t2.push_back(F);
	E.color = {1.0f, 0.0f, 0.0f};
	F.color = {0.0f, 1.0f, 0.0f};
	t3.push_back(E);
	t3.push_back(F);
	t3.push_back(C);


	std::vector<lve::LveModel::Vertex> ret;
	iter--;
	t1 = shierpinsk(iter, t1);
	t2 = shierpinsk(iter, t2);
	t3 = shierpinsk(iter, t3);
	ret.insert( ret.end(), t1.begin(), t1.end() );
	ret.insert( ret.end(), t2.begin(), t2.end() );
	ret.insert( ret.end(), t3.begin(), t3.end() );

	return ret;

}

namespace lve {

	struct SimplePushConstantData {
		glm::vec2 offset;
		alignas(16) glm::vec3 color;
	};

	FirstApp::FirstApp(){
		loadModels();
		createPipelineLayout();
		recreateSwapChain();
		createCommandBuffers();
	}

	FirstApp::~FirstApp(){
		vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
	}

	void FirstApp::run() {
		uint cont = 0;

		while (!lveWindow.shouldClose()){
			cont ++;
			glfwPollEvents();
			drawFrame();
		}

		vkDeviceWaitIdle(lveDevice.device());
	}

	void FirstApp::loadModels() {
		std::vector<LveModel::Vertex> vertices = shierpinsk(4, {
			{{  0.0f,  -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{ -0.5f,   0.5f}, {0.0f, 1.0f, 0.0f}},
			{{  0.5f,   0.5f}, {0.0f, 0.0f, 1.0f}}
		});

		lveModel = std::make_unique<LveModel>(lveDevice, vertices);
	}

	void FirstApp::createPipelineLayout() {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout)
				!= VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void FirstApp::createPipeline() {
		assert(lveSwapChain != nullptr && "Cannot create pipeline before swap chain");
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = lveSwapChain->getRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;
		lvePipeline = std::make_unique<LvePipeline>(
				lveDevice,
				"shaders/simple_shader.vert.spv",
				"shaders/simple_shader.frag.spv",
				pipelineConfig);
	}

	void FirstApp::recreateSwapChain(){
		auto extent = lveWindow.getExtent();
		while (extent.width == 0 || extent.height == 0) {
			extent = lveWindow.getExtent();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(lveDevice.device());

		if (lveSwapChain == nullptr){
			lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent);
		} else {
			lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent, std::move(lveSwapChain));
			if (lveSwapChain->imageCount() != commandBuffers.size()) {
				freeCommandBuffers();
				createCommandBuffers();
			}
		}
		createPipeline();

	}

	void FirstApp::createCommandBuffers() {
		commandBuffers.resize(lveSwapChain->imageCount());
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = lveDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if(vkAllocateCommandBuffers(lveDevice.device(), &allocInfo, commandBuffers.data())
				!= VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers");
		}

	}

	void FirstApp::freeCommandBuffers(){
		vkFreeCommandBuffers(
				lveDevice.device(),
				lveDevice.getCommandPool(),
				static_cast<uint32_t>(commandBuffers.size()),
				commandBuffers.data());
		commandBuffers.clear();
	}

	void FirstApp::recordCommandBuffer(int imageIndex){
		static int frame = 0;
		frame = (frame+1) % 20000;
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo)
				!= VK_SUCCESS){
			throw std::runtime_error("failed to begin recording command buffers");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = lveSwapChain->getRenderPass();
		renderPassInfo.framebuffer = lveSwapChain->getFrameBuffer(imageIndex);

		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = lveSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
		clearValues[1].depthStencil = {1.0f, 0};
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(
				commandBuffers[imageIndex],
				&renderPassInfo,
				VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(lveSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(lveSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{{0, 0}, lveSwapChain->getSwapChainExtent()};
		vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
		vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

		lvePipeline->bind(commandBuffers[imageIndex]);
		lveModel->bind(commandBuffers[imageIndex]);

		for (int j=0;j<4;j++){
			SimplePushConstantData push{};
			push.offset = {-1.0f + frame*0.0002f, -0.4 + j*0.25f};
			float R = j==0? 0.2f:0.0f;
			float G = j==1? 0.4f:0.0f;
			float B = j==2? 0.6f:0.0f;
			R = j==3? 0.8f:R;
			G = j==3? 0.8f:G;
			B = j==3? 0.8f:B;
			push.color = {R, G, B};

			vkCmdPushConstants(
					commandBuffers[imageIndex],
					pipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					0,
					sizeof(SimplePushConstantData),
					&push);

			lveModel->draw(commandBuffers[imageIndex]);

		}

		vkCmdEndRenderPass(commandBuffers[imageIndex]);
		if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void FirstApp::drawFrame() {
		uint32_t imageIndex;
		auto result = lveSwapChain->acquireNextImage(&imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		recordCommandBuffer(imageIndex);
		result = lveSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
		if (	result == VK_ERROR_OUT_OF_DATE_KHR || 
				result == VK_SUBOPTIMAL_KHR ||
				lveWindow.wasWindowResized()) {
			lveWindow.reserWindowResizedFlag();
			recreateSwapChain();
			return;
		} 
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}
	}

}
