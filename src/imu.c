/**
 * Ported from https://github.com/adafruit/Adafruit_LSM9DS1.
 */

#include <driver/i2c.h>

#include "imu.h"

#define IMU_ADDR_AG 0x6B
#define IMU_ADDR_M 0x1E

static imu_vector_t imu_acc = {0};
static imu_vector_t imu_gyr = {0};
static imu_vector_t imu_mag = {0};

static void imu_read_buf(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buffer) {
  // prepare command
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();

  // generate start sequence
  ESP_ERROR_CHECK(i2c_master_start(cmd));

  // write address
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true));

  // set multi read flag
  if (len > 1) {
    reg |= 0x80;
  }

  // write register
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, reg, true));

  // generate start sequence
  ESP_ERROR_CHECK(i2c_master_start(cmd));

  // write address
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, true));

  // read data
  ESP_ERROR_CHECK(i2c_master_read(cmd, buffer, len, true));

  // generate stop sequence
  ESP_ERROR_CHECK(i2c_master_stop(cmd));

  // issue config command
  ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 2000 / portTICK_RATE_MS));

  // cleanup command
  i2c_cmd_link_delete(cmd);
}

static uint8_t imu_read(uint8_t addr, uint8_t reg) {
  // read one byte
  uint8_t value;
  imu_read_buf(addr, reg, 1, &value);

  return value;
}

static void imu_write(uint8_t addr, uint8_t reg, uint8_t value) {
  // prepare command
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();

  // generate start sequence
  ESP_ERROR_CHECK(i2c_master_start(cmd));

  // write address
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true));

  // write register
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, reg, true));

  // write register data
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, value, true));

  // generate stop sequence
  ESP_ERROR_CHECK(i2c_master_stop(cmd));

  // issue config command
  ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 2000 / portTICK_RATE_MS));

  // cleanup command
  i2c_cmd_link_delete(cmd);
}

imu_vector_t imu_read_accelerometer() {
  // read data
  uint8_t buffer[6];
  for (uint8_t i = 0; i < 6; i++) {
    buffer[i] = imu_read(IMU_ADDR_AG, (uint8_t)(0x28 + i));
  }

  // get bytes
  uint8_t xlo = buffer[0];
  int16_t xhi = buffer[1];
  uint8_t ylo = buffer[2];
  int16_t yhi = buffer[3];
  uint8_t zlo = buffer[4];
  int16_t zhi = buffer[5];

  // shift bytes
  int16_t x = (xhi << 8) | xlo;
  int16_t y = (yhi << 8) | ylo;
  int16_t z = (zhi << 8) | zlo;

  // convert and set data
  imu_acc.x = x * 0.061 / 1000 * 9.80665;
  imu_acc.y = y * 0.061 / 1000 * 9.80665;
  imu_acc.z = z * 0.061 / 1000 * 9.80665;

  return imu_acc;
}

imu_vector_t imu_read_gyroscope() {
  // read data
  uint8_t buffer[6];
  for (uint8_t i = 0; i < 6; i++) {
    buffer[i] = imu_read(IMU_ADDR_AG, (uint8_t)(0x18 + i));
  }

  // get bytes
  uint8_t xlo = buffer[0];
  int16_t xhi = buffer[1];
  uint8_t ylo = buffer[2];
  int16_t yhi = buffer[3];
  uint8_t zlo = buffer[4];
  int16_t zhi = buffer[5];

  // shift bytes
  int16_t x = (xhi << 8) | xlo;
  int16_t y = (yhi << 8) | ylo;
  int16_t z = (zhi << 8) | zlo;

  // convert and set data
  imu_gyr.x = x * 0.00875;
  imu_gyr.y = y * 0.00875;
  imu_gyr.z = z * 0.00875;

  return imu_gyr;
}

imu_vector_t imu_read_magnetometer() {
  // read data
  uint8_t buffer[6];
  for (uint8_t i = 0; i < 6; i++) {
    buffer[i] = imu_read(IMU_ADDR_M, (uint8_t)(0x28 + i));
  }

  // get bytes
  uint8_t xlo = buffer[0];
  int16_t xhi = buffer[1];
  uint8_t ylo = buffer[2];
  int16_t yhi = buffer[3];
  uint8_t zlo = buffer[4];
  int16_t zhi = buffer[5];

  // shift bytes
  int16_t x = (xhi << 8) | xlo;
  int16_t y = (yhi << 8) | ylo;
  int16_t z = (zhi << 8) | zlo;

  // convert and set data
  imu_mag.x = x * 0.14 / 1000;
  imu_mag.y = y * 0.14 / 1000;
  imu_mag.z = z * 0.14 / 1000;

  return imu_mag;
}

void imu_init() {
  // check accelerometer and gyroscope id
  uint8_t id = imu_read(IMU_ADDR_AG, 0x0F);
  if (id != 0b01101000) {
    ESP_ERROR_CHECK(ESP_FAIL);
  };

  // check magnetometer id
  id = imu_read(IMU_ADDR_M, 0x0F);
  if (id != 0b00111101) {
    ESP_ERROR_CHECK(ESP_FAIL);
  };

  // soft reset accelerometer and gyroscope
  imu_write(IMU_ADDR_AG, 0x22, 0b00000101);

  // soft reset magnetometer
  imu_write(IMU_ADDR_M, 0x21, 0b00001100);

  // enable gyroscope (952hz, 245dps)
  imu_write(IMU_ADDR_AG, 0x10, 0b11000000);

  // enable accelerometer (952hz, ±2g)
  imu_write(IMU_ADDR_AG, 0x1F, 0b00111000);
  imu_write(IMU_ADDR_AG, 0x20, 0b11000000);

  // enable magnetometer (±4 gauss)
  imu_write(IMU_ADDR_M, 0x22, 0b00000000);
}
