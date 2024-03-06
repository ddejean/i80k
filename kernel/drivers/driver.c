// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include "driver.h"

#include <stdio.h>

// Start of the driver's declarations array, filled by the linker.
extern const struct driver _drivers_start[];
// End of the driver's declarations array, filled by the linker.
extern const struct driver _drivers_end[];

void driver_probes(void) {
    // Interate over the drivers structures and call the probe function of each
    // driver.
    for (const struct driver *drv = _drivers_start; drv != _drivers_end;
         drv++) {
        if (!drv->probe()) {
            printf("Drivers: driver %s probe failed\n", drv->name);
        }
    }
}
