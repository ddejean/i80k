// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "blkdev.h"
#include "cf20_defs.h"
#include "cpu.h"

bool cf20_set_feature(const struct cf20_private *pdev, uint8_t feature,
                      uint8_t config) {
    outb(pdev->regs.features, feature);
    outb(pdev->regs.sector_count, config);
    outb(pdev->regs.card_head, CHR_CARD0);
    outb(pdev->regs.cmd, CMD_SET_FEATURE);

    // Wait for the device to be ready.
    while (inb(pdev->regs.status) & SR_BSY)
        ;

    uint8_t byte = inb(pdev->regs.status);
    if (byte & SR_ERR) {
        byte = inb(pdev->regs.error);
        if (byte & ERR_ABORT) {
            return false;
        }
    }
    return true;
}

bool cf20_identify(const struct cf20_private *pdev, struct cf20_identity *id) {
    uint16_t *buf;

    buf = malloc(pdev->sector_sz / sizeof(*buf));
    if (!buf) {
        return false;
    }

    // Send the CF Identity commande.
    outb(pdev->regs.card_head, CHR_CARD0);
    outb(pdev->regs.cmd, CMD_IDENTIFY);

    while (inb(pdev->regs.status) & SR_BSY)
        ;

    // Pull the whole sector in a buffer.
    uint8_t *in_buf = (uint8_t *)buf;
    for (size_t i = 0; i < pdev->sector_sz; i++) {
        in_buf[i] = inb(pdev->regs.data);
    }

    // Pull and parse the result of the IDENTIFY command according to Compact
    // Flash specification 2.0 - §6.2.1.6.
    if (buf[0] != CF20_SIGNATURE) {
        // The signature is not correct, we can't expect the end of the buffer
        // to be valid.
        free(buf);
        return false;
    }
    // Default number of cylinders
    id->default_cyl_count = buf[1];
    // Default number of heads.
    id->default_heads_count = buf[3];
    // Default number of sectors per track.
    id->default_sec_per_track = buf[6];
    // Number of sectors per card
    id->sec_per_card = ((uint32_t)(buf[7]) << 16) | (uint32_t)buf[8];
    // Serial number on 20 bytes.
    for (int i = 0; i < 20; i += 2) {
        id->serial[i] = (char)buf[10 + (i / 2)];
        id->serial[i + 1] = (char)(buf[10 + (i / 2)] >> 8);
    }
    id->serial[20] = '\0';
    // Remove trailing spaces.
    for (int i = 19; i >= 0; i--) {
        if (id->serial[i] && id->serial[i] != ' ') {
            break;
        }
        id->serial[i] = '\0';
    }
    // Number of ECC bytes passed on read/write long commands.
    id->ecc_bytes_count = buf[22];
    // Firmware revision in ASCII.
    for (int i = 0; i < 8; i += 2) {
        id->firmware[i + 1] = (char)buf[23 + (i / 2)];
        id->firmware[i] = (char)(buf[23 + (i / 2)] >> 8);
    }
    id->firmware[8] = '\0';
    // Remove trailing spaces.
    for (int i = 7; i >= 0; i--) {
        if (id->firmware[i] && id->firmware[i] != ' ') {
            break;
        }
        id->firmware[i] = '\0';
    }
    // Model number in ASCII.
    for (int i = 0; i < 40; i += 2) {
        id->model[i + 1] = (char)buf[27 + (i / 2)];
        id->model[i] = (char)(buf[27 + (i / 2)] >> 8);
    }
    id->model[40] = '\0';
    // Remove trailing spaces.
    for (int i = 39; i >= 0; i--) {
        if (id->model[i] && id->model[i] != ' ') {
            break;
        }
        id->model[i] = '\0';
    }
    // Maximum number of sector on read/write multiple command.
    id->max_sec_count = buf[47];
    // Capabilities.
    id->capabilities = buf[49];
    // PIO data transfer cycle timing mode.
    id->pio_timing_mode = buf[51];
    // Field validity.
    id->field_validity = buf[53];
    // Current number of cylinders.
    id->curr_cyl_count = buf[54];
    // Current number of heads.
    id->curr_heads_count = buf[55];
    // Current sectors per track.
    id->curr_sec_per_track = buf[56];
    // Current capacity in sectors.
    id->cur_capacity = (uint32_t)buf[57] | ((uint32_t)(buf[58]) << 16);
    // Multiple sector setting.
    id->multiple_sector_setting = buf[59];
    // Total number of sectors addressable in LBA mode.
    id->total_sec_count = (uint32_t)buf[60] | ((uint32_t)(buf[61]) << 16);
    // Advanced PIO modes supported.
    id->advanced_pio_modes = buf[64];
    // Minimum PIO transfer cycle time without flow control.
    id->min_pio_xfer_cycle_wo_fc = buf[67];
    // Minimum PIO transfer cycle time with IORDY flow control.
    id->min_pio_xfer_cycle_with_fc = buf[68];
    // Features/command sets supported.
    id->feat_cmd_supported[0] = buf[82];
    id->feat_cmd_supported[1] = buf[83];
    id->feat_cmd_supported[2] = buf[84];
    // Features/command sets enabled.
    id->feat_cmd_enabled[0] = buf[85];
    id->feat_cmd_enabled[1] = buf[86];
    id->feat_cmd_enabled[2] = buf[87];
    // Bytes 176-177 are reserved.
    // Time required for Security erase unit completion.
    id->security_erase_time = buf[89];
    // Time required for Enhanced Security erase unit completion.
    id->enhanced_security_erase_time = buf[90];
    // Current Advanced power management value.
    id->adv_pm_value = buf[91];
    // Security status.
    id->security_status = buf[128];
    // Power requirement description.
    id->power_req = buf[160];
    // Key management schemes supported.
    id->key_mgmt_schemes = buf[162];

    free(buf);
    return true;
}

void cf20_send_read_sectors(const struct cf20_private *pdev, block_t block,
                            size_t count) {
    // 0 is a special value for 256.
    outb(pdev->regs.sector_count, count < 256 ? count : 0);
    outb(pdev->regs.lba_low, block);
    outb(pdev->regs.lba_mid, block >> 8);
    outb(pdev->regs.lba_high, block >> 16);
    outb(pdev->regs.card_head,
         CHR_CARD0 | CHR_LBA | ((block >> 24) & CHR_HEAD_MASK));
    outb(pdev->regs.cmd, CMD_READ_SECTORS);
}

void cf20_send_write_sectors(const struct cf20_private *pdev, block_t block,
                             size_t count) {
    // 0 is a special value for 256.
    outb(pdev->regs.sector_count, count < 256 ? count : 0);
    outb(pdev->regs.lba_low, block);
    outb(pdev->regs.lba_mid, block >> 8);
    outb(pdev->regs.lba_high, block >> 16);
    outb(pdev->regs.card_head,
         CHR_CARD0 | CHR_LBA | ((block >> 24) & CHR_HEAD_MASK));
    outb(pdev->regs.cmd, CMD_WRITE_SECTORS);
}

void cf20_read_sector(const struct cf20_private *pdev, uint8_t *buf) {
    for (size_t byte = 0; byte < pdev->sector_sz; byte++) {
        *buf = inb(pdev->regs.data);
        buf++;
    }
}

void cf20_write_sector(const struct cf20_private *pdev, uint8_t *buf) {
    for (size_t byte = 0; byte < pdev->sector_sz; byte++) {
        outb(pdev->regs.data, *buf);
        buf++;
    }
}