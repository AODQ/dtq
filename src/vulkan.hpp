#pragma once

#include <spdlog/spdlog.h>
#ifndef VULKAN_HPP_ASSERT
#define VULKAN_HPP_ASSERT(X) if (!(X)) { spdlog::error("##X##"); }
#endif
#include <vulkan/vulkan.hpp>
