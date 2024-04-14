#include "first_app.hpp"

#include <vulkan/vulkan_core.h>

#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <iostream>
#include <memory>
#include <ostream>
#include <vector>

#include "keyboard_movement_controller.hpp"
#include "lve/lve_buffer.hpp"
#include "lve/lve_camera.hpp"
#include "lve/lve_descriptors.hpp"
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

   auto globalSetLayout =
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
   MyTextureData filtered_img;
   ret = LoadTextureFromFile("Fondo.jpg", &filtered_img, lveDevice);
   IM_ASSERT(ret);

   // Veeer vvvvvvvv hay un Builder
   const std::vector<VkDescriptorSetLayoutBinding> desc_lay_bind = {
       {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1,
        VK_SHADER_STAGE_COMPUTE_BIT},
       {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1,
        VK_SHADER_STAGE_COMPUTE_BIT}};

   VkDescriptorSetLayoutCreateInfo desc_lay_info = {};
   desc_lay_info.sType =
       VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
   desc_lay_info.pNext = nullptr;
   desc_lay_info.flags = 0;
   desc_lay_info.bindingCount = desc_lay_bind.size();
   desc_lay_info.pBindings = desc_lay_bind.data();

   std::vector<VkDescriptorSetLayout> desc_lay = {};
	desc_lay.resize(1);
   vkCreateDescriptorSetLayout(lveDevice.device(), &desc_lay_info, nullptr,
                               &desc_lay[0]);
   // Hasta aca^^^^^^^^^

   ComputeSystem edge_detect{lveDevice, desc_lay,
                             "shaders/edges.comp.spv"};
   ComputeSystem blur_filter{lveDevice, desc_lay, "shaders/blur.comp.spv"};

   // Ver esto tmb
   VkDescriptorPoolSize desc_pool_size = {
       .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = 2};
   VkDescriptorPoolCreateInfo desc_pool_info = {
       .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
       .pNext = nullptr,
       .flags = 0,
       .maxSets = 2,
       .poolSizeCount = 1,
       .pPoolSizes = &desc_pool_size,
   };
   VkDescriptorPool desc_pool = {};
   vkCreateDescriptorPool(lveDevice.device(), &desc_pool_info, nullptr,
                          &desc_pool);

   VkDescriptorSetAllocateInfo desc_alloc_info = {
       .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
       .pNext = nullptr,
       .descriptorPool = desc_pool,
       .descriptorSetCount = 1,
       .pSetLayouts = desc_lay.data()};

   std::vector<VkDescriptorSet> DescriptorSets = {};
   DescriptorSets.resize(1);
   vkAllocateDescriptorSets(lveDevice.device(), &desc_alloc_info,
                            &DescriptorSets[0]);

   VkDescriptorSet DescriptorSet = DescriptorSets.front();

   const uint32_t comp_f_index =
       lveDevice.findPhysicalQueueFamilies().computeFamily;
   const uint32_t NumElements =
       initial_img.Width * initial_img.Height * initial_img.Channels;
   const uint32_t BufferSize = NumElements * sizeof(int32_t);

   VkBufferCreateInfo buf_info = {
       .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
       .pNext = nullptr,
       .flags = 0,
       .size = BufferSize + 1,
       .usage = VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR,
       .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
       .queueFamilyIndexCount = 1,
       .pQueueFamilyIndices = &comp_f_index,
   };

   VkBuffer InBuffer = {};
   vkCreateBuffer(lveDevice.device(), &buf_info, nullptr, &InBuffer);
   VkBuffer OutBuffer = {};
   vkCreateBuffer(lveDevice.device(), &buf_info, nullptr, &OutBuffer);

   VkMemoryRequirements inReq = {};
   vkGetBufferMemoryRequirements(lveDevice.device(), InBuffer, &inReq);
   VkMemoryRequirements outReq = {};
   vkGetBufferMemoryRequirements(lveDevice.device(), OutBuffer, &outReq);

   VkPhysicalDeviceMemoryProperties phys_mem_props = {};
   vkGetPhysicalDeviceMemoryProperties(lveDevice.physical_device(),
                                       &phys_mem_props);

   uint32_t MemoryTypeIndex = uint32_t(~0);
   VkDeviceSize MemoryHeapSize = uint32_t(~0);
   for (uint32_t CurrentMemoryTypeIndex = 0;
        CurrentMemoryTypeIndex < phys_mem_props.memoryTypeCount;
        ++CurrentMemoryTypeIndex) {
      VkMemoryType MemoryType =
          phys_mem_props.memoryTypes[CurrentMemoryTypeIndex];
      if ((VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT &
           MemoryType.propertyFlags) &&
          (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT &
           MemoryType.propertyFlags)) {
         MemoryHeapSize =
             phys_mem_props.memoryHeaps[MemoryType.heapIndex].size;
         MemoryTypeIndex = CurrentMemoryTypeIndex;
         break;
      }
   }

   std::cout << "Memory Type Index: " << MemoryTypeIndex << std::endl;
   std::cout << "Memory Heap Size : "
             << MemoryHeapSize / 1024 / 1024 / 1024 << " GB" << std::endl;

   // Allocate memory
   VkMemoryAllocateInfo InBufferMemoryAllocateInfo = {
       .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
       .pNext = nullptr,
       .allocationSize = inReq.size,
       .memoryTypeIndex = MemoryTypeIndex};
   VkMemoryAllocateInfo OutBufferMemoryAllocateInfo = {
       .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
       .pNext = nullptr,
       .allocationSize = outReq.size,
       .memoryTypeIndex = MemoryTypeIndex};

   VkDeviceMemory InBufferMemory = {};
   vkAllocateMemory(lveDevice.device(), &InBufferMemoryAllocateInfo,
                    nullptr, &InBufferMemory);

   VkDeviceMemory OutBufferMemory = {};
   vkAllocateMemory(lveDevice.device(), &OutBufferMemoryAllocateInfo,
                    nullptr, &OutBufferMemory);

   // Bind buffers to memory
   vkBindBufferMemory(lveDevice.device(), InBuffer, InBufferMemory, 0);
   vkBindBufferMemory(lveDevice.device(), OutBuffer, OutBufferMemory, 0);

   VkDescriptorBufferInfo InBufferInfo = {
       .buffer = InBuffer,
       .offset = 0,
       .range = NumElements * sizeof(int32_t),
   };
   VkDescriptorBufferInfo OutBufferInfo = {
       .buffer = OutBuffer,
       .offset = 0,
       .range = NumElements * sizeof(int32_t),
   };

   const std::vector<VkWriteDescriptorSet> WriteDescriptorSets = {
       {
           .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
           .pNext = nullptr,
           .dstSet = DescriptorSet,
           .dstBinding = 0,
           .dstArrayElement = 0,
           .descriptorCount = 1,
           .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
           .pImageInfo = nullptr,
           .pBufferInfo = &InBufferInfo,
           .pTexelBufferView = nullptr,
       },
       {
           .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
           .pNext = nullptr,
           .dstSet = DescriptorSet,
           .dstBinding = 1,
           .dstArrayElement = 0,
           .descriptorCount = 1,
           .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
           .pImageInfo = nullptr,
           .pBufferInfo = &OutBufferInfo,
           .pTexelBufferView = nullptr,
       },
   };
   vkUpdateDescriptorSets(lveDevice.device(), 2,
                          WriteDescriptorSets.data(), 0, nullptr);

   VkCommandBuffer CmdBuffer = lveDevice.beginSingleTimeCommands();
   vkCmdBindPipeline(CmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                     edge_detect.computePipeline);
   vkCmdBindDescriptorSets(CmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                           edge_detect.pipelineLayout, 0, 1,
                           DescriptorSets.data(), 0, nullptr);
   vkCmdDispatch(CmdBuffer, initial_img.Width, initial_img.Height,
                 initial_img.Channels);
   vkEndCommandBuffer(CmdBuffer);

   // Fence and submit
   VkQueue Queue = lveDevice.computeQueue();
   VkFenceCreateInfo FenceCreateInfo = {
       .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
       .pNext = nullptr,
       .flags = 0,
   };
   VkFence Fence = {};
   vkCreateFence(lveDevice.device(), &FenceCreateInfo, nullptr, &Fence);
   VkSubmitInfo SubmitInfo = {
       .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
       .pNext = nullptr,
       .waitSemaphoreCount = 0,
       .pWaitSemaphores = nullptr,
       .pWaitDstStageMask = nullptr,
       .commandBufferCount = 1,
       .pCommandBuffers = &CmdBuffer,
       .signalSemaphoreCount = 0,
       .pSignalSemaphores = nullptr,
   };
   vkQueueSubmit(Queue, 1, &SubmitInfo, Fence);
   vkWaitForFences(lveDevice.device(), 1, &Fence, true, uint64_t(-1));

   CmdBuffer = lveDevice.beginSingleTimeCommands();
   vkCmdBindPipeline(CmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                     blur_filter.computePipeline);
   vkCmdBindDescriptorSets(CmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                           blur_filter.pipelineLayout, 0, 1,
                           DescriptorSets.data(), 0, nullptr);
   vkCmdDispatch(CmdBuffer, initial_img.Width, initial_img.Height,
                 initial_img.Channels);
   vkEndCommandBuffer(CmdBuffer);

   // Fence and submit
	vkResetFences(lveDevice.device(), 1, &Fence);
   vkQueueSubmit(Queue, 1, &SubmitInfo, Fence);
   vkWaitForFences(lveDevice.device(), 1, &Fence, true, uint64_t(-1));

   // Hasta aca^^^^^^^^^ Aca deberian estar ambos filtros aplicados

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
   }

   vkDeviceWaitIdle(lveDevice.device());
   RemoveTexture(&initial_img, lveDevice);
   RemoveTexture(&filtered_img, lveDevice);
   vkDestroyFence(lveDevice.device(), Fence, nullptr);
   vkDestroyDescriptorSetLayout(lveDevice.device(), desc_lay[0], nullptr);
   vkDestroyDescriptorPool(lveDevice.device(), desc_pool, nullptr);
   vkFreeMemory(lveDevice.device(), InBufferMemory, nullptr);
   vkFreeMemory(lveDevice.device(), OutBufferMemory, nullptr);
   vkDestroyBuffer(lveDevice.device(), InBuffer, nullptr);
   vkDestroyBuffer(lveDevice.device(), OutBuffer, nullptr);
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
