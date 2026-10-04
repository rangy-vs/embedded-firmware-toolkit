#ifndef RING_BUFFER_H
#define RING_BUFFER_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Fixed-capacity byte FIFO. Single-producer / single-consumer safe when head is only
 * written by the consumer and tail only by the producer (e.g. UART ISR -> main loop),
 * provided capacity is a power of two and index reads/writes are atomic on the target. */
typedef struct {
    uint8_t *buf;
    size_t   mask;            /* capacity - 1 (capacity must be a power of two) */
    volatile size_t head;     /* read index  */
    volatile size_t tail;     /* write index */
} ring_buffer_t;

bool   rb_init(ring_buffer_t *rb, uint8_t *storage, size_t capacity_pow2);
bool   rb_push(ring_buffer_t *rb, uint8_t byte);   /* false if full  */
bool   rb_pop(ring_buffer_t *rb, uint8_t *byte);   /* false if empty */
size_t rb_count(const ring_buffer_t *rb);
size_t rb_free(const ring_buffer_t *rb);
void   rb_clear(ring_buffer_t *rb);
#endif
