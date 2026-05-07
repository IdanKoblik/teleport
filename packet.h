#ifndef PACKET_H_
#define PACKET_H_

#include <stdint.h>
#include "device.h"

#define PROTOCOL_VERSION 1

#define PACKET_SIZE sizeof(packet)

/*
 * Packet layout (packed, no padding):
 *
 * Byte offsets:
 *
 *   0        1        5        7        9        13
 *   +--------+--------+--------+--------+--------+
 *   | ver    | type   | event  | code   | value  |
 *   | u8     | enum   | u16    | u16    | i32    |
 *   +--------+--------+--------+--------+--------+
 *
 * Total size: sizeof(packet) = 13 bytes
 */

typedef struct packet {
    uint8_t version;
    uint8_t type;

    // taken from input_event <linux/input.h>
    uint16_t event;
    uint16_t code;
    int32_t value;
} __attribute__((packed)) packet;

void encode_packet(const packet *p, uint8_t *out);

void decode_packet(const uint8_t *data, packet *out);

#endif // PACKET_H_
