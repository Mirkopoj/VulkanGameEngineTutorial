#pragma once

#include "lve_camera.hpp"
#include "lve_game_object.hpp"

#include <glm/fwd.hpp>
#include <vulkan/vulkan.h>

namespace lve {

#define MAX_LIGHTS 10

	struct PointLight {
		glm::vec4 position{};
		glm::vec4 color{};
	};

	struct GlobalUbo {
		glm::mat4 projection{1.f};
		glm::mat4 view{1.f};
		glm::vec4 ambienLightColor{1.f, 1.f, 1.f, .02f};
		PointLight pointLights[MAX_LIGHTS];
		int numLights;
	};

	struct FrameInfo {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		LveCamera &camera;
		VkDescriptorSet globalDescriptorSet;
		LveGameObject::Map &gameObjects;
	};

}
