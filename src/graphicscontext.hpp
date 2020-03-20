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

  vk::UniqueCommandPool commandPool;

  vk::Queue graphicsQueue;
  vk::Queue computeQueue;
  vk::Queue transferQueue;

  uint32_t graphicsQueueIdx = 0;
  uint32_t computeQueueIdx  = 0;
  uint32_t transferQueueIdx = 0;

  std::unique_ptr<GlfwWindow> glfwWindow;
  vk::SurfaceKHR surface;

  bool enableDebugMarkers = false;

  static GraphicsContext Construct();
};

void LogDiagnosticInfo(GraphicsContext const & self);

uint32_t FindQueue(
  GraphicsContext const & self
, vk::QueueFlags const & desiredFlags
, vk::SurfaceKHR const & presentSurface = nullptr
);

bool DeviceExtensionPresent(
  vk::PhysicalDevice const & physicalDevice
, std::string const & extension
);

void Submit(
  GraphicsContext const & context
, vk::CommandBuffer const & commandBuffer
, vk::Semaphore const & wait
, vk::PipelineStageFlags waitStage
, vk::Semaphore const & signal
, vk::Fence const & fence
);
