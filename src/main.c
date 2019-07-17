#include <driver/adc.h>
#include <driver/i2c.h>
#include <naos.h>

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

uint8_t pos = 0;
bool add = true;
bool fwd = true;

static void loop() {
  // move
  pos = add ? pos + 1 : pos - 1;

  // set dir
  if (add && pos >= 255) {
    add = false;
  } else if (!add && pos <= 0) {
    add = true;
    fwd = !fwd;
  }

  // print
  naos_log("pos: %d, dir: %d, fwd: %d", pos, add, fwd);

  // set servos
  for (int i=1; i<=4; i++) {
    exp_servo(i, pos);
  }

  // set motors
  for (int i=1; i<=6; i++) {
    exp_motor(i, fwd, pos);
  }
}

static naos_param_t params[] = {};

static float battery() { return bat_read_factor(); }

static naos_config_t config = {.device_type = "blimpy",
                               .firmware_version = "0.1.0",
                               .parameters = params,
                               .num_parameters = 0,
                               .ping_callback = ping,
                               .loop_callback = loop,
                               .loop_interval = 10,
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
