#include "lve_window.hpp"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

#include <stdexcept>

namespace lve {

LveWindow::LveWindow(int w, int h, std::string name)
    : width{w}, height{h}, windowName{name} {
   initWindow();
}

LveWindow::~LveWindow() {
   glfwDestroyWindow(window);
   glfwTerminate();
}

void LveWindow::initWindow() {
   glfwInit();
   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
   glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

   window = glfwCreateWindow(width, height, windowName.c_str(), nullptr,
                             nullptr);
   glfwSetWindowUserPointer(window, this);
   glfwSetFramebufferSizeCallback(window, framebufferResizedCallback);
}

void LveWindow::createWindowSurface(VkInstance instance,
                                    VkSurfaceKHR *surface) {
   if (glfwCreateWindowSurface(instance, window, nullptr, surface) !=
       VK_SUCCESS) {
      throw std::runtime_error("failed to create window surface");
   }
}

void LveWindow::framebufferResizedCallback(GLFWwindow *window, int width,
                                           int height) {
   auto LveWindow = reinterpret_cast<class LveWindow *>(
       glfwGetWindowUserPointer(window));
   LveWindow->framebufferResized = true;
   LveWindow->width = width;
   LveWindow->height = height;
}

}  // namespace lve
