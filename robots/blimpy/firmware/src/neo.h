#ifndef NEO_H
#define NEO_H

#include <stdint.h>

typedef struct {
  uint8_t r, g, b, w;
} neo_pixel_t;

void neo_init(int num_pixel, bool has_white);

void neo_set_one(int i, uint8_t r, uint8_t g, uint8_t b, uint8_t w);

void neo_set_all(uint8_t r, uint8_t g, uint8_t b, uint8_t w);

void neo_show();

#endif  // NEO_H
