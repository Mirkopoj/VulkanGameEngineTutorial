#include "first_app.hpp"

#include <vulkan/vulkan_core.h>

#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <memory>
#include <vector>

#include "keyboard_movement_controller.hpp"
#include "lve/lve_buffer.hpp"
#include "lve/lve_camera.hpp"
#include "lve/lve_descriptors.hpp"
#include "lve/lve_device.hpp"
#include "lve/lve_frame_info.hpp"
#include "lve/lve_game_object.hpp"
#include "lve/lve_swap_chain.hpp"
#include "systems/compute_system.hpp"
#include "systems/imgui_system.hpp"
#include "systems/point_light_system.hpp"
#include "systems/simple_render_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <imgui.h>

#include <chrono>

namespace lve {

FirstApp::FirstApp() {
   globalPool = LveDescriptorPool::Builder(lveDevice)
                    .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
                    .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                 LveSwapChain::MAX_FRAMES_IN_FLIGHT)
                    .build();

   imguiPool =
       LveDescriptorPool::Builder(lveDevice)
           .setMaxSets(4)
           .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4)
           .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
           .build();

   computePool = LveDescriptorPool::Builder(lveDevice)
                     .setMaxSets(3)
                     .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 6)
                     .build();

   loadGameObjects();
}

FirstApp::~FirstApp() {
}

void FirstApp::run() {
   std::vector<std::unique_ptr<LveBuffer>> uboBuffers(
       LveSwapChain::MAX_FRAMES_IN_FLIGHT);
   for (int i = 0; i < uboBuffers.size(); i++) {
      uboBuffers[i] =
          std::make_unique<LveBuffer>(lveDevice, sizeof(GlobalUbo), 1,
                                      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
      uboBuffers[i]->map();
   }

   std::unique_ptr<LveDescriptorSetLayout> globalSetLayout =
       LveDescriptorSetLayout::Builder(lveDevice)
           .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                       VK_SHADER_STAGE_ALL_GRAPHICS)
           .build();

   std::vector<VkDescriptorSet> globalDescriptorSets(
       LveSwapChain::MAX_FRAMES_IN_FLIGHT);
   for (int i = 0; i < globalDescriptorSets.size(); i++) {
      auto bufferInfo = uboBuffers[i]->descriptorInfo();
      LveDescriptorWriter(*globalSetLayout, *globalPool)
          .writeBuffer(0, &bufferInfo)
          .build(globalDescriptorSets[i]);
   }

   SimpleRenderSystem simpleRenderSystem{
       lveDevice, lveRenderer.getSwapChainRenderPass(),
       globalSetLayout->getDescriptorSetLayout()};
   PointLightSystem pointLightSystem{
       lveDevice, lveRenderer.getSwapChainRenderPass(),
       globalSetLayout->getDescriptorSetLayout()};

   LveCamera camera{};

   auto viewerObject = LveGameObject::createGameObject();
   viewerObject.transform.translation.z = -2.5f;
   KeyboardMovementController cameraController{};

   ImGuiUi myimgui(lveWindow.getGLFWwindow(), lveDevice, lveRenderer,
                   imguiPool->descriptor_pool());

   auto currentTime = std::chrono::high_resolution_clock::now();

   MyTextureData initial_img;
   bool ret = LoadTextureFromFile("Fondo.jpg", &initial_img, lveDevice);
   IM_ASSERT(ret);
   MyTextureData buffer_img;
   ret = LoadTextureFromFile("Fondo.jpg", &buffer_img, lveDevice);
   IM_ASSERT(ret);
   MyTextureData filtered_img;
   ret = LoadTextureFromFile("Fondo.jpg", &filtered_img, lveDevice);
   IM_ASSERT(ret);

   std::unique_ptr<LveDescriptorSetLayout>
       computeFilterDescriptorSetLayout =
           LveDescriptorSetLayout::Builder(lveDevice)
               .addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                           VK_SHADER_STAGE_COMPUTE_BIT)
               .addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                           VK_SHADER_STAGE_COMPUTE_BIT)
               .build();

   ComputeSystem edge_detect{
       lveDevice,
       {computeFilterDescriptorSetLayout->getDescriptorSetLayout()},
       "shaders/edges.comp.spv"};
   ComputeSystem blur_filter{
       lveDevice,
       {computeFilterDescriptorSetLayout->getDescriptorSetLayout()},
       "shaders/blur.comp.spv"};
   ComputeSystem no_filter{
       lveDevice,
       {computeFilterDescriptorSetLayout->getDescriptorSetLayout()},
       "shaders/no_filter.comp.spv"};

   VkDescriptorSet DescriptorSetInOut = {};
   VkDescriptorSet DescriptorSetInBuf = {};
   VkDescriptorSet DescriptorSetBufOut = {};

   VkDescriptorImageInfo initImageInfo = {
       .imageView = initial_img.ImageView,
       .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
   };
   VkDescriptorImageInfo buffImageInfo = {
       .imageView = buffer_img.ImageView,
       .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
   };
   VkDescriptorImageInfo filteredImageInfo = {
       .imageView = filtered_img.ImageView,
       .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
   };

   LveDescriptorWriter(*computeFilterDescriptorSetLayout, *computePool)
       .writeImage(0, &initImageInfo)
       .writeImage(1, &filteredImageInfo)
       .build(DescriptorSetInOut);

   LveDescriptorWriter(*computeFilterDescriptorSetLayout, *computePool)
       .writeImage(0, &initImageInfo)
       .writeImage(1, &buffImageInfo)
       .build(DescriptorSetInBuf);

   LveDescriptorWriter(*computeFilterDescriptorSetLayout, *computePool)
       .writeImage(0, &buffImageInfo)
       .writeImage(1, &filteredImageInfo)
       .build(DescriptorSetBufOut);

   while (!lveWindow.shouldClose()) {
      glfwPollEvents();

      auto newTime = std::chrono::high_resolution_clock::now();
      float frameTime =
          std::chrono::duration<float, std::chrono::seconds::period>(
              newTime - currentTime)
              .count();
      currentTime = newTime;

      cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime,
                                     viewerObject);
      camera.setViewYXZ(viewerObject.transform.translation,
                        viewerObject.transform.rotation);

      float aspect = lveRenderer.getAspectRatio();
      camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f,
                                      100.f);

      if (auto commandBuffer = lveRenderer.beginFrame()) {
         int frameIndex = lveRenderer.getFrameIndex();
         FrameInfo frameInfo{frameIndex,
                             frameTime,
                             commandBuffer,
                             camera,
                             globalDescriptorSets[frameIndex],
                             gameObjects};
         myimgui.new_frame();

         // update
         GlobalUbo ubo{};
         ubo.projection = camera.getProjection();
         ubo.view = camera.getView();
         ubo.inverseView = camera.getInverseView();
         pointLightSystem.update(frameInfo, ubo);
         uboBuffers[frameIndex]->writeToBuffer(&ubo);
         uboBuffers[frameIndex]->flush();
         myimgui.update(&initial_img, &filtered_img);

         // render system
         lveRenderer.beginSwapChainRenderPass(commandBuffer);

         simpleRenderSystem.renderGameObjects(frameInfo);
         pointLightSystem.render(frameInfo);
         myimgui.render(commandBuffer);

         lveRenderer.endSwapChainRenderPass(commandBuffer);
         lveRenderer.endFrame();
      }

      int shader_count = myimgui.get_shader_count();
      if (shader_count == 0) {
         no_filter.instant_dispatch(initial_img.Width, initial_img.Height,
                            initial_img.Channels, DescriptorSetInOut);
      }
      if (shader_count > 0) {
         ComputeSystem &first =
             myimgui.get_first_shader() ? blur_filter : edge_detect;
         VkDescriptorSet &DescriptorSet =
             shader_count == 1 ? DescriptorSetInOut : DescriptorSetInBuf;
         first.instant_dispatch(initial_img.Width, initial_img.Height,
                        initial_img.Channels, DescriptorSet);
      }
      if (shader_count > 1) {
         ComputeSystem &second =
             myimgui.get_second_shader() ? blur_filter : edge_detect;
         second.instant_dispatch(initial_img.Width, initial_img.Height,
                         initial_img.Channels, DescriptorSetBufOut);
      }
   }

   vkDeviceWaitIdle(lveDevice.device());
   RemoveTexture(&initial_img, lveDevice);
   RemoveTexture(&buffer_img, lveDevice);
   RemoveTexture(&filtered_img, lveDevice);
}

