/**
 * Ported from https://github.com/hideakitai/MAX17048/blob/master/MAX17048.h.
 */

#include <driver/i2c.h>
#include <naos.h>

#include "bat.h"

#define BAT_ADDR 0x36

static void bat_write(uint8_t reg, const uint8_t *buf, uint8_t len, bool ack) {
  // prepare command
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();

  // generate start sequence
  ESP_ERROR_CHECK(i2c_master_start(cmd));

  // write address
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (BAT_ADDR << 1) | I2C_MASTER_WRITE, true));

  // write register
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, reg, true));

  // write data
  for (int i = 0; i < len; i++) {
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, buf[i], ack));
  }

  // generate stop sequence
  ESP_ERROR_CHECK(i2c_master_stop(cmd));

  // issue config command
  ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 2000 / portTICK_RATE_MS));

  // cleanup command
  i2c_cmd_link_delete(cmd);
}

static void bat_read(uint8_t reg, uint8_t *buf, uint8_t len) {
  // prepare command
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();

  // generate start sequence
  ESP_ERROR_CHECK(i2c_master_start(cmd));

  // write address
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (BAT_ADDR << 1) | I2C_MASTER_WRITE, true));

  // write register
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, reg, true));

  // generate start sequence
  ESP_ERROR_CHECK(i2c_master_start(cmd));

  // write address
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (BAT_ADDR << 1) | I2C_MASTER_READ, true));

  // read data
  for (int i = 0; i < len; i++) {
    ESP_ERROR_CHECK(i2c_master_read_byte(cmd, (buf + i), i == len - 1));
  }

  // generate stop sequence
  ESP_ERROR_CHECK(i2c_master_stop(cmd));

  // issue config command
  ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 2000 / portTICK_RATE_MS));

  // cleanup command
  i2c_cmd_link_delete(cmd);
}

static void bat_write16(uint8_t reg, uint16_t value, bool ack) {
  // get bytes MSB first
  uint8_t bytes[] = {(uint8_t)(value >> 8u) & 0xFFu, (uint8_t)value & 0xFFu};

  // write bytes
  bat_write(reg, bytes, 2, ack);
}

static uint16_t bat_read16(uint8_t reg) {
  // read bytes
  uint8_t bytes[2];
  bat_read(reg, bytes, 2);

  // convert to value MSB first
  uint16_t value = (uint16_t)(bytes[0]) << 8u;
  value |= (uint16_t)(bytes[1]);

  return value;
}

void bat_init() {
  // write reset to CMD register
  bat_write16(0xFE, 0x5400, false);

  // write quick start to MODE register
  bat_write16(0x06, 0x4000, true);
}

float bat_read_factor() {
  // read value from SOC register
  uint16_t value = bat_read16(0x04);

  // convert go factor
  return (float)value / 25600.0f;
}
