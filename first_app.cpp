#include "first_app.hpp"
#include "lve_game_object.hpp"
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
#include <glm/gtc/constants.hpp>

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
		glm::mat2 transform{1.f};
		glm::vec2 offset;
		alignas(16) glm::vec3 color;
	};

	FirstApp::FirstApp(){
		loadGameOjects();
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

	void FirstApp::loadGameOjects() {
		std::vector<LveModel::Vertex> vertices = shierpinsk(4, {
			{{  0.0f,  -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{ -0.5f,   0.5f}, {0.0f, 1.0f, 0.0f}},
			{{  0.5f,   0.5f}, {0.0f, 0.0f, 1.0f}}
		});

		auto lveModel = std::make_shared<LveModel>(lveDevice, vertices);

		/*auto triangle = LveGameObject::crateGameObject();
		triangle.model = lveModel;
		triangle.color = {.1f, .8f, .1f};
		triangle.transform2d.translation.x = .2f;
		triangle.transform2d.scale = {2.f, .5f};
		triangle.transform2d.rotation = .25f * glm::two_pi<float>();

		gameObjects.push_back(std::move(triangle));*/

		std::vector<glm::vec3> colors{
				{1.f, .7f, .73f},
				{1.f, .87f, .73f},
				{1.f, 1.f, .73f},
				{.73f, 1.f, .8f},
				{.73, .88f, 1.f}  //
			};
		for (auto& color : colors) {
			color = glm::pow(color, glm::vec3{2.2f});
		}
		for (int i = 0; i < 40; i++) {
			auto triangle = LveGameObject::createGameObject();
			triangle.model = lveModel;
			triangle.transform2d.scale = glm::vec2(.5f) + i * 0.025f;
			triangle.transform2d.rotation = i * glm::pi<float>() * .025f;
			triangle.color = colors[i % colors.size()];
			gameObjects.push_back(std::move(triangle));
		}
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

		renderGameObjects(commandBuffers[imageIndex]);

		vkCmdEndRenderPass(commandBuffers[imageIndex]);
		if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void FirstApp::renderGameObjects(VkCommandBuffer commandBuffer) {

		int i = 0;
		for (auto& obj : gameObjects) {
			i += 1;
			obj.transform2d.rotation =
				glm::mod<float>(obj.transform2d.rotation + 0.00001f * i, 2.f * glm::pi<float>());
		}

		lvePipeline->bind(commandBuffer);

		for (auto& obj: gameObjects) {

			obj.transform2d.rotation = glm::mod(obj.transform2d.rotation+0.001f, glm::two_pi<float>());

			SimplePushConstantData push{};
			push.offset = obj.transform2d.translation;
			push.color = obj.color;
			push.transform = obj.transform2d.mat2();

			vkCmdPushConstants(
					commandBuffer,
					pipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					0,
					sizeof(SimplePushConstantData),
					&push);
			obj.model->bind(commandBuffer);
			obj.model->draw(commandBuffer);
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
