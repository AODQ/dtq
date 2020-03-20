#include "swapchain.hpp"

#include "util.hpp"

#include "graphicscontext.hpp"

////////////////////////////////////////////////////////////////////////////////
Swapchain::Swapchain(
  GraphicsContext & context_,
  vk::SurfaceKHR & surface_
)
: context{&context_}, surface{surface_}
{
  auto surfaceFormats =
    this->context->physicalDevice.getSurfaceFormatsKHR(this->surface).value;

  // if usrface format list only includes one with eUndefined, there is no
  // preferred format, so assumed RGBA8
  if ((surfaceFormats.size() == 1)
   && (surfaceFormats[0].format == vk::Format::eUndefined)
  ) {
    this->colorFormat = vk::Format::eB8G8R8A8Unorm;
  } else {
    this->colorFormat = surfaceFormats[0].format;
  }
  this->colorSpace = surfaceFormats[0].colorSpace;

  this->graphicsDeviceQueueIdx =
    FindQueue(*this->context, vk::QueueFlagBits::eGraphics, this->surface);
}

////////////////////////////////////////////////////////////////////////////////
void Swapchain::Construct(const glm::uvec2& size)
{
  vk::SwapchainKHR oldSwapchain = swapchain;

  vk::SurfaceCapabilitiesKHR surfaceCapabilities =
    this->context->physicalDevice
      .getSurfaceCapabilitiesKHR(this->surface).value;

  std::vector<vk::PresentModeKHR> presentModes =
    this->context->physicalDevice
      .getSurfacePresentModesKHR(this->surface).value;

  if (surfaceCapabilities.currentExtent.width == static_cast<uint32_t>(-1)) {
    swapchainExtent.width = size.x;
    swapchainExtent.height = size.y;
  } else {
    swapchainExtent = surfaceCapabilities.currentExtent;
  }

  // prefer mailbox if available
  vk::PresentModeKHR swapchainPresentMode = vk::PresentModeKHR::eFifo;
  for (auto const& mode : presentModes) {
    if (mode == vk::PresentModeKHR::eMailbox) {
      swapchainPresentMode = vk::PresentModeKHR::eMailbox;
      break;
    }
    if ((swapchainPresentMode != vk::PresentModeKHR::eMailbox)
     && (mode == vk::PresentModeKHR::eImmediate)
    ) {
      swapchainPresentMode = vk::PresentModeKHR::eImmediate;
    }
  }

  // determine number of images
  uint32_t desiredImages = surfaceCapabilities.minImageCount+1;
  if ((surfaceCapabilities.maxImageCount > 0)
   && (desiredImages > surfaceCapabilities.maxImageCount)
  ) {
    desiredImages = surfaceCapabilities.maxImageCount;
  }


  { // create swapchain

    vk::SurfaceTransformFlagBitsKHR preTransform =
        (surfaceCapabilities.supportedTransforms
      & vk::SurfaceTransformFlagBitsKHR::eIdentity)
    ? vk::SurfaceTransformFlagBitsKHR::eIdentity
    : surfaceCapabilities.currentTransform
    ;

    vk::SwapchainCreateInfoKHR swapchainCI;
    swapchainCI.pNext = nullptr;
    swapchainCI.flags = {};
    swapchainCI.surface = this->surface;
    swapchainCI.minImageCount = desiredImages;
    swapchainCI.imageFormat = colorFormat;
    swapchainCI.imageColorSpace = colorSpace;
    swapchainCI.imageExtent = swapchainExtent;
    swapchainCI.imageArrayLayers = 1;
    swapchainCI.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
    swapchainCI.imageSharingMode = vk::SharingMode::eExclusive;
    swapchainCI.queueFamilyIndexCount = 0;
    swapchainCI.pQueueFamilyIndices = nullptr;
    swapchainCI.preTransform = preTransform;
    swapchainCI.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swapchainCI.presentMode = swapchainPresentMode;
    swapchainCI.clipped = VK_TRUE;
    swapchainCI.oldSwapchain = oldSwapchain;

    this->swapchain =
      CheckReturn(
        this->context->device->createSwapchainKHR(swapchainCI),
        "Swapchain creation"
      );
  }

  // finalize present info
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains    = &swapchain;
  presentInfo.pImageIndices  = &currentImage;

  // destroy old swapchain
  if (static_cast<bool>(oldSwapchain)) {
    for (uint32_t i = 0; i < this->images.size(); ++ i)
      { this->context->device->destroyImageView(this->images[i].view); }
    this->context->device->destroySwapchainKHR(oldSwapchain);
  }

  // TODO
  vk::ImageViewCreateInfo colorAttachmentView;
  colorAttachmentView.format = colorFormat;
    colorAttachmentView.viewType = vk::ImageViewType::e2D;
  { // subresource range
    auto & range = colorAttachmentView.subresourceRange;
    range.aspectMask     = vk::ImageAspectFlagBits::eColor;
    range.levelCount     = 1;
    range.layerCount     = 1;
    range.baseMipLevel   = 0;
    range.baseArrayLayer = 0;
  }

  // get swapchain iamges
  auto swapchainImages =
    context->device->getSwapchainImagesKHR(swapchain).value;

  // get swapchain buffers containing image and imageView
  images.resize(swapchainImages.size());
  for (uint32_t i = 0; i < static_cast<uint32_t>(images.size()); ++ i) {
    images[i].image = swapchainImages[i];
    colorAttachmentView.image = swapchainImages[i];
    images[i].view =
      context->device->createImageView(colorAttachmentView).value;
  }
}

