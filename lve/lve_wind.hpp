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

class LveWind {
  public:
   struct Vertex {
      glm::vec3 position{};
      glm::vec3 color{};

      static std::vector<VkVertexInputBindingDescription>
      getBindingDescriptions();
      static std::vector<VkVertexInputAttributeDescription>
      getAttributeDescriptions();
   };

   struct Builder {
      std::vector<Vertex> vertices{};
      std::vector<uint32_t> indices{};

      void generateMesh(
          const std::vector<std::vector<glm::float32>> &alttitudeMap,
          const std::vector<std::vector<glm::vec2>> &wind_speed);
   };

   LveWind(LveDevice &device, const LveWind::Builder &builder);
   ~LveWind();

   LveWind(const LveWind &) = delete;
   LveWind &operator=(const LveWind &) = delete;

   static std::unique_ptr<LveWind> createModelFromMesh(
       LveDevice &device,
       const std::vector<std::vector<glm::float32>> &alttitudeMap,
       const std::vector<std::vector<glm::vec2>> &wind_speed);

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
