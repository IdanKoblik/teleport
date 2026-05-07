#include "packet.h"
#include <string.h>

#define PACK_ENCODE(buf, offset, field) \
    do { \
        memcpy((buf) + (offset), &(field), sizeof(field)); \
        (offset) += sizeof(field); \
    } while (0)

#define PACK_DECODE(buf, offset, field) \
    do { \
        memcpy(&(field), (buf) + (offset), sizeof(field)); \
        (offset) += sizeof(field); \
    } while (0)

void encode_packet(const packet *p, uint8_t *out) {
    size_t offset = 0;

    PACK_ENCODE(out, offset, p->version);
    PACK_ENCODE(out, offset, p->type);
    PACK_ENCODE(out, offset, p->event);
    PACK_ENCODE(out, offset, p->code);
    PACK_ENCODE(out, offset, p->value);
}

void decode_packet(const uint8_t *data, packet *out) {
    size_t offset = 0;

    PACK_DECODE(data, offset, out->version);
    PACK_DECODE(data, offset, out->type);
    PACK_DECODE(data, offset, out->event);
    PACK_DECODE(data, offset, out->code);
    PACK_DECODE(data, offset, out->value);
}
