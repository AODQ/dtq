#include "util.hpp"
#include "glfw.hpp"
#include "graphicscontext.hpp"
#include "swapchain.hpp"

#include <iostream>
#include <thread>

////////////////////////////////////////////////////////////////////////////////
int main() {
  auto context = GraphicsContext::Construct();
  LogDiagnosticInfo(context);

  auto swapchain = Swapchain(context, context.surface);
  swapchain.Construct(glm::vec2(640, 480));

  vk::UniqueRenderPass renderPass;
  { // -- create renderpass
    std::vector<vk::AttachmentDescription> attachments;
    std::vector<vk::AttachmentReference> attachmentReferences;
    std::vector<vk::SubpassDescription> subpasses;
    std::vector<vk::SubpassDependency> subpassDependencies;

    { // -- color attachment
      vk::AttachmentDescription desc;
      desc.format = swapchain.colorFormat;
      desc.loadOp = vk::AttachmentLoadOp::eClear;
      desc.storeOp = vk::AttachmentStoreOp::eStore;
      desc.initialLayout = vk::ImageLayout::eUndefined;
      desc.finalLayout = vk::ImageLayout::ePresentSrcKHR;
      attachments.push_back(desc);
    }

    { // -- color attachment ref
      vk::AttachmentReference ref;
      ref.attachment = 0;
      ref.layout = vk::ImageLayout::eColorAttachmentOptimal;
      attachmentReferences.push_back(ref);
    }

    { // -- subpass
      vk::SubpassDescription desc;
      desc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
      desc.colorAttachmentCount = 1;
      desc.pColorAttachments = attachmentReferences.data();
      subpasses.push_back(desc);
    }

    { // -- subpass dependency
      vk::SubpassDependency dep;

      dep.srcSubpass = 0;
      dep.srcAccessMask = vk::AccessFlagBits::eMemoryRead;
      dep.srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;

      dep.dstSubpass = VK_SUBPASS_EXTERNAL;
      dep.dstAccessMask =
        vk::AccessFlagBits::eColorAttachmentRead
      | vk::AccessFlagBits::eColorAttachmentWrite;
      dep.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;

      subpassDependencies.push_back(dep);
    }

    { // -- renderpass create info
      vk::RenderPassCreateInfo info;
      info.attachmentCount = static_cast<uint32_t>(attachments.size());
      info.pAttachments = attachments.data();
      info.subpassCount = static_cast<uint32_t>(subpasses.size());
      info.pSubpasses = subpasses.data();
      info.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
      info.pDependencies = subpassDependencies.data();
      renderPass = vk::UniqueRenderPass {
        context.device->createRenderPass(info).value
      };
    }
  }

  std::vector<vk::Framebuffer> framebuffers;
  { // create framebuffers
    std::array<vk::ImageView, 1> imageViews;
    vk::FramebufferCreateInfo framebufferCI;
    framebufferCI.renderPass = *renderPass;
    framebufferCI.attachmentCount = static_cast<uint32_t>(imageViews.size());
    framebufferCI.pAttachments = imageViews.data();
    framebufferCI.width = swapchain.swapchainExtent.width;
    framebufferCI.height = swapchain.swapchainExtent.height;
    framebufferCI.layers = 1;

    framebuffers = swapchain.CreateFramebuffers(framebufferCI);
  }

  std::vector<vk::CommandBuffer> commandBuffers;
  { // allocate command buffres
    vk::CommandBufferAllocateInfo commandBufferAI;
    commandBufferAI.commandPool = *context.commandPool;
    commandBufferAI.commandBufferCount = swapchain.ImageLength();
    commandBufferAI.level = vk::CommandBufferLevel::ePrimary;
    commandBuffers =
      CheckReturn(
        context.device->allocateCommandBuffers(commandBufferAI),
        "Allocating command buffers"
      );
  }

  { // assign commands to command buffers
    static const std::vector<vk::ClearColorValue> clearColors {
      vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f})
    , vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 1.0f, 0.0f})
    , vk::ClearColorValue(std::array<float, 4>{0.0f, 1.0f, 0.0f, 0.0f})
    , vk::ClearColorValue(std::array<float, 4>{0.0f, 1.0f, 1.0f, 0.0f})
    , vk::ClearColorValue(std::array<float, 4>{1.0f, 0.0f, 0.0f, 0.0f})
    , vk::ClearColorValue(std::array<float, 4>{1.0f, 0.0f, 1.0f, 0.0f})
    , vk::ClearColorValue(std::array<float, 4>{1.0f, 1.0f, 0.0f, 0.0f})
    , vk::ClearColorValue(std::array<float, 4>{1.0f, 1.0f, 1.0f, 0.0f})
    };

    vk::ClearValue clearValue;
    auto renderPassBI = vk::RenderPassBeginInfo {
      *renderPass,
      {}, // no framebuffer explicitly set
      { {}, swapchain.swapchainExtent },
      1, // clear values
      &clearValue
    };

    for (size_t i = 0; i < swapchain.ImageLength(); ++ i) {
      auto const & commandBuffer = commandBuffers[i];
      clearValue.color = clearColors[i % clearColors.size()];
      renderPassBI.framebuffer = framebuffers[i];
      commandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
      commandBuffer.begin(vk::CommandBufferBeginInfo{});
      commandBuffer.beginRenderPass(renderPassBI, vk::SubpassContents::eInline);
      commandBuffer.endRenderPass();
      commandBuffer.end();
    }
  }

  vk::UniqueSemaphore acquireComplete;
  vk::UniqueSemaphore renderComplete;
  {
    acquireComplete = vk::UniqueSemaphore(context.device->createSemaphore({}).value);
    renderComplete  = vk::UniqueSemaphore(context.device->createSemaphore({}).value);
  }

  while (!ShouldWindowClose(*context.glfwWindow))
  {
    PollEvents(*context.glfwWindow);

    uint32_t currentBuffer = swapchain.AcquireNextImage(*acquireComplete);

    vk::Fence submitFence = swapchain.GetSubmitFence();

    Submit(
      context
    , commandBuffers[currentBuffer]
    , *acquireComplete
    , vk::PipelineStageFlagBits::eColorAttachmentOutput
    , *renderComplete
    , submitFence
    );

    swapchain.QueuePresent(*renderComplete);
  }

  context.graphicsQueue.waitIdle();
  context.device->waitIdle();

  return 0;
}
