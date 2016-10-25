// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
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

#ifndef __ESP_NEWLIB_H__
#define __ESP_NEWLIB_H__

/**
 * Function which sets up syscall table used by newlib functions in ROM.
 *
 * Called from the startup code, not intended to be called from application
 * code.
 */
void esp_setup_syscalls();


#endif //__ESP_NEWLIB_H__
