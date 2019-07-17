/**
 * A three dimensional vector.
 */
typedef struct {
  double x, y, z;
} imu_vector_t;

/**
 * Initialize IMU sensor.
 */
void imu_init();

/**
 * Read accelerometer.
 *
 * @return A vector of m/s2 acceleration.
 */
imu_vector_t imu_read_accelerometer();

/**
 * Read gyroscope.
 *
 * @return A vector in dps angular rate.
 */
imu_vector_t imu_read_gyroscope();

/**
 * Read magnetometer.
 *
 * @return A vector in gauss magnetic.
 */
imu_vector_t imu_read_magnetometer();
