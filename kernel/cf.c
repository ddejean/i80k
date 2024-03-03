// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include "cf.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "clock.h"
#include "cpu.h"

// Register
#define REG_DATA 0x1f0
#define REG_ERROR 0x1f1
#define REG_FEATURES 0x1f1
#define REG_SECTOR_COUNT 0x1f2
#define REG_LBA_LOW 0x1f3
#define REG_LBA_MID 0x1f4
#define REG_LBA_HIGH 0x1f5
#define REG_CARD_HEAD 0x1f6
#define REG_CMD 0x1f7
#define REG_STATUS 0x1f7

// Status register.
#define SR_ERR (1 << 2)
#define SR_CORR (1 << 2)
#define SR_DRQ (1 << 3)
#define SR_DSC (1 << 4)
#define SR_DWF (1 << 5)
#define SR_RDY (1 << 6)
#define SR_BSY (1 << 7)

// Error register.
#define ERR_GENERIC 1
#define ERR_ABORT (1 << 2)
#define ERR_ID_NOT_FOUND (1 << 4)
#define ERR_UNCORRECTABLE (1 << 6)
#define ERR_BAD_BLOCK (1 << 7)

// Card/Head register.
#define CHR_LBA (1 << 6)
#define CHR_CARD0 0xa0
#define CHR_CARD1 0xb0
#define CHR_HEAD_MASK 0xf

// Device Control Register
#define DCR_INT_DISABLE (1 << 1)
#define DCR_RESET (1 << 2)

// Commands
#define CMD_READ_SECTORS 0x20
#define CMD_WRITE_SECTORS 0x30
#define CMD_IDENTIFY 0xec
#define CMD_SET_FEATURE 0xef

#define FEAT_8BIT_ENABLE 0x01
#define FEAT_WRITE_CACHE_ENABLE 0x02
#define FEAT_SET_XFER_MODE 0x03
#define FEAT_8BIT_DISABLE 0x81

// Pull a word from the register |reg|.
#define DATA_PULL_WORD(reg) (inb(reg) | (inb(reg) << 8))

// Pull |count| bytes and drop them.
#define DATA_PULL_AND_SKIP(count)     \
    for (int i = 0; i < count; i++) { \
        inb(REG_DATA);                \
    }

#define CF_SIGNATURE ((uint16_t)0x848A)

struct cf_identity {
    uint16_t signature;
    uint16_t default_cyl_count;
    uint16_t default_heads_count;
    uint16_t default_sec_per_track;
    uint32_t sec_per_card;
    char serial[21];
    uint16_t ecc_bytes_count;
    char firmware[9];
    char model[41];
    uint16_t max_sec_count;
    uint16_t capabilities;
    uint16_t pio_timing_mode;
    uint16_t field_validity;
    uint16_t curr_cyl_count;
    uint16_t curr_heads_count;
    uint16_t curr_sec_per_track;
    uint32_t cur_capacity;
    uint16_t multiple_sector_setting;
    uint32_t total_sec_count;
    uint16_t advanced_pio_modes;
    uint16_t min_pio_xfer_cycle_wo_fc;
    uint16_t min_pio_xfer_cycle_with_fc;
    uint16_t feat_cmd_supported[3];
    uint16_t feat_cmd_enabled[3];
    uint16_t security_erase_time;
    uint16_t enhanced_security_erase_time;
    uint16_t adv_pm_value;
    uint16_t security_status;
    uint16_t power_req;
    uint16_t key_mgmt_schemes;
};

static bool cf_set_feature(uint8_t feature, uint8_t config) {
    outb(REG_FEATURES, feature);
    outb(REG_SECTOR_COUNT, config);
    outb(REG_CARD_HEAD, CHR_CARD0);
    outb(REG_CMD, CMD_SET_FEATURE);

    while (inb(REG_STATUS) & SR_BSY) {
        udelay(10);
    }

    uint8_t byte = inb(REG_STATUS);
    if (byte & SR_ERR) {
        byte = inb(REG_ERROR);
        if (byte & ERR_ABORT) {
            return false;
        }
    }
    return true;
}

