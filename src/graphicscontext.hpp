#pragma once

#include "glfw.hpp"
#include "vulkan.hpp"

#include <vector>

////////////////////////////////////////////////////////////////////////////////
struct GraphicsContext {
  GraphicsContext() = default;

  vk::UniqueInstance instance;
  std::vector<vk::PhysicalDevice> physicalDevices;
  vk::PhysicalDevice physicalDevice;
  vk::PhysicalDeviceProperties deviceProperties;
  vk::PhysicalDeviceFeatures deviceFeatures;
  std::vector<vk::QueueFamilyProperties> queueFamilyProperties;
  vk::PhysicalDeviceMemoryProperties deviceMemoryProperties;
  vk::UniqueDevice device;
  vk::Queue graphicsQueue;
  vk::Queue presentQueue;

  std::unique_ptr<GlfwWindow> glfwWindow;
  vk::SurfaceKHR surface;

  static GraphicsContext Construct();
};

void LogDiagnosticInfo(GraphicsContext const & self);

uint32_t FindQueue(
  GraphicsContext const & self
, vk::QueueFlags const & desiredFlags
, vk::SurfaceKHR const & presentSurface = nullptr
);
