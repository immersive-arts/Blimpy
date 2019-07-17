/**
 * Initialize battery reading.
 */
void bat_init();

/**
 * Read battery level as voltage.
 *
 * @return The battery voltage.
 */
float bat_read_voltage();

/**
 * Read battery level as a factor.
 *
 * @return The battery level.
 */
float bat_read_factor();
