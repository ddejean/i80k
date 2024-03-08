// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _DEVICES_H_
#define _DEVICES_H_

#include <stdbool.h>
#include <stdint.h>

// IO devices categories.
typedef enum _io_device_t {
    IO_DEV_PIC_MASTER = 0,
    IO_DEV_PIC_SLAVE,
    IO_DEV_PIT_TIMER0,
    IO_DEV_PIT_TIMER1,
    IO_DEV_PIT_TIMER2,
    IO_DEV_UART,
    IO_DEV_CF,
    IO_DEV_MAX,
} io_device_t;

struct io_device {
    uint16_t port;
    int irq;
    union {
        struct {
            int index;
            unsigned long freq;
        } timer;
        struct {
            unsigned long freq;
        } uart;
        struct {
            bool is_8bit;
        } cf;
    } u;
};

struct cfi_flash {
    // Identifiers of the CFI flash chip.
    uint8_t vendor_id, chip_id;
    // CFI device base address.
    uint32_t base_addr;
    // Number of sectors on the device.
    unsigned int sector_count;
    // Size of a sector in bytes.
    unsigned int sector_size;
};

// Device description pulled by a driver while probing.
struct device {
    // Name of the device.
    const char *name;
    // Driver handling the device.
    const char *driver;
    // Driver dependant configuration data.
    const void *config;
};

// macro-expanding concat
#define concat(a, b) __ex_concat(a, b)
#define __ex_concat(a, b) a##b

#define DEVICE(name_, driver_, config_)          \
    const struct device concat(__name_, driver_) \
        __attribute__((section(".devices"))) = { \
            .name = #name_,                      \
            .driver = #driver_,                  \
            .config = &config_,                  \
    }

// board_register_io_dev registers an I/O device in the devices set. Returns 0
// on success, -1 on error.
int board_register_io_dev(io_device_t type, struct io_device *dev);

// board_get_io_dev obtains the I/O device matching <type> if registered, NULL
// otherwise.
struct io_device *board_get_io_dev(io_device_t type);

// board_get_by_driver returns the device matching the driver name <name>.
const struct device *board_get_by_driver(const char *name);

#endif  // _DEVICES_H_
