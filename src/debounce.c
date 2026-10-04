#include "debounce.h"

void debounce_init(debounce_t *d, uint8_t threshold, bool initial) {
    d->count = 0; d->threshold = threshold ? threshold : 1; d->state = initial;
}
bool debounce_update(debounce_t *d, bool raw) {
    if (raw == d->state) { d->count = 0; return d->state; }
    if (++d->count >= d->threshold) { d->state = raw; d->count = 0; }
    return d->state;
}
