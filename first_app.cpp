#include "first_app.hpp"
#include "lve_model.hpp"
#include "lve_pipeline.hpp"
#include <GLFW/glfw3.h>
#include <array>
#include <cstdint>
#include <memory>
#include <sys/types.h>
#include <vector>
#include <vulkan/vulkan_core.h>

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

	FirstApp::FirstApp(){
		loadModels();
		createPipelineLayout();
		createPipeline();
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
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout)
				!= VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void FirstApp::createPipeline() {
		auto pipelineConfig =
			LvePipeline::defaultPipelineConfigInfo(lveSwapChain.width(), lveSwapChain.height());
		pipelineConfig.renderPass = lveSwapChain.getRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;
		lvePipeline = std::make_unique<LvePipeline>(
				lveDevice,
				"shaders/simple_shader.vert.spv",
				"shaders/simple_shader.frag.spv",
				pipelineConfig);
	}

	void FirstApp::createCommandBuffers() {
		commandBuffers.resize(lveSwapChain.imageCount());
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = lveDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if(vkAllocateCommandBuffers(lveDevice.device(), &allocInfo, commandBuffers.data())
				!= VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers");
		}

		for (int i=0;i<commandBuffers.size();i++){
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo)
					!= VK_SUCCESS){
				throw std::runtime_error("failed to begin recording command buffers");
			}

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = lveSwapChain.getRenderPass();
			renderPassInfo.framebuffer = lveSwapChain.getFrameBuffer(i);

			renderPassInfo.renderArea.offset = {0, 0};
			renderPassInfo.renderArea.extent = lveSwapChain.getSwapChainExtent();

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
			clearValues[1].depthStencil = {1.0f, 0};
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			lvePipeline->bind(commandBuffers[i]);
			lveModel->bind(commandBuffers[i]);
			lveModel->draw(commandBuffers[i]);

			vkCmdEndRenderPass(commandBuffers[i]);
			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer!");
			}
		}
	}

	void FirstApp::drawFrame() {
		uint32_t imageIndex;
		auto result = lveSwapChain.acquireNextImage(&imageIndex);

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		result = lveSwapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}
	}

}
