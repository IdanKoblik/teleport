#ifndef DEVICE_H_
#define DEVICE_H_

#include <stdint.h>
#include <stdio.h>

typedef struct device {
    uint8_t vendor_id;
    uint8_t product_id;

    FILE *input_file;
    int (*handle)(struct device *d);
} device;

#endif // DEVICE_H_