////////////////////////////////////////////////////////////////////////////////
std::vector<vk::Framebuffer> Swapchain::CreateFramebuffers(
  vk::FramebufferCreateInfo fbCI
) {
  std::vector<vk::ImageView> attachments;
  attachments.resize(fbCI.attachmentCount);
  for (size_t i = 0; i < fbCI.attachmentCount; ++ i)
    { attachments[i] = fbCI.pAttachments[i]; }
  fbCI.pAttachments = attachments.data();

  std::vector<vk::Framebuffer> framebuffers;
  framebuffers.resize(this->images.size());
  for (uint32_t i = 0; i < static_cast<uint32_t>(this->images.size()); ++ i) {
    attachments[0] = images[i].view;
    framebuffers[i] = context->device->createFramebuffer(fbCI).value;
  }

  return framebuffers;
}

////////////////////////////////////////////////////////////////////////////////
uint32_t Swapchain::AcquireNextImage(
  vk::Semaphore const& presentCompleteSemaphore
) {
  auto resultValue =
    context->device->acquireNextImageKHR(
      swapchain,
      std::numeric_limits<uint64_t>::max(),
      presentCompleteSemaphore,
      vk::Fence()
    );

  vk::Result result = resultValue.result;
  if (result != vk::Result::eSuccess)
  {
    printf("Invalid acquire result '%s'", vk::to_string(result).c_str());
    return 0;
  }

  this->currentImage = resultValue.value;
  return this->currentImage;
}

////////////////////////////////////////////////////////////////////////////////
vk::Fence const& Swapchain::GetSubmitFence() {
  auto& image = this->images[this->currentImage];
  if (image.fence) {
    vk::Result fenceResult = vk::Result::eTimeout;
    while (vk::Result::eTimeout == fenceResult) {
      fenceResult =
        context->device->waitForFences(
          image.fence
        , VK_TRUE
        , std::numeric_limits<uint64_t>::max()
        );
    }
    context->device->resetFences(image.fence);
  } else {
    image.fence = context->device->createFence(vk::FenceCreateFlags()).value;
  }
  return image.fence;
}

////////////////////////////////////////////////////////////////////////////////
vk::Result Swapchain::QueuePresent(vk::Semaphore const& waitSemaphore) {
  presentInfo.waitSemaphoreCount = waitSemaphore ? 1 : 0;
  presentInfo.pWaitSemaphores = &waitSemaphore;
  return this->context->graphicsQueue.presentKHR(presentInfo);
}

////////////////////////////////////////////////////////////////////////////////
void Swapchain::Cleanup() {
  for (auto const & image : images) {
    if (image.fence) {
      context->device->waitForFences(
        image.fence
      , VK_TRUE
      , std::numeric_limits<uint64_t>::max()
      );
      context->device->destroyFence(image.fence);
    }
    context->device->destroyImageView(image.view);
    // don't destroy vk::Image as it is owned by swapchain, will be destroyed by
    // destroySwapchainKHR
  }
  images.clear();
  context->device->destroySwapchainKHR(swapchain);
  context->instance->destroySurfaceKHR(surface);
}
