#include <stdint.h>

/**
 * Initialize LED controller.
 */
void led_init();

/**
 * Set the LED channels.
 *
 * @param r The red color.
 * @param g The green color.
 * @param b The blue color.
 */
void led_set(uint16_t r, uint16_t g, uint16_t b);
