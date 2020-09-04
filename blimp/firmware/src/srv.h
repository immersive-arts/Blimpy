#include <stdbool.h>
#include <stdint.h>

#define SRV_NUM 6

void srv_init(bool s34, bool s56);

void srv_set(uint8_t num, double pos);

void srv_motion(uint8_t num, double min, double max, double step);
