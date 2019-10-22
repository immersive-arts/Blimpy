/**
 * Ported from https://github.com/sparkfun/SparkFun_SX1509_Arduino_Library.
 */

#include <driver/i2c.h>
#include <naos.h>

#include "exp.h"

#define EXP_ADDR 0x3E

typedef enum {
    EXP_INPUT,
    EXP_INPUT_PULL_UP,
    EXP_INPUT_PULL_DOWN,
    EXP_OUTPUT,
    EXP_OUTPUT_PWM,
} exp_pin_mode_t;

static void exp_write_data(uint8_t reg, const uint8_t *buf, uint8_t len) {
  // prepare command
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();

  // generate start sequence
  ESP_ERROR_CHECK(i2c_master_start(cmd));

  // write address
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (EXP_ADDR << 1) | I2C_MASTER_WRITE, true));

  // write register
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, reg, true));

  // write data
  for (int i = 0; i < len; i++) {
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, buf[i], true));
  }

  // generate stop sequence
  ESP_ERROR_CHECK(i2c_master_stop(cmd));

  // issue config command
  ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_1, cmd, 2000 / portTICK_RATE_MS));

  // cleanup command
  i2c_cmd_link_delete(cmd);
}

static void exp_read_data(uint8_t reg, uint8_t *buf, uint8_t len) {
  // prepare command
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();

  // generate start sequence
  ESP_ERROR_CHECK(i2c_master_start(cmd));

  // write address
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (EXP_ADDR << 1) | I2C_MASTER_WRITE, true));

  // write register
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, reg, true));

  // generate start sequence
  ESP_ERROR_CHECK(i2c_master_start(cmd));

  // write address
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (EXP_ADDR << 1) | I2C_MASTER_READ, true));

  // read data
  for (int i = 0; i < len; i++) {
    ESP_ERROR_CHECK(i2c_master_read_byte(cmd, (buf + i), i == len - 1));
  }

  // generate stop sequence
  ESP_ERROR_CHECK(i2c_master_stop(cmd));

  // issue config command
  ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_1, cmd, 2000 / portTICK_RATE_MS));

  // cleanup command
  i2c_cmd_link_delete(cmd);
}

static void exp_write_8bit(uint8_t reg, uint8_t value) {
  // write single byte
  exp_write_data(reg, &value, 1);
}

static void exp_write_16bit(uint8_t reg, uint16_t value) {
  // get bytes MSB first
  uint8_t bytes[] = {(uint8_t)(value >> 8) & (uint8_t)0xFF, (uint8_t)value & (uint8_t)0xFF};

  // write bytes
  exp_write_data(reg, bytes, 2);
}

static uint8_t exp_read_8bit(uint8_t reg) {
  // read data
  uint8_t value;
  exp_read_data(reg, &value, 1);

  return value;
}

static uint16_t exp_read_16bit(uint8_t reg) {
  // read bytes
  uint8_t bytes[2];
  exp_read_data(reg, bytes, 2);

  // convert to value MSB first
  uint16_t value = (uint16_t)(bytes[0]) << 8;
  value |= (uint16_t)(bytes[1]);

  return value;
}

static void exp_pin_mode(uint8_t pin, exp_pin_mode_t mode) {
  // get direction
  uint in = (mode == EXP_INPUT || mode == EXP_INPUT_PULL_UP || mode == EXP_INPUT_PULL_DOWN);

  // get direction register
  uint16_t reg = exp_read_16bit(0x0E);

  // set pin direction
  if (in) {
    reg |= (1 << pin);
  } else {
    reg &= ~(1 << pin);
  }

  // write register
  exp_write_16bit(0x0E, reg);

  // set pull up if requested
  if (mode == EXP_INPUT_PULL_UP) {
    // get pull up and down registers
    uint16_t pull_up = exp_read_16bit(0x06);
    uint16_t pull_down = exp_read_16bit(0x08);

    // set pin
    pull_up |= (1<<pin);
    pull_down &= ~(1<<pin);

    // write registers
    exp_write_16bit(0x06, pull_up);
    exp_write_16bit(0x08, pull_down);
  }

  // set pull down if requested
  if (mode == EXP_INPUT_PULL_DOWN) {
    // get pull up and down registers
    uint16_t pull_up = exp_read_16bit(0x06);
    uint16_t pull_down = exp_read_16bit(0x08);

    // set pin
    pull_down |= (1<<pin);
    pull_up &= ~(1<<pin);

    // write registers
    exp_write_16bit(0x06, pull_up);
    exp_write_16bit(0x08, pull_down);
  }

  // enable pwm if requested
  if (mode == EXP_OUTPUT_PWM) {
    // disable input buffer
    reg = exp_read_16bit(0x00);
    reg |= (1<<pin);
    exp_write_16bit(0x00, reg);

    // disable pull-up
    reg = exp_read_16bit(0x06);
    reg &= ~(1<<pin);
    exp_write_16bit(0x06, reg);

    // enable LED driver
    reg = exp_read_16bit(0x20);
    reg |= (1<<pin);
    exp_write_16bit(0x20, reg);

    // start LED driver
    reg = exp_read_16bit(0x10);
    reg &= ~(1<<pin);
    exp_write_16bit(0x10, reg);
  }
}

