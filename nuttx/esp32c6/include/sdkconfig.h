/*
 * Copyright (c) 2021 Espressif Systems (Shanghai) Co., Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <nuttx/config.h>

#define CONFIG_ESP_REV_MIN_FULL 0
#define CONFIG_ESP_REV_MAX_FULL 99
#define CONFIG_IDF_TARGET_ARCH_RISCV 1
#define CONFIG_IDF_TARGET_ESP32C6 1
#define CONFIG_IDF_FIRMWARE_CHIP_ID 0x000D
#define CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ CONFIG_ESPRESSIF_CPU_FREQ_MHZ
#define CONFIG_RTC_CLK_SRC_INT_RC 1
#define CONFIG_RTC_CLK_CAL_CYCLES 1024
#define CONFIG_RTC_CLOCK_BBPLL_POWER_ON_WITH_USB 1
#define CONFIG_XTAL_FREQ 40

#define CONFIG_HAL_SYSTIMER_USE_ROM_IMPL 1
#define CONFIG_HAL_WDT_USE_ROM_IMPL 1

#define CONFIG_MMU_PAGE_SIZE 0x10000
#define SOC_MMU_PAGE_SIZE CONFIG_MMU_PAGE_SIZE
#define CONFIG_LOG_TIMESTAMP_SOURCE_RTOS 1

#define CONFIG_IDF_ENV_FPGA 0

#define CONFIG_ESPTOOLPY_FLASHMODE_DIO 1

#ifdef CONFIG_ESPRESSIF_BROWNOUT_DET
#define CONFIG_ESP_BROWNOUT_DET_LVL CONFIG_ESPRESSIF_BROWNOUT_DET_LVL
#else
#define CONFIG_ESP_BROWNOUT_DET 0
#endif

#if (defined(CONFIG_UART0_SERIAL_CONSOLE) && defined(CONFIG_ESPRESSIF_UART0)) || defined(CONFIG_ESPRESSIF_USBSERIAL)
#define CONFIG_ESP_CONSOLE_UART_NUM 0
#elif defined(CONFIG_UART1_SERIAL_CONSOLE) && defined(CONFIG_ESPRESSIF_UART1)
#define CONFIG_ESP_CONSOLE_UART_NUM 1
#endif

#if defined(CONFIG_DEBUG_INFO)
#define CONFIG_LOG_DEFAULT_LEVEL 3
#define CONFIG_LOG_MAXIMUM_LEVEL 3
#elif defined(CONFIG_DEBUG_WARN)
#define CONFIG_LOG_DEFAULT_LEVEL 2
#define CONFIG_LOG_MAXIMUM_LEVEL 2
#elif defined(CONFIG_DEBUG_ERROR)
#define CONFIG_LOG_DEFAULT_LEVEL 1
#define CONFIG_LOG_MAXIMUM_LEVEL 1
#else
#define CONFIG_LOG_DEFAULT_LEVEL 0
#define CONFIG_LOG_MAXIMUM_LEVEL 0
#endif
