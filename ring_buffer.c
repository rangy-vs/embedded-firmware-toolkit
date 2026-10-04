#include "ring_buffer.h"

bool rb_init(ring_buffer_t *rb, uint8_t *storage, size_t cap) {
    if (!rb || !storage || cap < 2 || (cap & (cap - 1)) != 0) return false;
    rb->buf = storage; rb->mask = cap - 1; rb->head = rb->tail = 0;
    return true;
}
size_t rb_count(const ring_buffer_t *rb) { return (rb->tail - rb->head) & (rb->mask * 2 + 1); }
/* Indices run over 2*capacity so "full" and "empty" are distinguishable without a spare slot. */
size_t rb_free(const ring_buffer_t *rb) { return (rb->mask + 1) - rb_count(rb); }

bool rb_push(ring_buffer_t *rb, uint8_t byte) {
    if (rb_free(rb) == 0) return false;
    rb->buf[rb->tail & rb->mask] = byte;
    rb->tail = (rb->tail + 1) & (rb->mask * 2 + 1);
    return true;
}
bool rb_pop(ring_buffer_t *rb, uint8_t *byte) {
    if (rb_count(rb) == 0) return false;
    *byte = rb->buf[rb->head & rb->mask];
    rb->head = (rb->head + 1) & (rb->mask * 2 + 1);
    return true;
}
void rb_clear(ring_buffer_t *rb) { rb->head = rb->tail = 0; }
