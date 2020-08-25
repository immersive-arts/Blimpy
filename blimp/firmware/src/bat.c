/**
 * Ported from https://github.com/hideakitai/MAX17048/blob/master/MAX17048.h.
 */

#include <driver/i2c.h>
#include <naos.h>

#include "bat.h"

#define BAT_ADDR_LOW 0x36u
#define BAT_ADDR_HIGH 0x0Bu

static void bat_write(uint8_t addr, uint8_t reg, const uint8_t *buf, uint8_t len, bool ack) {
  // prepare command
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();

  // generate start sequence
  ESP_ERROR_CHECK(i2c_master_start(cmd));

  // write address
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (addr << 1u) | I2C_MASTER_WRITE, true));

  // write register
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, reg, true));

  // write data
  for (int i = 0; i < len; i++) {
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, buf[i], ack));
  }

  // generate stop sequence
  ESP_ERROR_CHECK(i2c_master_stop(cmd));

  // issue command
  ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 2000 / portTICK_RATE_MS));

  // cleanup command
  i2c_cmd_link_delete(cmd);
}

static void bat_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len) {
  // prepare command
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();

  // generate start sequence
  ESP_ERROR_CHECK(i2c_master_start(cmd));

  // write address
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (addr << 1u) | I2C_MASTER_WRITE, true));

  // write register
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, reg, true));

  // generate start sequence
  ESP_ERROR_CHECK(i2c_master_start(cmd));

  // write address
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (addr << 1u) | I2C_MASTER_READ, true));

  // read data
  for (int i = 0; i < len; i++) {
    ESP_ERROR_CHECK(i2c_master_read_byte(cmd, (buf + i), i == len - 1));
  }

  // generate stop sequence
  ESP_ERROR_CHECK(i2c_master_stop(cmd));

  // issue command
  ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 2000 / portTICK_RATE_MS));

  // cleanup command
  i2c_cmd_link_delete(cmd);
}

static void bat_write16(uint8_t addr, uint8_t reg, uint16_t value, bool ack) {
  // get bytes LSB first
  uint8_t bytes[] = {(uint8_t)value & 0xFFu, (uint8_t)(value >> 8u) & 0xFFu};

  // write bytes
  bat_write(addr, reg, bytes, 2, ack);
}

static uint16_t bat_read16(uint8_t addr, uint8_t reg) {
  // read bytes
  uint8_t bytes[2];
  bat_read(addr, reg, bytes, 2);

  // convert to value LSB first
  uint16_t value = (uint16_t)bytes[0] | (uint16_t)bytes[1] << 8u;

  return value;
}

static int16_t bat_read16i(uint8_t addr, uint8_t reg) { return (int16_t)bat_read16(addr, reg); }

void bat_init() {
  // configure
  // bat_write16(BAT_ADDR_HIGH, 0xB5, 0xC01, true); // nConfig
  bat_write16(BAT_ADDR_HIGH, 0xA5, 0x44C, true);  // nFullCapNom
  bat_write16(BAT_ADDR_HIGH, 0xA9, 0x44C, true);  // nFullCapRep
  // bat_write16(BAT_ADDR_HIGH, 0xCF, 0x01F4, true); // nRSens
  bat_write16(BAT_ADDR_HIGH, 0xB3, 0x44C, true);  // nDesignCap
  bat_write16(BAT_ADDR_LOW, 0xBB, 0x0001, true);  // fuel gauge reset
}

float bat_charge() {
  // read value from SOC register
  uint16_t value = bat_read16(BAT_ADDR_LOW, 0x06);

  // convert to factor
  return (float)value / 25600.0f;
}

bat_data_t bat_data() {
  // gather data
  float charge = (float)bat_read16(BAT_ADDR_LOW, 0x06) / 256.0f;
  float voltage = (float)bat_read16(BAT_ADDR_LOW, 0x09) * 0.078125f;
  float avg_voltage = (float)bat_read16(BAT_ADDR_LOW, 0x19) * 0.078125f;
  float current = (float)bat_read16i(BAT_ADDR_LOW, 0x0A) * 0.0015625f / 0.005f;
  float avg_current = (float)bat_read16i(BAT_ADDR_LOW, 0x0B) * 0.0015625f / 0.005f;

  // get data
  bat_data_t data = {
      .charge = charge / 100.f,
      .voltage = voltage/ 1000.f,
      .avg_voltage = avg_voltage/ 1000.f,
      .current = current/ 1000.f,
      .avg_current = avg_current/ 1000.f,
  };

  return data;
}
