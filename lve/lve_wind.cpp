#include "lve_wind.hpp"

#include <strings.h>
#include <vulkan/vulkan_core.h>

#include <cstddef>
#include <cstdint>
#include <glm/common.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <memory>
#include <vector>

#include "lve_buffer.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <cassert>
#include <cstring>

namespace lve {

LveWind::LveWind(LveDevice &device, const LveWind::Builder &builder)
    : lveDevice{device} {
   createVertexBuffers(builder.vertices);
   createIndexBuffers(builder.indices);
}

LveWind::~LveWind() {
}

std::unique_ptr<LveWind> LveWind::createModelFromMesh(
    LveDevice &device,
    const std::vector<std::vector<glm::float32>> &alttitudeMap,
    const std::vector<std::vector<glm::vec2>> &wind_speed) {
   Builder builder{};
   builder.generateMesh(alttitudeMap, wind_speed);

   return std::make_unique<LveWind>(device, builder);
}

void LveWind::createVertexBuffers(const std::vector<Vertex> &vertices) {
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

void LveWind::createIndexBuffers(const std::vector<uint32_t> &indices) {
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

void LveWind::draw(VkCommandBuffer commandBuffer) {
   if (hasIndexBuffer) {
      vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
   } else {
      vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
   }
}

void LveWind::bind(VkCommandBuffer commandBuffer) {
   VkBuffer buffers[] = {vertexBuffer->getBuffer()};
   VkDeviceSize offsets[] = {0};
   vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

   if (hasIndexBuffer) {
      vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0,
                           VK_INDEX_TYPE_UINT32);
   }
}

std::vector<VkVertexInputBindingDescription>
LveWind::Vertex::getBindingDescriptions() {
   std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
   bindingDescriptions[0].binding = 0;
   bindingDescriptions[0].stride = sizeof(Vertex);
   bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
   return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription>
LveWind::Vertex::getAttributeDescriptions() {
   std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

   attributeDescriptions.push_back(
       {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
   attributeDescriptions.push_back(
       {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});

   return attributeDescriptions;
}

void LveWind::Builder::generateMesh(
    const std::vector<std::vector<glm::float32>> &alttitudeMap,
    const std::vector<std::vector<glm::vec2>> &wind_speed) {
   vertices.clear();
   indices.clear();

   const uint32_t xn = alttitudeMap[0].size();
   const uint32_t yn = alttitudeMap.size();

   std::vector<std::vector<Vertex>> lines;
   for (size_t i = 0; i < xn; ++i) {
      std::vector<Vertex> interno_principio;
      Vertex vertex_principio = {.position = glm::vec3(i, 0, 0),
                                 .color = glm::vec3(0)};
      interno_principio.push_back(vertex_principio);
      lines.push_back(interno_principio);
      std::vector<Vertex> interno_final;
      Vertex vertex_final = {.position = glm::vec3(i, 0, yn - 1),
                             .color = glm::vec3(0)};
      interno_final.push_back(vertex_final);
      lines.push_back(interno_final);
   }
   for (size_t i = 0; i < yn; ++i) {
      std::vector<Vertex> interno_principio;
      Vertex vertex_principio = {.position = glm::vec3(0, 0, i),
                                 .color = glm::vec3(0)};
      interno_principio.push_back(vertex_principio);
      lines.push_back(interno_principio);
      std::vector<Vertex> interno_final;
      Vertex vertex_final = {.position = glm::vec3(xn - 1, 0, i),
                             .color = glm::vec3(0)};
      interno_final.push_back(vertex_final);
      lines.push_back(interno_final);
   }

   const float spacing = 10.f;
   const size_t max_samples = 10000;

   for (std::vector<Vertex> &line : lines) {
      bool adentro = true;
      while (adentro && line.size() < max_samples) {
         Vertex &vertex = line.back();

         uint32_t x0 = glm::clamp(xn - (uint32_t)floorf(vertex.position.x),
                                  (uint32_t)0, xn - 1);
         uint32_t y0 = glm::clamp((uint32_t)floorf(vertex.position.z),
                                  (uint32_t)0, yn - 1);

         uint32_t x1 = glm::clamp(xn - (uint32_t)ceilf(vertex.position.x),
                                  (uint32_t)0, xn - 1);
         uint32_t y1 = glm::clamp((uint32_t)ceilf(vertex.position.z),
                                  (uint32_t)0, yn - 1);

         float x_reg = glm::fract(vertex.position.x);
         float y_reg = glm::fract(vertex.position.z);

         float z00 = alttitudeMap[y0][x0];
         float z01 = alttitudeMap[y0][x1];
         float z10 = alttitudeMap[y1][x0];

         float wind_heigth =
             -2.0 - (z00 + x_reg * (z01 - z00) + y_reg * (z10 - z00));
         vertex.position.y = wind_heigth;

         glm::vec2 v00 = wind_speed[y0][x0];
         glm::vec2 v01 = wind_speed[y0][x1];
         glm::vec2 v10 = wind_speed[y1][x0];
         glm::vec2 v11 = wind_speed[y1][x1];

         glm::vec2 p00 = glm::vec2(0.f, 0.f);
         glm::vec2 p01 = glm::vec2(1.f, 0.f);
         glm::vec2 p10 = glm::vec2(0.f, 1.f);
         glm::vec2 p11 = glm::vec2(1.f, 1.f);

         glm::vec2 pos = glm::vec2(x_reg, y_reg);

         float d00 = glm::dot(p00 - pos, p00 - pos);
         float d01 = glm::dot(p01 - pos, p01 - pos);
         float d10 = glm::dot(p10 - pos, p10 - pos);
         float d11 = glm::dot(p11 - pos, p11 - pos);

         glm::vec2 moveDir = glm::clamp(1.f - d00, 0.f, 1.f) * v00 +
                             glm::clamp(1.f - d01, 0.f, 1.f) * v01 +
                             glm::clamp(1.f - d10, 0.f, 1.f) * v10 +
                             glm::clamp(1.f - d11, 0.f, 1.f) * v11;

         float amount = glm::dot(moveDir, moveDir);
         vertex.color = glm::vec3(amount, 0.2, 0.2);

         Vertex next_vertex = vertex;
         const float moveSpeed = 0.1;
         moveDir *= moveSpeed;
         next_vertex.position.x += moveDir.x;
         next_vertex.position.z += moveDir.y;

         if (next_vertex.position.x > xn - 1 ||
             next_vertex.position.x < 0 ||
             next_vertex.position.z > yn - 1 ||
             next_vertex.position.z < 0) {
            adentro = false;
				/*
            if (next_vertex.position.x > xn - 1) {
               float x_min = xn - 1 - vertex.position.x;
               moveDir *= x_min / moveDir.x;
            }
            if (next_vertex.position.x < 0) {
               float x_min = vertex.position.x;
               moveDir *= x_min / moveDir.x;
            }
            if (next_vertex.position.z > yn - 1) {
               float y_min = yn - 1 - vertex.position.z;
               moveDir *= y_min / moveDir.y;
            }
            if (next_vertex.position.z < 0) {
               float y_min = vertex.position.z;
               moveDir *= y_min / moveDir.y;
            }
            next_vertex.position = vertex.position;
            next_vertex.position.x = moveDir.x;
            next_vertex.position.z = moveDir.y;*/
         } else {
            line.push_back(next_vertex);
         }
      }
   }
   size_t index = 0;
   for (std::vector<Vertex> line : lines) {
      for (Vertex vertex : line) {
         vertices.push_back(vertex);
         indices.push_back(index);
         ++index;
      }
      indices.push_back(0xFFFFFFFF);
   }
}

}  // namespace lve
