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
  GraphicsContext* context = nullptr;

  vk::SurfaceKHR surface { nullptr };
  vk::SwapchainKHR swapchain { nullptr };
  std::vector<SwapchainImage> images {};
  vk::PresentInfoKHR presentInfo;

public:
  Swapchain(GraphicsContext & context_, vk::SurfaceKHR & surface);
  ~Swapchain() { Cleanup(); }
  Swapchain(Swapchain const &) = delete;
  Swapchain(Swapchain &&) = delete;

  vk::Extent2D swapchainExtent;
  vk::Format colorFormat;
  vk::ColorSpaceKHR colorSpace;
  uint32_t currentImage { 0 };

  // index of the gfx & presenting dev
  uint32_t graphicsDeviceQueueIdx = std::numeric_limits<uint32_t>::max();

  size_t ImageLength() { return images.size(); }

  void Construct(const glm::uvec2& size);
  std::vector<vk::Framebuffer> CreateFramebuffers(
    vk::FramebufferCreateInfo fbCI
  );
  uint32_t AcquireNextImage(vk::Semaphore const & presentCompleteSemaphore);
  vk::Fence const & GetSubmitFence();
  vk::Result QueuePresent(vk::Semaphore const & waitSemaphore);
  void Cleanup();
};
