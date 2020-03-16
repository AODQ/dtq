#pragma once

#include "vulkan.hpp"

#include <glm/glm.hpp>

#include <limits>
#include <utility>
#include <vector>

struct GraphicsContext; // -- fwd decl

struct SwapchainImage {
  SwapchainImage() = default;

  vk::Image     image;
  vk::ImageView view;
  vk::Fence     fence;
};

class Swapchain {
private:
  vk::SurfaceKHR surface;
  vk::SwapchainKHR swapchain;
  std::vector<SwapchainImage> images;
  vk::PresentInfoKHR presentInfo;

  GraphicsContext* context;

public:
  Swapchain() = default;

  vk::Extent2D swapchainExtent;
  vk::Format colorFormat;
  vk::ColorSpaceKHR colorSpace;
  uint32_t imageCount { 0 };
  uint32_t currentImage { 0 };

  // index of the gfx & presenting dev
  uint32_t graphicsDeviceQueueIdx = std::numeric_limits<uint32_t>::max();

  Swapchain(GraphicsContext* context_) : context(context_) {}
  Swapchain(Swapchain const&) = delete;
  Swapchain(Swapchain&&) = delete;

  void SetWindowSurface(const vk::SurfaceKHR& surface);
  void Construct(const glm::uvec2& size);
  std::vector<vk::Framebuffer> CreateFramebuffers(
    vk::FramebufferCreateInfo fbCI
  );
  uint32_t AcquireNextImage(vk::Semaphore const& presentCompleteSemaphore);
  vk::Fence const& GetSubmitFence();
  vk::Result QueuePresent(vk::Semaphore const& waitSemaphore);
  void Cleanup();
};
