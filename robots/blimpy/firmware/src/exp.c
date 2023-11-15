/**
 * Ported from https://github.com/sparkfun/SparkFun_SX1509_Arduino_Library.
 */

#include <driver/i2c.h>
#include <naos.h>

#include "exp.h"

#define EXP_ADDR 0x3Eu

typedef enum {
  EXP_INPUT,
  EXP_INPUT_PULL_UP,
  EXP_INPUT_PULL_DOWN,
  EXP_OUTPUT,
  EXP_OUTPUT_PWM,
} exp_pin_mode_t;

static void exp_write(uint8_t addr, uint8_t reg, const uint8_t *buf, uint8_t len) {
  // prepare command
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();

  // generate start sequence
  ESP_ERROR_CHECK(i2c_master_start(cmd));

  // write address
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, ((EXP_ADDR + addr) << 1u) | I2C_MASTER_WRITE, true));

  // write register
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, reg, true));

  // write data
  for (int i = 0; i < len; i++) {
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, buf[i], true));
  }

  // generate stop sequence
  ESP_ERROR_CHECK(i2c_master_stop(cmd));

  // issue command
  ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 2000 / portTICK_RATE_MS));

  // cleanup command
  i2c_cmd_link_delete(cmd);
}

static void exp_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len) {
  // prepare command
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();

  // generate start sequence
  ESP_ERROR_CHECK(i2c_master_start(cmd));

  // write address
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, ((EXP_ADDR + addr) << 1u) | I2C_MASTER_WRITE, true));

  // write register
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, reg, true));

  // generate start sequence
  ESP_ERROR_CHECK(i2c_master_start(cmd));

  // write address
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, ((EXP_ADDR + addr) << 1u) | I2C_MASTER_READ, true));

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

static void exp_write8(uint8_t addr, uint8_t reg, uint8_t value) {
  // write single byte
  exp_write(addr, reg, &value, 1);
}

static void exp_write16(uint8_t addr, uint8_t reg, uint16_t value) {
  // get bytes MSB first
  uint8_t bytes[] = {(uint8_t)(value >> 8u) & 0xFFu, value & 0xFFu};

  // write bytes
  exp_write(addr, reg, bytes, 2);
}

static uint8_t exp_read8(uint8_t addr, uint8_t reg) {
  // read data
  uint8_t value;
  exp_read(addr, reg, &value, 1);

  return value;
}

static uint16_t exp_read16(uint8_t addr, uint8_t reg) {
  // read bytes
  uint8_t bytes[2];
  exp_read(addr, reg, bytes, 2);

  // convert to value MSB first
  uint16_t value = (uint16_t)(bytes[0]) << 8u;
  value |= (uint16_t)(bytes[1]);

  return value;
}

static void exp_pin_mode(uint8_t addr, uint8_t pin, exp_pin_mode_t mode) {
  // get direction
  uint input = (mode == EXP_INPUT || mode == EXP_INPUT_PULL_UP || mode == EXP_INPUT_PULL_DOWN);

  // get direction register
  uint16_t reg = exp_read16(addr, 0x0E);

  // set pin direction
  if (input) {
    reg |= (1u << pin);
  } else {
    reg &= ~(1u << pin);
  }

  // write register
  exp_write16(addr, 0x0E, reg);

  // set pull up if requested
  if (mode == EXP_INPUT_PULL_UP) {
    // get pull up and down registers
    uint16_t pull_up = exp_read16(addr, 0x06);
    uint16_t pull_down = exp_read16(addr, 0x08);

    // set pin
    pull_up |= (1u << pin);
    pull_down &= ~(1u << pin);

    // write registers
    exp_write16(addr, 0x06, pull_up);
    exp_write16(addr, 0x08, pull_down);
  }

  // set pull down if requested
  if (mode == EXP_INPUT_PULL_DOWN) {
    // get pull up and down registers
    uint16_t pull_up = exp_read16(addr, 0x06);
    uint16_t pull_down = exp_read16(addr, 0x08);

    // set pin
    pull_down |= (1u << pin);
    pull_up &= ~(1u << pin);

    // write registers
    exp_write16(addr, 0x06, pull_up);
    exp_write16(addr, 0x08, pull_down);
  }

  // enable pwm if requested
  if (mode == EXP_OUTPUT_PWM) {
    // disable input buffer
    reg = exp_read16(addr, 0x00);
    reg |= (1u << pin);
    exp_write16(addr, 0x00, reg);

    // disable pull-up
    reg = exp_read16(addr, 0x06);
    reg &= ~(1u << pin);
    exp_write16(addr, 0x06, reg);

    // enable LED driver
    reg = exp_read16(addr, 0x20);
    reg |= (1u << pin);
    exp_write16(addr, 0x20, reg);

    // start LED driver
    reg = exp_read16(addr, 0x10);
    reg &= ~(1u << pin);
    exp_write16(addr, 0x10, reg);
  }
}

static bool exp_pin_get(uint8_t addr, uint8_t pin) {
  // read data register
  uint16_t data = exp_read16(addr, 0x10);

  // return whether pin is high
  return data & (1u << pin);
}

static void exp_pin_set(uint8_t addr, uint8_t pin, bool level) {
  // get data
  uint16_t data = exp_read16(addr, 0x10);

  // apply mask
  if (level) {
    data |= (1u << pin);
  } else {
    data &= ~(1u << pin);
  }

  // write data
  exp_write16(addr, 0x10, data);
}

static uint8_t exp_reg_pwm[16] = {0x2A, 0x2D, 0x30, 0x33, 0x36, 0x3B, 0x40, 0x45,
                                  0x4A, 0x4D, 0x50, 0x53, 0x56, 0x5B, 0x60, 0x65};

static void exp_pin_pwm(uint8_t addr, uint8_t pin, uint8_t intensity) {
  // write intensity
  exp_write8(addr, exp_reg_pwm[pin], intensity);
}

void exp_init() {
  // prepare reset config
  gpio_config_t rst = {
      .pin_bit_mask = GPIO_SEL_2,
      .mode = GPIO_MODE_OUTPUT,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
  };

  // configure reset pin
  ESP_ERROR_CHECK(gpio_config(&rst));

  // reset chip
  ESP_ERROR_CHECK(gpio_set_level(GPIO_NUM_2, 0));
  ESP_ERROR_CHECK(gpio_set_level(GPIO_NUM_2, 1));

  // read test register
  uint16_t bytes = exp_read16(0, 0x13);
  if (bytes != 0xFF00) {
    ESP_ERROR_CHECK(ESP_FAIL);
  }

  // set clock to 2Mhz
  exp_write8(0, 0x1E, 0x40);

  // enable pwm output
  exp_write8(0, 0x1F, 0x70);

  // configure led pins
  exp_pin_mode(0, 13, EXP_OUTPUT_PWM);
  exp_pin_mode(0, 14, EXP_OUTPUT_PWM);
  exp_pin_mode(0, 15, EXP_OUTPUT_PWM);
}

void exp_led(uint8_t r, uint8_t g, uint8_t b) {
  // set pins
  exp_pin_pwm(0, 13, r);
  exp_pin_pwm(0, 15, g);
  exp_pin_pwm(0, 14, b);
}
