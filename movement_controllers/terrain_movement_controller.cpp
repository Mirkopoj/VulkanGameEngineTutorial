#include "terrain_movement_controller.hpp"

#include <GLFW/glfw3.h>

#include <glm/common.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/constants.hpp>
#include <limits>

namespace lve {

void TerrainMovementController::moveInPlaneXZ(GLFWwindow* window, float dt,
                                              LveGameObject& gameObject,
                                              float floor, float roof) {
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

      float moveSpeed =
          moveSpeedMin +
          moveSpeedMax * ((-gameObject.transform.translation.y - floor) /
                          (roof - floor));

      if (glm::dot(moveDir, moveDir) >
          std::numeric_limits<float>::epsilon()) {
         gameObject.transform.translation +=
             moveSpeed * dt * glm::normalize(moveDir);
      }
   }
}

}  // namespace lve
