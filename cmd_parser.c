#include "cmd_parser.h"
#include "crc.h"
#include <string.h>

void cmd_init(cmd_parser_t *p) { memset(p, 0, sizeof *p); p->state = ST_SOF; }

static uint16_t frame_crc(const cmd_parser_t *p) {
    uint16_t c = crc16_ccitt(&p->len, 1, 0xFFFF);
    return crc16_ccitt(p->payload, p->len, c);
}

cmd_event_t cmd_feed(cmd_parser_t *p, uint8_t b) {
    switch (p->state) {
    case ST_SOF:
        if (b == CMD_SOF) p->state = ST_LEN;
        break;
    case ST_LEN:
        if (b > CMD_MAX_PAYLOAD) { p->state = (b == CMD_SOF) ? ST_LEN : ST_SOF; p->frames_bad++; return CMD_BAD_LEN; }
        p->len = b; p->idx = 0;
        p->state = b ? ST_PAYLOAD : ST_CRC_HI;
        break;
    case ST_PAYLOAD:
        p->payload[p->idx++] = b;
        if (p->idx == p->len) p->state = ST_CRC_HI;
        break;
    case ST_CRC_HI:
        p->crc_rx = (uint16_t)b << 8; p->state = ST_CRC_LO;
        break;
    case ST_CRC_LO:
        p->crc_rx |= b; p->state = ST_SOF;
        if (p->crc_rx == frame_crc(p)) { p->frames_ok++; return CMD_FRAME_OK; }
        p->frames_bad++;
        return CMD_BAD_CRC;
    }
    return CMD_NONE;
}

size_t cmd_encode(const uint8_t *payload, uint8_t len, uint8_t *out) {
    if (len > CMD_MAX_PAYLOAD) return 0;
    out[0] = CMD_SOF; out[1] = len;
    if (len) memcpy(&out[2], payload, len);
    uint16_t c = crc16_ccitt(&out[1], (size_t)len + 1, 0xFFFF);
    out[2 + len] = (uint8_t)(c >> 8); out[3 + len] = (uint8_t)c;
    return (size_t)len + 4;
}
