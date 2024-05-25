#include "terrain_movement_controller.hpp"

#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdint>
#include <glm/common.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/constants.hpp>
#include <limits>

namespace lve {

void TerrainMovementController::moveInPlaneXZ(
    GLFWwindow* window, float dt, LveGameObject& gameObject,
    std::vector<std::vector<glm::float32>>& altitudeMap,
    float cameraHeight, bool caminata) {
   int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
   if (state == GLFW_PRESS && !changedMouse) {
      int cursor_mode =
          normalMouse ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL;
      int raw_mouse_motion = normalMouse ? GLFW_FALSE : GLFW_TRUE;
      normalMouse ^= true;
      glfwSetInputMode(window, GLFW_CURSOR, cursor_mode);
      if (glfwRawMouseMotionSupported()) {
         glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, raw_mouse_motion);
      }
   }
   changedMouse = state == GLFW_PRESS;

   if (!normalMouse) {
      double xpos, ypos;
      glfwGetCursorPos(window, &ypos, &xpos);

      double dx = xpos - lastX;
      double dy = ypos - lastY;
      lastX = xpos;
      lastY = ypos;

      glm::vec3 rotate{0};
      rotate.y += dy;
      rotate.x -= dx;

      if (glm::dot(rotate, rotate) >
          std::numeric_limits<float>::epsilon()) {
         gameObject.transform.rotation +=
             lookSpeed * dt * glm::normalize(rotate);
      }

      gameObject.transform.rotation.x =
          glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
      gameObject.transform.rotation.y =
          glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

      float yaw = gameObject.transform.rotation.y;
      const glm::vec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
      const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
      const glm::vec3 upDir{0.f, -1.f, 0.f};

      glm::vec3 moveDir{0.f};
      if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS ||
          glfwGetKey(window, keys.moveForward2) == GLFW_PRESS) {
         moveDir += forwardDir;
      }
      if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS ||
          glfwGetKey(window, keys.moveBackward2) == GLFW_PRESS) {
         moveDir -= forwardDir;
      }
      if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS ||
          glfwGetKey(window, keys.moveRight2) == GLFW_PRESS) {
         moveDir += rightDir;
      }
      if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS ||
          glfwGetKey(window, keys.moveLeft2) == GLFW_PRESS) {
         moveDir -= rightDir;
      }
      if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) {
         moveDir += upDir;
      }
      if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) {
         moveDir -= upDir;
      }

      uint32_t xn = altitudeMap[0].size();
      uint32_t yn = altitudeMap.size();

      uint32_t x = glm::clamp(
          xn - (uint32_t)roundf(gameObject.transform.translation.x),
          (uint32_t)0, xn - 1);
      uint32_t y =
          glm::clamp((uint32_t)roundf(gameObject.transform.translation.z),
                     (uint32_t)0, yn - 1);

      float roof = fmin(xn, yn);
		float floor = altitudeMap[y][x];

      float moveSpeed =
          moveSpeedMin +
          moveSpeedMax * ((-gameObject.transform.translation.y - floor) /
                          (roof - floor));

      if (glm::dot(moveDir, moveDir) >
          std::numeric_limits<float>::epsilon()) {
         gameObject.transform.translation +=
             moveSpeed * dt * glm::normalize(moveDir);
      }

      uint32_t x0 = glm::clamp(
          xn - (uint32_t)floorf(gameObject.transform.translation.x),
          (uint32_t)0, xn - 1);
      uint32_t y0 =
          glm::clamp((uint32_t)floorf(gameObject.transform.translation.z),
                     (uint32_t)0, yn - 1);

      uint32_t x1 = glm::clamp(
          xn - (uint32_t)ceilf(gameObject.transform.translation.x),
          (uint32_t)0, xn - 1);
      uint32_t y1 =
          glm::clamp((uint32_t)ceilf(gameObject.transform.translation.z),
                     (uint32_t)0, yn - 1);

      float x_reg = glm::fract(gameObject.transform.translation.x);
      float y_reg = glm::fract(gameObject.transform.translation.z);

      float z00 = altitudeMap[y0][x0];
      float z01 = altitudeMap[y0][x1];
      float z10 = altitudeMap[y1][x0];

      float cam_floor = -cameraHeight -
                        (z00 + x_reg * (z01 - z00) + y_reg * (z10 - z00));
      float cam_roof = caminata ? cam_floor - 0.1 : -roof;
      gameObject.transform.translation = glm::clamp(
          gameObject.transform.translation, glm::vec3{0.f, cam_roof, 0.f},
          glm::vec3{xn, cam_floor, yn});
   }
}

}  // namespace lve
