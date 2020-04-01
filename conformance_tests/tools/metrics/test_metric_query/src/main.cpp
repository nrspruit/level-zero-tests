/*
 *
 * Copyright (C) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "gmock/gmock.h"
#include "logging/logging.hpp"
#include "utils/utils.hpp"

int main(int argc, char **argv) {
  ::testing::InitGoogleMock(&argc, argv);
  std::vector<std::string> command_line(argv + 1, argv + argc);
  level_zero_tests::init_logging(command_line);

  ze_result_t result = zeInit(ZE_INIT_FLAG_NONE);
  if (result) {
    throw std::runtime_error("zeInit failed: " +
                             level_zero_tests::to_string(result));
  }
  LOG_TRACE << "Driver initialized";

  result = zetInit(ZE_INIT_FLAG_NONE);
  if (result) {
    throw std::runtime_error("zetInit failed: " +
                             level_zero_tests::to_string(result));
  }
  LOG_TRACE << "Tools API initialized";
  return RUN_ALL_TESTS();
}
