#include "lve_terrain.hpp"

#include <vulkan/vulkan_core.h>

#include <cstddef>
#include <cstdint>
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
    const std::vector<std::vector<float>> alttitudeMap)
{
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
       {0, 0, VK_FORMAT_R16_SFLOAT, offsetof(Vertex, alttitude)});
   attributeDescriptions.push_back(
       {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
   attributeDescriptions.push_back(
       {2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});

   return attributeDescriptions;
}

void LveTerrain::Builder::generateMesh(
    const std::vector<std::vector<float>> alttitudeMap) {
   vertices.clear();
   indices.clear();

   uint32_t xn = alttitudeMap.size();
   uint32_t yn = alttitudeMap[0].size();
   uint32_t total_verts = yn + (xn - 1) * (2 * yn - 2);

   for (uint32_t i = 0; i < total_verts; ++i) {
      uint32_t r = i / 2;
      uint32_t s = (r / xn) % 2;
      uint32_t m = 1 - 2 * s;

      uint32_t x = s * (xn - 1) + m * (((i + s) / 2) % xn);
      uint32_t y = m * (i % 2) + (r / xn) * 2;

      uint32_t index = x + y * xn;

      Vertex vertex = {.alttitude = alttitudeMap[x][y],
                       .color = {1.f, 1.f, 1.f},
                       .normal = {1.f, 1.f, 1.f}};

      vertices.push_back(vertex);
      indices.push_back(index);
   }
}
}  // namespace lve
