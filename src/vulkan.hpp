#pragma once

#ifdef VULKAN_HPP_ASSERT
#error Vulkan HPP Assert already defined
#endif

#include <spdlog/spdlog.h>
#define VULKAN_HPP_ASSERT(X) if (!(X)) { spdlog::error("##X##"); }
#include <vulkan/vulkan.hpp>
