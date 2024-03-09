// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "update.h"

#include <stdint.h>
#include <stdio.h>

#include "board.h"
#include "clock.h"
#include "cpu.h"
#include "fmem.h"
#include "third_party/async_xmodem/xmodem_server.h"

// Xmodem server instance.
static struct xmodem_server server;
// Reception buffer.
static uint8_t payload[XMODEM_MAX_PACKET_SIZE];
// Destination address for the update file. The address is stored on 32 bits int
// to ease the manipulation of the segment:address couple.
static uint32_t dst = 0x80000;

static inline void dst_split(uint32_t dst, uint16_t *seg, uint8_t **addr) {
    *seg = (uint16_t)((dst & 0xf0000) >> 4);
    *addr = (uint8_t *)(uint16_t)(dst & 0xffff);
}

void tx_byte(struct xmodem_server *xdm, uint8_t byte, void *cb_data) {
    (void)xdm;
    (void)cb_data;
    putchar((int)byte);
}

void update(void) {
    uint32_t update_size = 0;

    printf("Receiving software update: starting xmodem server\n");
    xmodem_server_init(&server, tx_byte, NULL);

    while (!xmodem_server_is_done(&server)) {
        int c;
        int rx_data_len;
        uint32_t block_num;

        c = getchar();
        if (c >= 0) {
            xmodem_server_rx_byte(&server, (uint8_t)(c & 0xff));
        }
        rx_data_len =
            xmodem_server_process(&server, payload, &block_num, clock_now());
        if (rx_data_len > 0) {
            uint16_t seg;
            uint8_t *addr;
            // Extract the segment and destination address from the 32 bits
            // address.
            dst_split(dst, &seg, &addr);
            // Copy the xmodem packet payload to the destination.
            fmemcpy(fmem_void_fptr(seg, addr),
                    fmem_void_fptr(KERNEL_DS, payload), rx_data_len);
            // Increment the destination address.
            dst += rx_data_len;
            update_size += rx_data_len;
        }
    }
    if (xmodem_server_get_state(&server) == XMODEM_STATE_FAILURE) {
        printf("Xmodem transfer failed.\n");
        return;
    }

    // TODO: write to the flash using the block devices interface.

    printf("\ndone.\n");
    printf("Rebooting now.\n");
    reboot();
}