static bool exp_pin_get(uint8_t pin) {
  // read data register
  uint16_t data = exp_read_16bit(0x10);

  // return whether pin is high
  return data & (1<<pin);
}

void exp_pin_set(uint8_t pin, bool level) {
  // get data
  uint16_t data = exp_read_16bit(0x10);

  if (level) {
    data |= (1<<pin);
  } else {
    data &= ~(1<<pin);
  }

  // write data
  exp_write_16bit(0x10, data);
}

uint8_t exp_reg_pwm[16] = {0x2A, 0x2D, 0x30, 0x33,
                           0x36, 0x3B, 0x40, 0x45,
                           0x4A, 0x4D, 0x50, 0x53,
                           0x56, 0x5B, 0x60, 0x65};

static void exp_pin_pwm(uint8_t pin, uint8_t intensity) {
  // write intensity
  exp_write_8bit(exp_reg_pwm[pin], intensity);
}

void exp_init() {
  // prepare i2c config
  i2c_config_t conf;
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = GPIO_NUM_14;
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf.scl_io_num = GPIO_NUM_27;
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.master.clk_speed = 400000; // 400 Khz

  // configure i2c port
  ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_1, &conf));

  // install i2c driver
  ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_1, conf.mode, 0, 0, 0));

  // prepare gpio config
  gpio_config_t gpio = {
      .pin_bit_mask = GPIO_SEL_26,
      .mode = GPIO_MODE_OUTPUT,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
  };

  // configure gpio
  gpio_config(&gpio);

  // reset chip
  gpio_set_level(GPIO_NUM_26, 1);
  naos_delay(1);
  gpio_set_level(GPIO_NUM_26, 0);
  naos_delay(1);
  gpio_set_level(GPIO_NUM_26, 1);

  // read test register
  uint16_t bytes = exp_read_16bit(0x13);
  if (bytes != 0xFF00) {
    ESP_ERROR_CHECK(ESP_FAIL);
  }

  // set clock to 2Mhz
  exp_write_8bit(0x1E, 0x40);

  // enable pwm output
  exp_write_8bit(0x1F, 0x70);

  // configure servos
  exp_pin_mode(0, EXP_OUTPUT_PWM);
  exp_pin_mode(1, EXP_OUTPUT_PWM);
  exp_pin_mode(2, EXP_OUTPUT_PWM);
  exp_pin_mode(3, EXP_OUTPUT_PWM);

  // configure motors
  exp_pin_mode(4, EXP_OUTPUT);
  exp_pin_mode(5, EXP_OUTPUT_PWM);
  exp_pin_mode(6, EXP_OUTPUT);
  exp_pin_mode(7, EXP_OUTPUT_PWM);
  exp_pin_mode(8, EXP_OUTPUT);
  exp_pin_mode(9, EXP_OUTPUT_PWM);
  exp_pin_mode(10, EXP_OUTPUT);
  exp_pin_mode(11, EXP_OUTPUT_PWM);
  exp_pin_mode(12, EXP_OUTPUT);
  exp_pin_mode(13, EXP_OUTPUT_PWM);
  exp_pin_mode(14, EXP_OUTPUT);
  exp_pin_mode(15, EXP_OUTPUT_PWM);
}

uint8_t exp_motor_dir[6] = {8, 10, 12, 14, 4, 6};
uint8_t exp_motor_pwm[6] = {9, 11, 13, 15, 5, 7};

void exp_motor(uint8_t num, bool fwd, uint8_t duty) {
  // set pwm
  exp_pin_pwm(exp_motor_pwm[num-1], 255-duty);

  // set dir
  exp_pin_set(exp_motor_dir[num-1], fwd);
}

void exp_servo(uint8_t num, uint8_t duty) {
  // set pwm
  exp_pin_pwm(num-1, duty);
}