static bool cf_identify(struct cf_identity *id) {
    uint16_t word;

    outb(REG_CARD_HEAD, CHR_CARD0);
    outb(REG_CMD, CMD_IDENTIFY);

    while (inb(REG_STATUS) & SR_BSY) {
        udelay(10);
    }

    // Pull and parse the result of the IDENTIFY command according to Compact
    // Flash specification 2.0 - §6.2.1.6.

    // General configuration - Compact Flash signature
    id->signature = DATA_PULL_WORD(REG_DATA);
    // Default number of cylinders
    id->default_cyl_count = DATA_PULL_WORD(REG_DATA);
    // Reserved
    DATA_PULL_AND_SKIP(2);
    // Default number of heads
    id->default_heads_count = DATA_PULL_WORD(REG_DATA);
    // Obsolete
    DATA_PULL_AND_SKIP(4);
    // Default number of sectors per track.
    id->default_sec_per_track = DATA_PULL_WORD(REG_DATA);
    // Number of sectors per card
    word = DATA_PULL_WORD(REG_DATA);
    id->sec_per_card = ((uint32_t)word) << 16;
    word = DATA_PULL_WORD(REG_DATA);
    id->sec_per_card |= word;
    // Obsolete
    DATA_PULL_AND_SKIP(2);
    // Serial number on 20 bytes.
    for (int i = 0; i < 20; i += 2) {
        id->serial[i] = (char)(inb(REG_DATA));
        id->serial[i + 1] = (char)(inb(REG_DATA));
    }
    id->serial[20] = '\0';
    // Remove trailing spaces.
    for (int i = 20; i >= 0; i--) {
        if (id->serial[i] && id->serial[i] != ' ') {
            break;
        }
        id->serial[i] = '\0';
    }
    // Obsolete
    DATA_PULL_AND_SKIP(4);
    // Number of ECC bytes passed on read/write long commands.
    id->ecc_bytes_count = DATA_PULL_WORD(REG_DATA);
    // Firmware revision in ASCII.
    for (int i = 0; i < 8; i += 2) {
        id->firmware[i + 1] = inb(REG_DATA);
        id->firmware[i] = inb(REG_DATA);
    }
    id->firmware[8] = '\0';
    // Remove trailing spaces.
    for (int i = 8; i >= 0; i--) {
        if (id->firmware[i] && id->firmware[i] != ' ') {
            break;
        }
        id->firmware[i] = '\0';
    }
    // Model number in ASCII.
    for (int i = 0; i < 40; i += 2) {
        id->model[i + 1] = inb(REG_DATA);
        id->model[i] = inb(REG_DATA);
    }
    id->model[40] = '\0';
    // Remove trailing spaces.
    for (int i = 40; i >= 0; i--) {
        if (id->model[i] && id->model[i] != ' ') {
            break;
        }
        id->model[i] = '\0';
    }
    // Maximum number of sector on read/write multiple command.
    id->max_sec_count = DATA_PULL_WORD(REG_DATA);
    // Reserved.
    DATA_PULL_AND_SKIP(2);
    // Capabilities.
    id->capabilities = DATA_PULL_WORD(REG_DATA);
    // Reserved.
    DATA_PULL_AND_SKIP(2);
    // PIO data transfer cycle timing mode.
    id->pio_timing_mode = DATA_PULL_WORD(REG_DATA);
    // Obsolete.
    DATA_PULL_AND_SKIP(2);
    // Field validity.
    id->field_validity = DATA_PULL_WORD(REG_DATA);
    // Current number of cylinders.
    id->curr_cyl_count = DATA_PULL_WORD(REG_DATA);
    // Current number of heads.
    id->curr_heads_count = DATA_PULL_WORD(REG_DATA);
    // Current sectors per track.
    id->curr_sec_per_track = DATA_PULL_WORD(REG_DATA);
    // Current capacity in sectors.
    word = DATA_PULL_WORD(REG_DATA);
    id->cur_capacity = (uint32_t)word;
    word = DATA_PULL_WORD(REG_DATA);
    id->cur_capacity |= ((uint32_t)word) << 16;
    // Multiple sector setting.
    id->multiple_sector_setting = DATA_PULL_WORD(REG_DATA);
    // Total number of sectors addressable in LBA mode.
    word = DATA_PULL_WORD(REG_DATA);
    id->total_sec_count = word;
    word = DATA_PULL_WORD(REG_DATA);
    id->total_sec_count |= ((uint32_t)word) << 16;
    // Reserved.
    DATA_PULL_AND_SKIP(4);
    // Advanced PIO modes supported.
    id->advanced_pio_modes = DATA_PULL_WORD(REG_DATA);
    // Reserved.
    DATA_PULL_AND_SKIP(4);
    // Minimum PIO transfer cycle time without flow control.
    id->min_pio_xfer_cycle_wo_fc = DATA_PULL_WORD(REG_DATA);
    // Minimum PIO transfer cycle time with IORDY flow control.
    id->min_pio_xfer_cycle_with_fc = DATA_PULL_WORD(REG_DATA);
    // Reserved.
    DATA_PULL_AND_SKIP(24);
    // Features/command sets supported.
    id->feat_cmd_supported[0] = DATA_PULL_WORD(REG_DATA);
    id->feat_cmd_supported[1] = DATA_PULL_WORD(REG_DATA);
    id->feat_cmd_supported[2] = DATA_PULL_WORD(REG_DATA);
    // Features/command sets enabled.
    id->feat_cmd_enabled[0] = DATA_PULL_WORD(REG_DATA);
    id->feat_cmd_enabled[1] = DATA_PULL_WORD(REG_DATA);
    id->feat_cmd_enabled[2] = DATA_PULL_WORD(REG_DATA);
    // Reserved.
    DATA_PULL_AND_SKIP(2);
    // Time required for Security erase unit completion.
    id->security_erase_time = DATA_PULL_WORD(REG_DATA);
    // Time required for Enhanced Security erase unit completion.
    id->enhanced_security_erase_time = DATA_PULL_WORD(REG_DATA);
    // Current Advanced power management value.
    id->adv_pm_value = DATA_PULL_WORD(REG_DATA);
    // Reserved.
    DATA_PULL_AND_SKIP(72);
    // Security status.
    id->security_status = DATA_PULL_WORD(REG_DATA);
    // Vendor unique bytes.
    DATA_PULL_AND_SKIP(64);
    // Power requirement description.
    id->power_req = DATA_PULL_WORD(REG_DATA);
    // Reserved.
    DATA_PULL_AND_SKIP(2);
    // Key management schemes supported.
    id->key_mgmt_schemes = DATA_PULL_WORD(REG_DATA);
    // Reserved.
    DATA_PULL_AND_SKIP(166);

    if (id->signature != CF_SIGNATURE) {
        return false;
    }
    return true;
}

