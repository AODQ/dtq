#include "glfw.hpp"
#include "graphicscontext.hpp"
#include "swapchain.hpp"

#include <iostream>
#include <thread>

////////////////////////////////////////////////////////////////////////////////
int main() {
  auto context = GraphicsContext::Construct();
  LogDiagnosticInfo(context);

  Swapchain swapchain;
  swapchain.SetWindowSurface(context.surface);
  swapchain.Construct(glm::vec2(640, 480));

  vk::RenderPass renderPass;
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
      dep.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
      dep.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;

      dep.dstSubpass = VK_SUBPASS_EXTERNAL;
      dep.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead;
      dep.dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;

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
      renderPass = context.device->createRenderPass(info).value;
    }
  }

  return 0;
}
