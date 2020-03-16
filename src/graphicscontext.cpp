#include "graphicscontext.hpp"

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

  for (auto const & ext : extensions)
    spdlog::info("Extension '{}' enabled", ext);

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
    self.instance = vk::UniqueInstance(vk::createInstance(info).value);
  }

  { // physical device
    self.physicalDevices = self.instance->enumeratePhysicalDevices().value;
    self.physicalDevice         = self.physicalDevices[0];
    self.deviceProperties       = self.physicalDevice.getProperties();
    self.deviceFeatures         = self.physicalDevice.getFeatures();
    self.deviceMemoryProperties = self.physicalDevice.getMemoryProperties();
    self.queueFamilyProperties = self.physicalDevice.getQueueFamilyProperties();
  }

  self.glfwWindow->Construct(glm::uvec2(640, 480));
  self.surface =
    ConstructWindowSurface(*self.glfwWindow, self.instance.get());

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
