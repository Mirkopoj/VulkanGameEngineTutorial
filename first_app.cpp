#include "first_app.hpp"
#include "simple_render_system.hpp"

#include "lve_game_object.hpp"
#include "lve_model.hpp"
#include "lve_swap_chain.hpp"
#include <GLFW/glfw3.h>
#include <cstdint>
#include <memory>
#include <sys/types.h>
#include <vector>
#include <vulkan/vulkan_core.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <cassert>
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
		loadGameOjects();
	}

	FirstApp::~FirstApp(){ }

	void FirstApp::run() {
		SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass()};

		while (!lveWindow.shouldClose()){
			glfwPollEvents();

			if (auto commandBuffer = lveRenderer.beginFrame()){
				lveRenderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);
				lveRenderer.endSwapChainRenderPass(commandBuffer);
				lveRenderer.endFrame();
			}
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

}
