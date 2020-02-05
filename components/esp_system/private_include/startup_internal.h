// Copyright 2015-2018 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "esp_attr.h"
typedef struct {
  void (*fn)(void);
  uint32_t cores;
} esp_system_init_fn_t;

/*
 * Declare an component initialization function that will execute on the specified cores (ex. if BIT0 == 1, will execute
 * on CORE0, CORE1 if BIT1 and so on).
 * 
 * @note Initialization functions should be placed in a compilation unit where at least one other
 * symbol is referenced 'meaningfully' in another compilation unit, otherwise this gets discarded during linking. (By
 * 'meaningfully' we mean the reference should not itself get optimized out by the compiler/discarded by the linker).
 */
#define ESP_SYSTEM_INIT_FN(f, c, ...) \
static void  __attribute__((used)) __VA_ARGS__ __esp_system_init_fn_##f(void); \
static __attribute__((used)) esp_system_init_fn_t _SECTION_ATTR_IMPL(".esp_system_init_fn", f) \
                    esp_system_init_fn_##f = { .fn = ( __esp_system_init_fn_##f), .cores = (c) }; \
static __attribute__((used)) __VA_ARGS__ void __esp_system_init_fn_##f(void) // [refactor-todo] this can be made public API if we allow components to declare init functions, 
                                                                             // instead of calling them explicitly

