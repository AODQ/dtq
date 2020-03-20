#pragma once


#ifndef NDEBUG

#include <spdlog/spdlog.h>
#include <vulkan/vulkan.hpp>

#include <utility>

template <typename Result>
decltype(Result::value) CheckReturn(Result && result , char const * errMsg) {
  if (result.result != vk::Result::eSuccess) {
    spdlog::critical(
      "Error: '{}' Reason: {}",
      errMsg, vk::to_string(result.result)
    );
    spdlog::dump_backtrace();
  }
  return result.value;
}

#else
#define CheckReturn(R, U...) (R .value)
#endif
