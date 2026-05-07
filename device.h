#ifndef DEVICE_H_
#define DEVICE_H_

#include <stdint.h>
#include <stdio.h>

typedef enum device_type {
    MOUSE = 1 << 0,
    KEYBOARD = 1 << 1
} device_type;

typedef struct device_entry {
    char name[256];
    char devnode[256];
} device_entry;

device_entry *fetch_devices(device_type types, size_t *out_count);

int create_vdevice();

void emit_key(int fd, int type, int code, int val);

#endif // DEVICE_H_