void FirstApp::loadGameObjects() {
   std::shared_ptr<LveModel> lveModel =
       LveModel::createModelFromFile(lveDevice, "models/flat_vase.obj");

   auto flatVase = LveGameObject::createGameObject();
   flatVase.model = lveModel;
   flatVase.transform.translation = {-.5f, .5f, 0.0f};
   flatVase.transform.scale = glm::vec3(3.f);
   gameObjects.emplace(flatVase.getId(), std::move(flatVase));

   lveModel =
       LveModel::createModelFromFile(lveDevice, "models/smooth_vase.obj");

   auto smoothVase = LveGameObject::createGameObject();
   smoothVase.model = lveModel;
   smoothVase.transform.translation = {.5f, .5f, 0.0f};
   smoothVase.transform.scale = {3.f, 1.5f, 3.f};
   gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

   lveModel = LveModel::createModelFromFile(lveDevice, "models/quad.obj");

   auto floor = LveGameObject::createGameObject();
   floor.model = lveModel;
   floor.transform.translation = {.0f, .5f, .0f};
   floor.transform.scale = {3.f, 1.f, 3.f};
   gameObjects.emplace(floor.getId(), std::move(floor));

   lveModel =
       LveModel::createModelFromFile(lveDevice, "models/colored_cube.obj");

   auto cubo = LveGameObject::createGameObject();
   cubo.model = lveModel;
   cubo.transform.translation = {.0f, 0.f, 1.f};
   cubo.transform.scale = {0.2f, 0.2f, 0.2f};
   gameObjects.emplace(cubo.getId(), std::move(cubo));

   std::vector<glm::vec3> lightColors{{1.f, .1f, .1f}, {.1f, .1f, 1.f},
                                      {.1f, 1.f, .1f}, {1.f, 1.f, .1f},
                                      {.1f, 1.f, 1.f}, {1.f, 1.f, 1.f}};

   for (int i = 0; i < lightColors.size(); i++) {
      auto pointLight = LveGameObject::makePointLight(0.2f);
      pointLight.color = lightColors[i];
      auto rotateLight = glm::rotate(
          glm::mat4(1.f), (i * glm::two_pi<float>()) / lightColors.size(),
          {0.f, -1.f, 0.f});
      pointLight.transform.translation =
          glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));

      gameObjects.emplace(pointLight.getId(), std::move(pointLight));
   }
}

}  // namespace lve
