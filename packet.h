#ifndef PACKET_H_
#define PACKET_H_

#include <stdint.h>

#include "keyboard.h"

#define PROTOCOL_VERSION 1

/*
 * Packet layout
 * =============
 *
 * Byte offsets:
 *
 *   0         1          5                              5 + sizeof(device)
 *   +---------+----------+------------------------------+
 *   | version |   type   |            device            |
 *   |  (u8)   |  (enum)  |   (union, tagged by `type`)  |
 *   +---------+----------+------------------------------+
 *
 *   device when type == KEYBOARD:
 *
 *       0         2         3         4
 *       +---------+---------+---------+
 *       |   key   |  state  |  mods   |
 *       |  (u16)  |  (u8)   |  (u8)   |
 *       +---------+---------+---------+
 *
 *   device when type == MOUSE:  (TODO #6)
 *
 * Field semantics
 *   version : must equal PROTOCOL_VERSION; receiver rejects mismatches
 *   type    : discriminator selecting the active `device` variant
 *   key     : key code
 *   state   : key_state — PRESSED | RELEASED | AUTO_REPEAT
 *   mods    : bitmask of `modifiers` held when the event fired
 */

typedef enum packet_type {
    MOUSE,
    KEYBOARD
} packet_type;

typedef struct keyboard_packet {
    uint16_t key;
    uint8_t state;
    uint8_t mods;
} keyboard_packet;

typedef union device_packet {
    keyboard_packet keyboard;
    /* TODO mouse packet (#6) */
} device_packet;

typedef struct packet {
    uint8_t version;

    packet_type type;
    device_packet device;
} packet;

#endif // PACKET_H_
