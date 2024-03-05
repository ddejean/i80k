// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include <stdbool.h>
#include <stdint.h>

#ifndef _CF20_DEFS_H_
#define _CF20_DEFS_H_

// Register
#define REG_DATA(p) (p)
#define REG_ERROR(p) (p + 1)
#define REG_FEATURES(p) (p + 1)
#define REG_SECTOR_COUNT(p) (p + 2)
#define REG_LBA_LOW(p) (p + 3)
#define REG_LBA_MID(p) (p + 4)
#define REG_LBA_HIGH(p) (p + 5)
#define REG_CARD_HEAD(p) (p + 6)
#define REG_CMD(p) (p + 7)
#define REG_STATUS(p) (p + 7)

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

// Commands.
#define CMD_READ_SECTORS 0x20
#define CMD_WRITE_SECTORS 0x30
#define CMD_IDENTIFY 0xec
#define CMD_SET_FEATURE 0xef

// Features.
#define FEAT_8BIT_ENABLE 0x01
#define FEAT_WRITE_CACHE_ENABLE 0x02
#define FEAT_SET_XFER_MODE 0x03
#define FEAT_8BIT_DISABLE 0x81

// Capabilities.
#define CAP_STDBY_TIMER (1 << 13)
#define CAP_IORDY (1 << 11)
#define CAP_LBA (1 << 9)
#define CAP_DMA (1 << 8)

// Driver private data.
struct cf20_private {
    // Device registers.
    struct {
        uint16_t data;
        uint16_t error;
        uint16_t features;
        uint16_t sector_count;
        uint16_t lba_low;
        uint16_t lba_mid;
        uint16_t lba_high;
        uint16_t card_head;
        uint16_t cmd;
        uint16_t status;
    } regs;
};

// Expected signature in the identity structure.
#define CF20_SIGNATURE ((uint16_t)0x848A)

// struct cf_identity represents the set of fields available from the CF2.0
// IDENTIFY command.
struct cf20_identity {
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

// cf20_set_feature sends a feature and its configuration value to the card.
bool cf20_set_feature(const struct cf20_private *pdev, uint8_t feature,
                      uint8_t config);

// cf20_get_identity retrieves the card identity, parses the result in |id|.
bool cf20_identify(const struct cf20_private *pdev, struct cf20_identity *id);

#endif  // _CF20_DEFS_H_
