#include "graphicscontext.hpp"

#include "util.hpp"

#include <set>

#include <spdlog/spdlog.h>

////////////////////////////////////////////////////////////////////////////////
GraphicsContext GraphicsContext::Construct() {
  GraphicsContext self;

  self.glfwWindow = std::make_unique<GlfwWindow>();

  // get extensions
  std::vector<char const*> extensions = {
    VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
  };
  auto requiredInstanceExt = RequiredInstanceExtensions(*self.glfwWindow);
  for (auto const & ext : requiredInstanceExt)
    extensions.emplace_back(ext.c_str());

  std::vector<char const*> layers = {
  #ifndef NDEBUG
    "VK_LAYER_LUNARG_standard_validation"
  #endif
  };

  for (auto const & ext : extensions)
    spdlog::info("Extension '{}' enabled", ext);

  for (auto const & layer : layers)
    spdlog::info("Layer '{}' enabled", layer);

  { // Vulkan instance
    vk::ApplicationInfo appInfo;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = "Demo Toad Quill";
    appInfo.applicationVersion = 1;
    appInfo.pEngineName = "DTQ";
    appInfo.engineVersion = 1;
    appInfo.apiVersion = VK_API_VERSION_1_2;

    vk::InstanceCreateInfo info;
    info.pApplicationInfo = &appInfo;
    info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    info.ppEnabledExtensionNames = extensions.data();
    info.enabledLayerCount = static_cast<uint32_t>(layers.size());
    info.ppEnabledLayerNames = layers.data();
    self.instance = vk::UniqueInstance(vk::createInstance(info).value);
  }

  { // physical device
    self.physicalDevices = self.instance->enumeratePhysicalDevices().value;
    self.physicalDevice         = self.physicalDevices[0];
    self.deviceProperties       = self.physicalDevice.getProperties();
    self.deviceFeatures         = self.physicalDevice.getFeatures();
    self.deviceMemoryProperties = self.physicalDevice.getMemoryProperties();
  }

  self.glfwWindow->Construct(glm::uvec2(640, 480));
  self.surface =
    ConstructWindowSurface(*self.glfwWindow, self.instance.get());

  { // queue
    self.queueFamilyProperties = self.physicalDevice.getQueueFamilyProperties();
    using QueueTuple = std::tuple<vk::QueueFlags, uint32_t*>;
    for (auto it : {
      QueueTuple { vk::QueueFlagBits::eGraphics, &self.graphicsQueueIdx },
      QueueTuple { vk::QueueFlagBits::eCompute,  &self.computeQueueIdx  },
      QueueTuple { vk::QueueFlagBits::eTransfer, &self.transferQueueIdx },
    }) {
      *std::get<1>(it) = FindQueue(self, std::get<0>(it), self.surface);
    }
  }

  { // device
    vk::DeviceCreateInfo deviceCI;

    std::vector<vk::DeviceQueueCreateInfo> deviceQueues;
    std::vector<std::vector<float>> deviceQueuePriorities;
    { // queue / properties
      using QueueTuple = std::tuple<uint32_t, float>;

      for (auto it : {
        QueueTuple { self.graphicsQueueIdx, 0.0f },
        QueueTuple { self.computeQueueIdx,  0.0f },
        QueueTuple { self.transferQueueIdx, 0.0f },
      }) {
        // check not already in list
        bool inList = false;
        for (auto const & deviceQueue : deviceQueues) {
          if (deviceQueue.queueFamilyIndex == std::get<0>(it)) {
            inList = true;
            break;
          }
        }
        if (inList) { continue; }

        // emplace a vector size queueCount each with given priority
        deviceQueuePriorities.emplace_back(
          self.queueFamilyProperties[std::get<0>(it)].queueCount,
          std::get<1>(it)
        );

        // device queue pointing to emplaced device queue priority
        auto deviceQueueCI = vk::DeviceQueueCreateInfo{{}, std::get<0>(it)};
        deviceQueueCI.pQueuePriorities = deviceQueuePriorities.back().data();
        deviceQueueCI.queueCount =
          static_cast<uint32_t>(deviceQueuePriorities.back().size());
        deviceQueues.emplace_back(deviceQueueCI);
      }

      deviceCI.queueCreateInfoCount
        = static_cast<uint32_t>(deviceQueues.size());
      deviceCI.pQueueCreateInfos = deviceQueues.data();
    }

    std::set<std::string> deviceExtensions;
    deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    /* for (auto & ext : extensions) deviceExtensions.insert(ext); */

    for (auto const & ext : deviceExtensions)
      spdlog::info("Device Extension '{}' enabled", ext);

    std::vector<char const *> enabledExtensions;
    for (auto const & extension : deviceExtensions)
      { enabledExtensions.emplace_back(extension.c_str()); }

    if (
      DeviceExtensionPresent(
        self.physicalDevice
      , VK_EXT_DEBUG_MARKER_EXTENSION_NAME
      )
    ) {
      enabledExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
      self.enableDebugMarkers = true;
    }

    if (!enabledExtensions.empty()) {
      deviceCI.enabledExtensionCount =
        static_cast<uint32_t>(enabledExtensions.size());
      deviceCI.ppEnabledExtensionNames = enabledExtensions.data();
    }

    self.device =
      vk::UniqueDevice(
        CheckReturn(
          self.physicalDevice.createDevice(deviceCI),
          "Could not create device"
        )
      );
  }

  { // queue again
    self.graphicsQueue = self.device->getQueue(self.graphicsQueueIdx, 0);
  }

  { // command pool
    vk::CommandPoolCreateInfo commandPoolCI;
    commandPoolCI.queueFamilyIndex = self.graphicsQueueIdx;
    commandPoolCI.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    self.commandPool =
      vk::UniqueCommandPool(
        CheckReturn(
          self.device->createCommandPool(commandPoolCI),
          "Creating command pool"
        )
      );
  }

  return self;
}

