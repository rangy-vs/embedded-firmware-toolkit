#ifndef DEBOUNCE_H
#define DEBOUNCE_H
#include <stdbool.h>
#include <stdint.h>
/* Integrator debouncer: call debounce_update() at a fixed rate (e.g. 1 kHz) with the raw pin
 * level. Output flips only after the input has disagreed with it for `threshold` consecutive ticks. */
typedef struct { uint8_t count; uint8_t threshold; bool state; } debounce_t;
void debounce_init(debounce_t *d, uint8_t threshold, bool initial);
bool debounce_update(debounce_t *d, bool raw);   /* returns the debounced state */
#endif
