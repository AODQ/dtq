#include "glfw.hpp"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

////////////////////////////////////////////////////////////////////////////////
GlfwWindow::GlfwWindow() {
  glfwInit();
}

////////////////////////////////////////////////////////////////////////////////
GlfwWindow::~GlfwWindow() {
  if (this->window)
    { glfwDestroyWindow(this->window); }
}

////////////////////////////////////////////////////////////////////////////////
void GlfwWindow::Construct(glm::uvec2 size) {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  this->window =
    glfwCreateWindow(size.x, size.y, "Demo Toad Quill", nullptr, nullptr);
}

////////////////////////////////////////////////////////////////////////////////
vk::SurfaceKHR ConstructWindowSurface(
  GlfwWindow& self,
  vk::Instance const & instance
) {
  VkSurfaceKHR rawSurface;
  auto result =
    static_cast<vk::Result>(
      glfwCreateWindowSurface(
        static_cast<VkInstance>(instance)
      , self.window
      , nullptr
      , &rawSurface
      )
    );

  if (result != vk::Result::eSuccess)
    spdlog::error("Could not create window surface");

  auto cvrResult =
    vk::createResultValue(result, rawSurface, "vk::CommandBuffer::begin");
  if (cvrResult.result != vk::Result::eSuccess)
    spdlog::error("Could not create window surface");
  return cvrResult.value;
}

////////////////////////////////////////////////////////////////////////////////
std::vector<std::string> RequiredInstanceExtensions(const GlfwWindow& self) {
  std::vector<std::string> result;
  uint32_t count = 0;
  char const ** names = glfwGetRequiredInstanceExtensions(&count);
  for (uint32_t i = 0; i < count && names && count; ++ i)
    result.emplace_back(names[i]);
  return result;
}

////////////////////////////////////////////////////////////////////////////////
bool ShouldWindowClose(GlfwWindow & window) {
  return glfwWindowShouldClose(window.window);
}

////////////////////////////////////////////////////////////////////////////////
void PollEvents(GlfwWindow & window) {
  glfwPollEvents();
}