////////////////////////////////////////////////////////////////////////////////
void LogDiagnosticInfo(GraphicsContext const & self) {
  spdlog::info("Device name: '{}'", self.deviceProperties.deviceName);
  spdlog::info(
    "Device type: {}",
    vk::to_string(self.deviceProperties.deviceType)
  );
  spdlog::info("Memory heaps: {}", self.deviceMemoryProperties.memoryHeapCount);
  for (size_t i = 0; i < self.deviceMemoryProperties.memoryHeapCount; ++ i) {
    auto const & heap = self.deviceMemoryProperties.memoryHeaps[i];
    spdlog::info(
      "\tHeap {} flags '{}' size {}"
    , i, vk::to_string(heap.flags), heap.size/(1024*1024)
    );
  }

  auto queueProps = self.queueFamilyProperties;

  for (size_t i = 0; i < queueProps.size(); ++ i)
  {
    spdlog::info(
      "Queue family {} Flags '{}' count {}"
    , i, vk::to_string(queueProps[i].queueFlags), queueProps[i].queueCount
    );
  }
}

////////////////////////////////////////////////////////////////////////////////
uint32_t FindQueue(
  GraphicsContext const & self
, vk::QueueFlags const & desiredFlags
, vk::SurfaceKHR const & presentSurface
) {
  uint32_t bestMatch = VK_QUEUE_FAMILY_IGNORED;
  VkQueueFlags bestMatchExtraFlag = VK_QUEUE_FLAG_BITS_MAX_ENUM;

  for (size_t i = 0; i < self.queueFamilyProperties.size(); ++ i)
  {
    auto currentFlags = self.queueFamilyProperties[i].queueFlags;

    if (!(currentFlags & desiredFlags)) { continue; }

    if (presentSurface
     && self.physicalDevice.getSurfaceSupportKHR(i, presentSurface).value
     == VK_FALSE
    ) {
      continue;
    }

    auto currentExtraFlags =
      static_cast<VkQueueFlags>((currentFlags & ~desiredFlags));

    if (currentExtraFlags == 0) { return i; }

    if (bestMatch == VK_QUEUE_FAMILY_IGNORED
     || currentExtraFlags < bestMatchExtraFlag
    ) {
      bestMatch = i;
      bestMatchExtraFlag = currentExtraFlags;
    }
  }

  return bestMatch;
}

////////////////////////////////////////////////////////////////////////////////
bool DeviceExtensionPresent(
  vk::PhysicalDevice const & physicalDevice
, std::string const & extension
) {
  std::set<std::string> extensions;
  for (
    auto const & ext
  : CheckReturn(
      physicalDevice.enumerateDeviceExtensionProperties(),
      "Could not enumerate device extension properties"
    )
  ) { extensions.insert(ext.extensionName); }
  return extensions.count(extension) != 0;
}

////////////////////////////////////////////////////////////////////////////////
void Submit(
  GraphicsContext const & context
, vk::CommandBuffer const & commandBuffer
, vk::Semaphore const & wait
, vk::PipelineStageFlags waitStage
, vk::Semaphore const & signal
, vk::Fence const & fence
) {
  vk::SubmitInfo submitInfo;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &signal;

  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &wait;

  submitInfo.pWaitDstStageMask = &waitStage;

  submitInfo.signalSemaphoreCount = 1;
  context.graphicsQueue.submit(submitInfo, fence);
}
