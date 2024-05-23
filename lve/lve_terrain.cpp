#include "lve_terrain.hpp"

#include <strings.h>
#include <vulkan/vulkan_core.h>

#include <cstddef>
#include <cstdint>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <memory>
#include <vector>

#include "lve_buffer.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <cassert>
#include <cstring>

namespace std {}  // namespace std

namespace lve {

LveTerrain::LveTerrain(LveDevice &device,
                       const LveTerrain::Builder &builder)
    : lveDevice{device} {
   createVertexBuffers(builder.vertices);
   createIndexBuffers(builder.indices);
}

LveTerrain::~LveTerrain() {
}

std::unique_ptr<LveTerrain> LveTerrain::createModelFromMesh(
    LveDevice &device,
    const std::vector<std::vector<glm::float32>> alttitudeMap) {
   Builder builder{};
   builder.generateMesh(alttitudeMap);

   return std::make_unique<LveTerrain>(device, builder);
}

void LveTerrain::createVertexBuffers(const std::vector<Vertex> &vertices) {
   vertexCount = static_cast<uint32_t>(vertices.size());
   assert(vertexCount >= 3 && "Vertex count must be at least 3");
   VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
   uint32_t vertexSize = sizeof(vertices[0]);

   LveBuffer stagingBuffer{
       lveDevice,
       vertexSize,
       vertexCount,
       VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
   };

   stagingBuffer.map();
   stagingBuffer.writeToBuffer((void *)vertices.data());

   vertexBuffer =
       std::make_unique<LveBuffer>(lveDevice, vertexSize, vertexCount,
                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                       VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

   lveDevice.copyBuffer(stagingBuffer.getBuffer(),
                        vertexBuffer->getBuffer(), bufferSize);
}

void LveTerrain::createIndexBuffers(const std::vector<uint32_t> &indices) {
   indexCount = static_cast<uint32_t>(indices.size());
   hasIndexBuffer = indexCount > 0;

   if (!hasIndexBuffer) return;

   VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
   uint32_t indexSize = sizeof(indices[0]);

   LveBuffer stagingBuffer{
       lveDevice,
       indexSize,
       indexCount,
       VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
   };

   stagingBuffer.map();
   stagingBuffer.writeToBuffer((void *)indices.data());

   indexBuffer = std::make_unique<LveBuffer>(
       lveDevice, indexSize, indexCount,
       VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

   lveDevice.copyBuffer(stagingBuffer.getBuffer(),
                        indexBuffer->getBuffer(), bufferSize);
}

void LveTerrain::draw(VkCommandBuffer commandBuffer) {
   if (hasIndexBuffer) {
      vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
   } else {
      vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
   }
}

void LveTerrain::bind(VkCommandBuffer commandBuffer) {
   VkBuffer buffers[] = {vertexBuffer->getBuffer()};
   VkDeviceSize offsets[] = {0};
   vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

   if (hasIndexBuffer) {
      vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0,
                           VK_INDEX_TYPE_UINT32);
   }
}

std::vector<VkVertexInputBindingDescription>
LveTerrain::Vertex::getBindingDescriptions() {
   std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
   bindingDescriptions[0].binding = 0;
   bindingDescriptions[0].stride = sizeof(Vertex);
   bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
   return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription>
LveTerrain::Vertex::getAttributeDescriptions() {
   std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

   attributeDescriptions.push_back(
       {0, 0, VK_FORMAT_R32_SFLOAT, offsetof(Vertex, alttitude)});
   attributeDescriptions.push_back(
       {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
   attributeDescriptions.push_back(
       {2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});

   return attributeDescriptions;
}

// Temporal
glm::vec3 color(float x, float y, uint32_t xn, uint32_t yn) {
   float index_n = x + y;
   index_n /= xn + yn - 2;
   glm::vec3 color;
   float a = (1 - index_n) / 0.2;
   int X = a;
   float Y = 255.f * (a - (float)X);
   switch (X) {
      case 0:
         color = {255.0, Y, 0.0};
         break;
      case 1:
         color = {255.0 - Y, 255.0, 0.0};
         break;
      case 2:
         color = {0.0, 255.0, Y};
         break;
      case 3:
         color = {0.0, 255.0 - Y, 255.0};
         break;
      case 4:
         color = {Y, 0.0, 255.0};
         break;
      case 5:
         color = {255.0, 0.0, 255.0};
         break;
   }
   color /= 255.0;
   return color;
}

void LveTerrain::Builder::generateMesh(
    const std::vector<std::vector<glm::float32>> alttitudeMap) {
   vertices.clear();
   indices.clear();

   uint32_t xn = alttitudeMap.size();
   uint32_t yn = alttitudeMap[0].size();
   uint32_t total_verts = yn + (xn - 1) * (2 * yn - 2);
   uint32_t n = 4 * xn - 2;

   for (int x = xn-1; x >= 0; --x) {
      for (int y = 0; y < yn; ++y) {
         uint32_t xs = x == xn - 1 ? x : x + 1;
         uint32_t xa = x == 0 ? x : x - 1;
         uint32_t ys = y == yn - 1 ? y : y + 1;
         uint32_t ya = y == 0 ? y : y - 1;

         glm::vec3 x_y = {x, y, alttitudeMap[x][y]};
         glm::vec3 xs_y = {xs, y, alttitudeMap[xs][y]};
         glm::vec3 xa_y = {xa, y, alttitudeMap[xa][y]};
         glm::vec3 x_ys = {x, ys, alttitudeMap[x][ys]};
         glm::vec3 x_ya = {x, ya, alttitudeMap[x][ya]};

         glm::vec3 x_a = x_y - x_ys;
         glm::vec3 x_b = x_y - xs_y;
         glm::vec3 x_c = x_y - x_ya;
         glm::vec3 x_d = x_y - xa_y;

         glm::vec3 n1 = glm::cross(x_a, x_b);
         glm::vec3 n2 = glm::cross(x_b, x_c);
         glm::vec3 n3 = glm::cross(x_c, x_d);
         glm::vec3 n4 = glm::cross(x_d, x_a);

         glm::vec3 normal = (n1 + n2 + n3 + n4) / 4.f;

         Vertex vertex = {.alttitude = -alttitudeMap[x][y],
                          .color = color(x, y, xn, yn),
                          .normal = normal};

         vertices.push_back(vertex);
      }
   }

   for (uint32_t i = 0; i < total_verts; ++i) {
      uint32_t r = i % n;
      uint32_t c = r / 2;
      uint32_t d = (c / xn) % 2;
      uint32_t s = 1 - 2 * d;

      uint32_t y = s * (i % 2) + (c / xn) * 2 + (i / n) * 2;
      uint32_t x = d * (xn - 1) + s * (((r + d) / 2) % xn);

      uint32_t index = x + y * xn;
      indices.push_back(index);
   }
}
}  // namespace lve