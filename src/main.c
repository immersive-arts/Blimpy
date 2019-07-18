#include <driver/adc.h>
#include <driver/i2c.h>
#include <naos.h>
#include <string.h>
#include <art32/strconv.h>
#include <art32/numbers.h>

#include "bat.h"
#include "led.h"
#include "exp.h"

static naos_status_t last_status = NAOS_DISCONNECTED;

static void status(naos_status_t status) {
  // set last status
  last_status = status;

  // set led accordingly
  switch (status) {
    case NAOS_DISCONNECTED:
      led_set(1024, 0, 0);
      break;
    case NAOS_CONNECTED:
      led_set(0, 0, 1024);
      break;
    case NAOS_NETWORKED:
      led_set(0, 1024, 0);
      break;
  }
}

static void ping() {
  // blink white once
  led_set(1024, 1024, 1024);
  naos_delay(300);

  // set status led
  status(last_status);
}

static void online() {
  naos_subscribe("#", 0, NAOS_LOCAL);
}

static void message(const char *topic, uint8_t *payload, size_t len, naos_scope_t scope) {
  // set motor 1
  if(scope == NAOS_LOCAL && strcmp(topic, "m1") == 0) {
    int speed = a32_constrain_i(a32_str2i((const char*)payload), -255, 255);
    bool fwd = true;
    if (speed < 0) {
      speed *= -1;
      fwd = false;
    }

    exp_motor(1, fwd, speed);

    return;
  }

  // set motor 2
  if(scope == NAOS_LOCAL && strcmp(topic, "m2") == 0) {
    int speed = a32_constrain_i(a32_str2i((const char*)payload), -255, 255);
    bool fwd = true;
    if (speed < 0) {
      speed *= -1;
      fwd = false;
    }

    exp_motor(2, fwd, speed);

    return;
  }

  // set motor 3
  if(scope == NAOS_LOCAL && strcmp(topic, "m3") == 0) {
    int speed = a32_constrain_i(a32_str2i((const char*)payload), -255, 255);
    bool fwd = true;
    if (speed < 0) {
      speed *= -1;
      fwd = false;
    }

    exp_motor(3, fwd, speed);

    return;
  }

  // set motor 4
  if(scope == NAOS_LOCAL && strcmp(topic, "m4") == 0) {
    int speed = a32_constrain_i(a32_str2i((const char*)payload), -255, 255);
    bool fwd = true;
    if (speed < 0) {
      speed *= -1;
      fwd = false;
    }

    exp_motor(4, fwd, speed);

    return;
  }
}

static float battery() { return bat_read_factor(); }

static naos_config_t config = {.device_type = "blimpy",
                               .firmware_version = "0.1.0",
                               .ping_callback = ping,
                               .online_callback = online,
                               .message_callback = message,
                               .battery_level = battery,
                               .status_callback = status};

void app_main() {
  // prepare i2c config
  i2c_config_t conf;
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = GPIO_NUM_21;
  conf.sda_pullup_en = GPIO_PULLUP_DISABLE;
  conf.scl_io_num = GPIO_NUM_22;
  conf.scl_pullup_en = GPIO_PULLUP_DISABLE;
  conf.master.clk_speed = 100000;

  // configure i2c port
  ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));

  // install i2c driver
  ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0));

  // define adc width
  ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_12Bit));

  // initialize components
  bat_init();
  led_init();
  exp_init();

  // set led
  status(NAOS_DISCONNECTED);

  // initialize naos
  naos_init(&config);
}
