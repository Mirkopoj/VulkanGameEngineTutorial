#pragma once

#include <GLFW/glfw3.h>

#include "lve/lve_game_object.hpp"

namespace lve {

class KeyboardMovementController {
  public:
   struct KeyMappings {
      int moveLeft = GLFW_KEY_A;
      int moveRight = GLFW_KEY_D;
      int moveForward = GLFW_KEY_W;
      int moveBackward = GLFW_KEY_S;
      int moveUp = GLFW_KEY_SPACE;
      int moveDown = GLFW_KEY_LEFT_SHIFT;
      int moveLeft2 = GLFW_KEY_LEFT;
      int moveRight2 = GLFW_KEY_RIGHT;
      int moveForward2 = GLFW_KEY_UP;
      int moveBackward2 = GLFW_KEY_DOWN;
   };

   void moveInPlaneXZ(GLFWwindow* window, float dt,
                      LveGameObject& gameObject);

   KeyMappings keys{};
   float moveSpeed{3.f};
   float lookSpeed{2.5f};
   bool normalMouse{true};
   bool changedMouse{false};
	double lastX;
	double lastY;
};

}  // namespace lve
