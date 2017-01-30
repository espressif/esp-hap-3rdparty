// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <stddef.h>
#include <stdint.h>
#include "flash_qio_mode.h"
#include "esp_log.h"
#include "rom/spi_flash.h"
#include "soc/spi_struct.h"
#include "sdkconfig.h"

/* SPI flash controller */
#define SPIFLASH SPI1

/* SPI commands (actual on-wire commands not SPI controller bitmasks)
   Suitable for use with the execute_flash_command static function.
*/
#define CMD_RDID       0x9F
#define CMD_WRSR       0x01
#define CMD_WRSR2      0x31 /* Not all SPI flash uses this command */
#define CMD_WREN       0x06
#define CMD_WRDI       0x04
#define CMD_RDSR       0x05
#define CMD_RDSR2      0x35 /* Not all SPI flash uses this command */

static const char *TAG = "qio_mode";

typedef struct __attribute__((packed)) {
    const char *manufacturer;
    uint8_t mfg_id; /* 8-bit JEDEC manufacturer ID */
    uint16_t flash_id; /* 16-bit JEDEC flash chip ID */
    uint16_t id_mask; /* Bits to match on in flash chip ID */
    uint8_t read_status_command;
    uint8_t write_status_command;
    uint8_t status_qio_bit; /* Currently assumes same bit for read/write status */
} qio_info_t;

/* Array of known flash chips and data to enable Quad I/O mode

   Manufacturer & flash ID can be tested by running "esptool.py
   flash_id"

   If manufacturer ID matches, and flash ID ORed with flash ID mask
   matches, enable_qio_mode() will execute "Read Cmd", test if bit
   number "QIE Bit" is set, and if not set it will call "Write Cmd"
   with this bit set.

   Searching of this table stops when the first match is found.

   (This table currently makes a lot of assumptions about how Quad I/O
   mode is enabled, some flash chips in future may require more complex
   handlers - for example a function pointer to a handler function.)
 */
const static qio_info_t chip_data[] = {
/*   Manufacturer,   mfg_id, flash_id, id mask, Read Cmd,  Write Cmd, QIE Bit */
    { "MXIC",        0xC2,   0x2000, 0xFF00,    CMD_RDSR,  CMD_WRSR,  6 },
    { "ISSI",        0x9D,   0x4000, 0xFF00,    CMD_RDSR,  CMD_WRSR,  6 },

    /* Final entry is default entry, if no other IDs have matched.

       This approach works for chips including:
       GigaDevice (mfg ID 0xC8, flash IDs including 4016),
       FM25Q32 (mfg ID 0xA1, flash IDs including 4016)
    */
    { NULL,          0xFF,    0xFFFF, 0xFFFF,   CMD_RDSR2, CMD_WRSR2, 1 }, /* Bit 9 of status register (second byte) */
};

#define NUM_CHIPS (sizeof(chip_data) / sizeof(qio_info_t))

static void enable_qio_mode(uint8_t read_status_command,
                            uint8_t write_status_command,
                            uint8_t status_qio_bit);

/* Generic function to use the "user command" SPI controller functionality
   to send commands to the SPI flash and read the respopnse.

   The command passed here is always the on-the-wire command given to the SPI flash unit.
*/
static uint32_t execute_flash_command(uint8_t command, uint32_t mosi_data, uint8_t mosi_len, uint8_t miso_len);

void bootloader_enable_qio_mode(void)
{
    uint32_t raw_flash_id;
    uint8_t mfg_id;
    uint16_t flash_id;
    int i;

    ESP_LOGD(TAG, "Probing for QIO mode enable...");
    SPI_Wait_Idle(&g_rom_flashchip);

    /* Set up some of the SPIFLASH user/ctrl variables which don't change
       while we're probing using execute_flash_command() */
    SPIFLASH.ctrl.val = 0;
    SPIFLASH.user.usr_dummy = 0;
    SPIFLASH.user.usr_addr = 0;
    SPIFLASH.user.usr_command = 1;
    SPIFLASH.user2.usr_command_bitlen = 7;

    raw_flash_id = execute_flash_command(CMD_RDID, 0, 0, 24);
    ESP_LOGD(TAG, "Raw SPI flash chip id 0x%x", raw_flash_id);

    mfg_id = raw_flash_id & 0xFF;
    flash_id = (raw_flash_id >> 16) | (raw_flash_id & 0xFF00);
    ESP_LOGD(TAG, "Manufacturer ID 0x%02x chip ID 0x%04x", mfg_id, flash_id);

    for (i = 0; i < NUM_CHIPS-1; i++) {
        const qio_info_t *chip = &chip_data[i];
        if (mfg_id == chip->mfg_id && (flash_id & chip->id_mask) == (chip->flash_id & chip->id_mask)) {
            ESP_LOGI(TAG, "Enabling QIO for flash chip %s", chip_data[i].manufacturer);
            break;
        }
    }

    if (i == NUM_CHIPS - 1) {
        ESP_LOGI(TAG, "Enabling default flash chip QIO");
    }

    enable_qio_mode(chip_data[i].read_status_command,
                    chip_data[i].write_status_command,
                    chip_data[i].status_qio_bit);
}

static void enable_qio_mode(uint8_t read_status_command,
                            uint8_t write_status_command,
                            uint8_t status_qio_bit)
{
    uint32_t status_len = (status_qio_bit + 8) & ~7; /* 8, 16, 24 bit status values */
    uint32_t status;

    SPI_Wait_Idle(&g_rom_flashchip);

    status = execute_flash_command(read_status_command, 0, 0, status_len);
    ESP_LOGD(TAG, "Initial flash chip status 0x%x", status);

    if ((status & (1<<status_qio_bit)) == 0) {
        execute_flash_command(CMD_WREN, 0, 0, 0);
        execute_flash_command(write_status_command, status | (1<<status_qio_bit), status_len, 0);

        SPI_Wait_Idle(&g_rom_flashchip);

        status = execute_flash_command(read_status_command, 0, 0, status_len);
        ESP_LOGD(TAG, "Updated flash chip status 0x%x", status);
        if ((status & (1<<status_qio_bit)) == 0) {
            ESP_LOGE(TAG, "Failed to set QIE bit, not enabling QIO mode");
            return;
        }

    } else {
        ESP_LOGD(TAG, "QIO mode already enabled in flash");
    }

    ESP_LOGD(TAG, "Enabling QIO mode...");

    SpiFlashRdMode mode;
#if CONFIG_FLASHMODE_QOUT
    mode = SPI_FLASH_QOUT_MODE;
#else
    mode = SPI_FLASH_QIO_MODE;
#endif
    SPIMasterReadModeCnfig(mode);
}

static uint32_t execute_flash_command(uint8_t command, uint32_t mosi_data, uint8_t mosi_len, uint8_t miso_len)
{
    SPIFLASH.user2.usr_command_value = command;
    SPIFLASH.user.usr_miso = miso_len > 0;
    SPIFLASH.miso_dlen.usr_miso_dbitlen = miso_len ? (miso_len - 1) : 0;
    SPIFLASH.user.usr_mosi = mosi_len > 0;
    SPIFLASH.mosi_dlen.usr_mosi_dbitlen = mosi_len ? (mosi_len - 1) : 0;
    SPIFLASH.data_buf[0] = mosi_data;

    SPIFLASH.cmd.usr = 1;
    while(SPIFLASH.cmd.usr != 0)
    { }

    return SPIFLASH.data_buf[0];
}
