/*
 *
 * Copyright (C) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef level_zero_tests_ZE_TEST_HARNESS_MEMORY_HPP
#define level_zero_tests_ZE_TEST_HARNESS_MEMORY_HPP

#include "test_harness_device.hpp"
#include <level_zero/ze_api.h>
#include "gtest/gtest.h"

namespace level_zero_tests {

const auto memory_allocation_sizes =
    ::testing::Values(1, 10, 100, 1000, 10000, 100000);
const auto memory_allocation_alignments =
    ::testing::Values(1, 2, 4, 8, 16, 32, 64);

void *allocate_host_memory(const size_t size);
void *allocate_host_memory(const size_t size, const size_t alignment);
void *allocate_device_memory(const size_t size);
void *allocate_device_memory(const size_t size, const size_t alignment);
void *allocate_device_memory(const size_t size, const size_t alignment,
                             const ze_device_mem_alloc_flag_t flags);
void *allocate_device_memory(const size_t size, const size_t alignment,
                             const ze_device_mem_alloc_flag_t flags,
                             ze_device_handle_t device,
                             ze_driver_handle_t driver);
void *allocate_device_memory(const size_t size, const size_t alignment,
                             const ze_device_mem_alloc_flag_t flags,
                             const uint32_t ordinal,
                             ze_device_handle_t device_handle,
                             ze_driver_handle_t driver);
void *allocate_shared_memory(const size_t size);
void *allocate_shared_memory(const size_t size, const size_t alignment);
void *allocate_shared_memory(const size_t size, const size_t alignment,
                             const ze_device_mem_alloc_flag_t dev_flags,
                             const ze_host_mem_alloc_flag_t host_flags);
void *allocate_shared_memory(const size_t size, const size_t alignment,
                             const ze_device_mem_alloc_flag_t dev_flags,
                             const ze_host_mem_alloc_flag_t host_flags,
                             ze_device_handle_t device);
void allocate_mem(void **memory, ze_memory_type_t mem_type, size_t size);

void free_memory(const void *ptr);
void free_memory(ze_driver_handle_t driver, const void *ptr);

void allocate_mem_and_get_ipc_handle(ze_ipc_mem_handle_t *handle, void **memory,
                                     ze_memory_type_t mem_type);
void allocate_mem_and_get_ipc_handle(ze_ipc_mem_handle_t *handle, void **memory,
                                     ze_memory_type_t mem_type, size_t size);
void get_ipc_handle(ze_ipc_mem_handle_t *handle, void *memory);
void write_data_pattern(void *buff, size_t size, int8_t data_pattern);
void validate_data_pattern(void *buff, size_t size, int8_t data_pattern);
void get_mem_alloc_properties(
    ze_driver_handle_t driver, const void *memory,
    ze_memory_allocation_properties_t *memory_properties);
void get_mem_alloc_properties(
    ze_driver_handle_t driver, const void *memory,
    ze_memory_allocation_properties_t *memory_properties,
    ze_device_handle_t *device);
}; // namespace level_zero_tests
#endif
