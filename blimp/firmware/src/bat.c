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

void bat_write16(uint8_t addr, uint8_t reg, uint16_t value, bool ack) {
  // get bytes MSB first
  uint8_t bytes[] = {(uint8_t)value & 0xFFu, (uint8_t)(value >> 8u) & 0xFFu};

  // write bytes
  bat_write(addr, reg, bytes, 2, ack);
}

uint16_t bat_read16(uint8_t addr, uint8_t reg) {
  // read bytes
  uint8_t bytes[2];
  bat_read(addr, reg, bytes, 2);

  // convert to value MSB first
  uint16_t value = (uint16_t)(bytes[1]) << 8u;
  value |= (uint16_t)(bytes[0]);

  return value;
}

uint8_t bat_read8(uint8_t addr, uint8_t reg) {
  // read data
  uint8_t value;
  bat_read(addr, reg, &value, 1);

  return value;
}

void bat_init() {
  // configure
  // bat_write16(BAT_ADDR_HIGH, 0xB5, 0xC01, true); // nConfig
  bat_write16(BAT_ADDR_HIGH, 0xA5, 0x44C, true);  // nFullCapNom
  bat_write16(BAT_ADDR_HIGH, 0xA9, 0x44C, true);  // nFullCapRep
  // bat_write16(BAT_ADDR_HIGH, 0xCF, 0x01F4, true); // nRSens
  bat_write16(BAT_ADDR_HIGH, 0xB3, 0x44C, true);  // nDesignCap
  bat_write16(BAT_ADDR_LOW, 0xBB, 0x0001, true);  // fuel gauge reset
}

float bat_read_factor() {
  // read value from SOC register
  uint16_t value = bat_read16(BAT_ADDR_LOW, 0x06);

  // convert to factor
  return (float)value / 25600.0f;
}

void bat_data() {
  printf("Battery feedback\n");
  uint16_t word = bat_read16(BAT_ADDR_LOW, 0x21);
  printf("Device ID: %x \n", word);
  word = bat_read16(BAT_ADDR_HIGH, 0xB5);
  printf("Config: %x \n", word);
  word = bat_read16(BAT_ADDR_HIGH, 0xB3);
  printf("nDesignCap: %x \n", word);
  word = bat_read16(BAT_ADDR_HIGH, 0xA5);
  printf("nFullCapNom: %x \n", word);
  word = bat_read16(BAT_ADDR_HIGH, 0xA9);
  printf("nFullCapRep: %x \n", word);
  word = bat_read16(BAT_ADDR_HIGH, 0xCF);
  printf("nRSense: %x \n", word);
  word = bat_read16(BAT_ADDR_LOW, 0x06);
  float RepSOC = word * 1.0f / 256.0f;
  printf("RepSOC: %.3f (%x) \n", word * 1.0f / 256.0f, word);
  word = bat_read16(BAT_ADDR_LOW, 0x09);
  printf("VCell: %.3f (%x) \n", word * 0.078125f, word);
  word = bat_read16(BAT_ADDR_LOW, 0x19);
  printf("Avg VCell: %.3f (%x) \n", word * 0.078125f, word);
  word = bat_read16(BAT_ADDR_LOW, 0x0A);
  printf("Current: %.3f (%x) \n", ((int16_t)word) * 1.5625e-6f / 5000e-6f * 1000.0f, (int16_t)word);
  word = bat_read16(BAT_ADDR_LOW, 0x0B);
  printf("Avg Current: %.3f %.3f (%x) \n", ((int16_t)word) * 1.5625e-6f / 5000e-6f * 1000.0f,
         ((int16_t)word) * 1.5625e-6f / 5000e-6f * 1000.0f, (int16_t)word);

  if (RepSOC < 30.0f) {
    gpio_set_level(GPIO_NUM_12, 0);
  } else {
    gpio_set_level(GPIO_NUM_12, 1);
  }
}
