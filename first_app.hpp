#pragma once

#include "lve/lve_window.hpp"
#include "lve/lve_device.hpp"
#include "lve/lve_game_object.hpp"
#include "lve/lve_renderer.hpp"
#include "lve/lve_descriptors.hpp"

#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace lve {

	class FirstApp {
		public:
			static constexpr int WIDTH = 800;
			static constexpr int HEIGHT = 600;

			FirstApp();
			~FirstApp();

			FirstApp(const FirstApp &) = delete;
			FirstApp &operator=(const FirstApp &) = delete;

			void run();

		private:
			void loadGameObjects();

			LveWindow lveWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
			LveDevice lveDevice{lveWindow};
			LveRenderer lveRenderer{lveWindow, lveDevice};

			std::unique_ptr<LveDescriptorPool> globalPool{};
			LveGameObject::Map gameObjects;

	};
}
