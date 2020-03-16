#pragma once

#include "vulkan.hpp"

#include <glm/glm.hpp>

#include <string>
#include <vector>

struct GLFWwindow; // -- fwd decl

struct GlfwWindow {
  GlfwWindow();
  ~GlfwWindow();

  GlfwWindow(GlfwWindow &&) = delete;
  GlfwWindow(GlfwWindow const &) = delete;
  GlfwWindow& operator=(GlfwWindow const &) = delete;

  GLFWwindow* window = nullptr;

  void Construct(glm::uvec2 size);
};

vk::SurfaceKHR ConstructWindowSurface(
  GlfwWindow& self,
  vk::Instance const & instance
);

std::vector<std::string> RequiredInstanceExtensions(const GlfwWindow& self);
