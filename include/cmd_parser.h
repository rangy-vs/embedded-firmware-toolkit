#ifndef CMD_PARSER_H
#define CMD_PARSER_H
#include <stddef.h>
#include <stdint.h>

/* Framed binary protocol parser (state machine), fed one byte at a time from a UART ISR/loop.
 *
 *   | 0xAA | LEN | PAYLOAD (LEN bytes, LEN <= CMD_MAX_PAYLOAD) | CRC16-CCITT (hi, lo) |
 *
 * CRC covers LEN + PAYLOAD. Resynchronises on the next 0xAA after any error. */
#define CMD_SOF          0xAA
#define CMD_MAX_PAYLOAD  32

typedef enum { CMD_NONE = 0, CMD_FRAME_OK, CMD_BAD_CRC, CMD_BAD_LEN } cmd_event_t;
typedef enum { ST_SOF, ST_LEN, ST_PAYLOAD, ST_CRC_HI, ST_CRC_LO } cmd_state_t;

typedef struct {
    cmd_state_t state;
    uint8_t  len, idx;
    uint8_t  payload[CMD_MAX_PAYLOAD];
    uint16_t crc_rx;
    uint32_t frames_ok, frames_bad;   /* link-quality counters */
} cmd_parser_t;

void        cmd_init(cmd_parser_t *p);
cmd_event_t cmd_feed(cmd_parser_t *p, uint8_t byte);
/* Build a frame into out (needs len+4 bytes). Returns frame size, or 0 if len too large. */
size_t      cmd_encode(const uint8_t *payload, uint8_t len, uint8_t *out);
#endif
