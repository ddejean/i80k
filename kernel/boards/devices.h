// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _DEVICES_H_
#define _DEVICES_H_

#include <stdbool.h>
#include <stdint.h>

// IO devices categories.
typedef enum _io_device_t {
    IO_DEV_PIC_MASTER = 0,
    IO_DEV_PIC_SLAVE,
    IO_DEV_UART,
    IO_DEV_MAX,
} io_device_t;

struct io_device {
    uint16_t port;
    int irq;
    union {
        struct {
            unsigned long freq;
        } uart;
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
    // Shift of the sector size.
    unsigned int sector_shift;
};

struct cf20 {
    // Base I/O port.
    uint16_t port;
    // IRQ the device is mapped to.
    int irq;
    // Weither the device is mapped on a 8 bits bus.
    bool is_8bit;
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
    const struct device concat(name_, driver_)   \
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

// board_get_by_name returns the device named <name>.
const struct device *board_get_by_name(const char *name);

#endif  // _DEVICES_H_