void cf_initialize(void) {
    struct cf_identity cf_id;

    if (!cf_set_feature(FEAT_8BIT_ENABLE, 0)) {
        printf("CF: failed to enable 8 bits mode.\n");
        return;
    }

    if (!cf_identify(&cf_id)) {
        printf("CF: failed to read identity.\n");
        return;
    }

    printf("CF: model: %s, revision: %s, serial number: %s\n", cf_id.model,
           cf_id.firmware, cf_id.serial);
    printf("CF: %lu sectors of %d bytes, capacity: %luMB\n", cf_id.cur_capacity,
           CF_SECTOR_SIZE, (cf_id.cur_capacity * CF_SECTOR_SIZE) / 1000000);
}

void cf_read_blocks(uint32_t addr, unsigned int count, char *buffer) {
    // TODO: check addr is within the range.
    if (!count || count > 256 || !buffer) {
        return;
    }

    // 0 is a special value for 256.
    outb(REG_SECTOR_COUNT, count < 256 ? count : 0);
    outb(REG_LBA_LOW, addr);
    outb(REG_LBA_MID, addr >> 8);
    outb(REG_LBA_HIGH, addr >> 16);
    outb(REG_CARD_HEAD, CHR_CARD0 | CHR_LBA | ((addr >> 24) & CHR_HEAD_MASK));
    outb(REG_CMD, CMD_READ_SECTOR);

    for (unsigned int sector = 0; sector < count; sector++) {
        while (inb(REG_STATUS) & SR_BSY) {
            udelay(10);
        }
        while ((inb(REG_STATUS) & SR_DRQ) != SR_DRQ) {
            udelay(10);
        }

        for (int byte = 0; byte < CF_SECTOR_SIZE; byte += 2) {
            buffer[sector * CF_SECTOR_SIZE + byte + 1] = inb(REG_DATA);
            buffer[sector * CF_SECTOR_SIZE + byte] = inb(REG_DATA);
        }
    }
}

void cf_write_blocks(uint32_t addr, unsigned int count, char *buffer) {
    // TODO: check addr is within the range.
    if (!count || count > 256 || !buffer) {
        return;
    }

    // 0 is a special value for 256.
    outb(REG_SECTOR_COUNT, count < 256 ? count : 0);
    outb(REG_LBA_LOW, addr);
    outb(REG_LBA_MID, addr >> 8);
    outb(REG_LBA_HIGH, addr >> 16);
    outb(REG_CARD_HEAD, CHR_CARD0 | CHR_LBA | ((addr >> 24) & CHR_HEAD_MASK));
    outb(REG_CMD, CMD_WRITE_SECTORS);

    for (unsigned int sector = 0; sector < count; sector++) {
        while (inb(REG_STATUS) & SR_BSY) {
            udelay(10);
        }
        while ((inb(REG_STATUS) & SR_DRQ) != SR_DRQ) {
            udelay(10);
        }

        for (int byte = 0; byte < CF_SECTOR_SIZE; byte += 2) {
            outb(REG_DATA, buffer[sector * CF_SECTOR_SIZE + byte + 1]);
            outb(REG_DATA, buffer[sector * CF_SECTOR_SIZE + byte]);
        }
    }
}