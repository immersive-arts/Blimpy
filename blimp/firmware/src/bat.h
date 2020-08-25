typedef struct {
  float charge;
  float voltage;
  float avg_voltage;
  float current;
  float avg_current;
} bat_data_t;

/**
 * Initialize battery reading.
 */
void bat_init();

/**
 * Read battery level as a factor.
 *
 * @return The battery level.
 */
float bat_read_factor();

bat_data_t bat_data();
