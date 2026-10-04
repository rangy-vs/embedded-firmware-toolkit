#include <stdio.h>
#include <string.h>
#include "ring_buffer.h"
#include "crc.h"
#include "debounce.h"
#include "cmd_parser.h"

static int failures = 0, checks = 0;
#define CHECK(c) do { checks++; if (!(c)) { failures++; printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #c); } } while (0)

static void test_ring_buffer(void) {
    uint8_t store[8]; ring_buffer_t rb; uint8_t v;
    CHECK(!rb_init(&rb, store, 6));              /* not a power of two */
    CHECK(rb_init(&rb, store, 8));
    CHECK(!rb_pop(&rb, &v));                     /* empty */
    for (uint8_t i = 0; i < 8; i++) CHECK(rb_push(&rb, i));   /* uses ALL 8 slots */
    CHECK(rb_count(&rb) == 8 && rb_free(&rb) == 0);
    CHECK(!rb_push(&rb, 99));                    /* full */
    for (uint8_t i = 0; i < 8; i++) { CHECK(rb_pop(&rb, &v)); CHECK(v == i); }
    for (int round = 0; round < 50; round++) {   /* wrap-around stress */
        for (uint8_t i = 0; i < 5; i++) CHECK(rb_push(&rb, (uint8_t)(round + i)));
        for (uint8_t i = 0; i < 5; i++) { CHECK(rb_pop(&rb, &v)); CHECK(v == (uint8_t)(round + i)); }
    }
    CHECK(rb_count(&rb) == 0);
    rb_push(&rb, 1); rb_clear(&rb); CHECK(rb_count(&rb) == 0);
}

static void test_crc(void) {
    const uint8_t s[] = "123456789";
    CHECK(crc16_ccitt(s, 9, 0xFFFF) == 0x29B1);  /* standard CRC-16/CCITT-FALSE check value */
    CHECK(crc8(s, 9, 0x00) == 0xF4);             /* standard CRC-8 check value */
    CHECK(crc16_ccitt(s, 0, 0xFFFF) == 0xFFFF);  /* empty input leaves init untouched */
    uint16_t part = crc16_ccitt(s, 4, 0xFFFF);   /* incremental == one-shot */
    CHECK(crc16_ccitt(s + 4, 5, part) == 0x29B1);
}

static void test_debounce(void) {
    debounce_t d; debounce_init(&d, 3, false);
    CHECK(!debounce_update(&d, true));  CHECK(!debounce_update(&d, true));
    CHECK(debounce_update(&d, true));            /* 3rd consecutive high flips output */
    CHECK(debounce_update(&d, false));           /* bounce glitch ignored... */
    CHECK(debounce_update(&d, true));            /* ...and the counter resets */
    CHECK(debounce_update(&d, false)); CHECK(debounce_update(&d, false));
    CHECK(!debounce_update(&d, false));          /* 3 consecutive lows flip back */
}

static cmd_event_t feed_all(cmd_parser_t *p, const uint8_t *b, size_t n) {
    cmd_event_t last = CMD_NONE;
    for (size_t i = 0; i < n; i++) { cmd_event_t e = cmd_feed(p, b[i]); if (e != CMD_NONE) last = e; }
    return last;
}

static void test_parser(void) {
    cmd_parser_t p; cmd_init(&p);
    uint8_t frame[CMD_MAX_PAYLOAD + 4]; const uint8_t pl[] = {0x01, 0xAA, 0x55, 0xAA};  /* payload may contain 0xAA */
    size_t n = cmd_encode(pl, sizeof pl, frame);
    CHECK(n == 8);
    CHECK(feed_all(&p, frame, n) == CMD_FRAME_OK);
    CHECK(p.len == 4 && memcmp(p.payload, pl, 4) == 0);

    frame[3] ^= 0x10;                                          /* corrupt payload -> CRC error */
    CHECK(feed_all(&p, frame, n) == CMD_BAD_CRC);

    const uint8_t garbage[] = {0x00, 0x13, 0xFF, 0x7E};        /* noise before a good frame */
    feed_all(&p, garbage, sizeof garbage);
    frame[3] ^= 0x10;                                          /* un-corrupt */
    CHECK(feed_all(&p, frame, n) == CMD_FRAME_OK);

    const uint8_t badlen[] = {CMD_SOF, 0xFF};                  /* length > max rejected */
    CHECK(feed_all(&p, badlen, 2) == CMD_BAD_LEN);
    CHECK(feed_all(&p, frame, n) == CMD_FRAME_OK);             /* recovers on next frame */

    uint8_t empty[4];                                          /* zero-length payload is legal */
    CHECK(cmd_encode(NULL, 0, empty) == 4);
    CHECK(feed_all(&p, empty, 4) == CMD_FRAME_OK);
    CHECK(cmd_encode(pl, CMD_MAX_PAYLOAD + 1, frame) == 0);
    CHECK(p.frames_ok == 4 && p.frames_bad == 2);
}

int main(void) {
    test_ring_buffer(); test_crc(); test_debounce(); test_parser();
    printf("%d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
