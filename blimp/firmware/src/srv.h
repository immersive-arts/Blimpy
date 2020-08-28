#include <stdbool.h>
#include <stdint.h>

void srv_init();

void srv_set(uint8_t num, double pos);

void srv_motion(uint8_t num, double min, double max, double step);
