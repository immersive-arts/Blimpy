#include <art32/numbers.h>
#include <art32/strconv.h>
#include <driver/adc.h>
#include <driver/i2c.h>
#include <naos.h>
#include <string.h>

#include "bat.h"
#include "exp.h"
#include "led.h"
#include "mod.h"

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
  // subscribe to all local messages
  naos_subscribe("#", 0, NAOS_LOCAL);
}

static void update(const char *param, const char *value) {
  // recalculate model if motor configuration changed
  if (strncmp(param, "model-m", 7) == 0) {
    mod_calculate();
  }
}

static void message(const char *topic, uint8_t *payload, size_t len, naos_scope_t scope) {
  // set motors duty cycles "255,-255,127,64,..."
  if (scope == NAOS_LOCAL && strcmp(topic, "motors") == 0) {
    // prepare speeds
    int speeds[8] = {0};

    // parse comma separated speeds
    char *ptr = strtok((char *)payload, ",");
    int i = 0;
    while (ptr != NULL && i < 8) {
      speeds[i] = a32_constrain_i(a32_str2i(ptr), -255, 255);
      ptr = strtok(NULL, ",");
      i++;
    }

    // set motors
    for (i = 0; i < 8; i++) {
      // get speed
      int speed = speeds[i];

      // get direction
      bool fwd = true;
      if (speed < 0) {
        speed *= -1;
        fwd = false;
      }

      // set motor
      exp_motor(i + 1, fwd, speed);
    }

    return;
  }

  // set model forces and torques "1,-1,0.5,0.25"
  if (scope == NAOS_LOCAL && strcmp(topic, "forces") == 0) {
    // prepare speeds
    double speeds[4] = {0};

    // parse comma separated speeds
    char *ptr = strtok((char *)payload, ",");
    int i = 0;
    while (ptr != NULL && i < 4) {
      speeds[i] = a32_constrain_d(a32_str2d(ptr), -1, 1);
      ptr = strtok(NULL, ",");
      i++;
    }

    // calculate model
    mod_result_t res = mod_calc(speeds[0], speeds[1], speeds[2], speeds[3]);

    // set motors
    for (i = 0; i < 4; i++) {
      // get speed
      int speed;
      switch (i) {
        case 0:
          speed = res.m1;
          break;
        case 1:
          speed = res.m2;
          break;
        case 2:
          speed = res.m3;
          break;
        case 3:
          speed = res.m4;
          break;
        default:
          speed = 0;
      }

      // get direction
      bool fwd = true;
      if (speed < 0) {
        speed *= -1;
        fwd = false;
      }

      // set motor
      exp_motor(i + 1, fwd, speed);
    }

    return;
  }
}

static void loop() {
  // print fault
  // naos_log("fault: %x", exp_fault());
}

static float battery() {
  // read battery
  return bat_read_factor();
}

static naos_param_t params[] = {
    {.name = "model-m1", .type = NAOS_STRING, .default_s = "0,0,0,0,0,0"},
    {.name = "model-m2", .type = NAOS_STRING, .default_s = "0,0,0,0,0,0"},
    {.name = "model-m3", .type = NAOS_STRING, .default_s = "0,0,0,0,0,0"},
    {.name = "model-m4", .type = NAOS_STRING, .default_s = "0,0,0,0,0,0"},
};

static naos_config_t config = {.device_type = "blimpy",
                               .firmware_version = "0.1.1",
                               .parameters = params,
                               .num_parameters = 4,
                               .ping_callback = ping,
                               .online_callback = online,
                               .update_callback = update,
                               .message_callback = message,
                               .loop_callback = loop,
                               .loop_interval = 1,
                               .battery_level = battery,
                               .status_callback = status};

void app_main() {
  // install global interrupt service
  ESP_ERROR_CHECK(gpio_install_isr_service(0));

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

  // prepare model
  mod_calculate();
}
