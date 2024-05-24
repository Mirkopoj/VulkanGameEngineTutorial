#pragma once

#include <glm/fwd.hpp>
#include <memory>
#include <vector>

#include "lve_buffer.hpp"
#include "lve_device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace lve {

class LveTerrain {
  public:
   struct Vertex {
      glm::float32 alttitude{};
      glm::vec3 color{};
      glm::vec3 normal{};

      static std::vector<VkVertexInputBindingDescription>
      getBindingDescriptions();
      static std::vector<VkVertexInputAttributeDescription>
      getAttributeDescriptions();
   };

   struct Builder {
      std::vector<Vertex> vertices{};
      std::vector<uint32_t> indices{};

      void generateMesh(
          const std::vector<std::vector<glm::float32>> alttitudeMap);
   };

   LveTerrain(LveDevice &device, const LveTerrain::Builder &builder);
   ~LveTerrain();

   LveTerrain(const LveTerrain &) = delete;
   LveTerrain &operator=(const LveTerrain &) = delete;

   static std::unique_ptr<LveTerrain> createModelFromMesh(
       LveDevice &device,
       const std::vector<std::vector<glm::float32>> alttitudeMap);

   void bind(VkCommandBuffer commandBuffer);
   void draw(VkCommandBuffer commandBuffer);

  private:
   void createVertexBuffers(const std::vector<Vertex> &vertices);
   void createIndexBuffers(const std::vector<uint32_t> &indices);

   LveDevice &lveDevice;

   std::unique_ptr<LveBuffer> vertexBuffer;
   uint32_t vertexCount;

   bool hasIndexBuffer = false;
   std::unique_ptr<LveBuffer> indexBuffer;
   uint32_t indexCount;
};

}  // namespace lve

