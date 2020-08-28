#include <art32/numbers.h>
#include <art32/strconv.h>
#include <driver/adc.h>
#include <driver/i2c.h>
#include <naos.h>
#include <string.h>

#include "bat.h"
#include "exp.h"
#include "mod.h"
#include "mot.h"
#include "neo.h"
#include "pwr.h"
#include "srv.h"

static naos_status_t last_status = NAOS_DISCONNECTED;

static double safety_off = 0;

static double motor_map[6] = {0};

static int32_t led_size = 0;
static bool led_white = false;

static void status(naos_status_t status) {
  // set last status
  last_status = status;

  // set status led
  switch (status) {
    case NAOS_NETWORKED:
      exp_led(0, 255, 0);
      break;
    case NAOS_CONNECTED:
      exp_led(0, 0, 255);
      break;
    case NAOS_DISCONNECTED:
    default:
      exp_led(255, 0, 0);
      break;
  }
}

static void power(bool pressed) {
  // set led
  if (pressed) {
    exp_led(0, 255, 255);
  } else {
    status(last_status);
  }
}

static void ping() {
  // blink white once
  exp_led(255, 255, 255);
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
  // set motor duty cycles "m1,m2,m3,m4,m5,m6" (-1 to 1)
  if (scope == NAOS_LOCAL && strcmp(topic, "motors") == 0) {
    // prepare speeds
    float speeds[6] = {0};

    // parse comma separated speeds
    char *ptr = strtok((char *)payload, ",");
    int i = 0;
    while (ptr != NULL && i < 6) {
      speeds[i] = a32_constrain_f(a32_str2f(ptr), -1, 1);
      ptr = strtok(NULL, ",");
      i++;
    }

    // set motors
    for (i = 0; i < 6; i++) {
      mot_set(i, speeds[i]);
    }

    return;
  }

  // set model forces and torques "fx,fy,fz,mx,my,mz" (-1 to 1)
  if (scope == NAOS_LOCAL && strcmp(topic, "forces") == 0) {
    // prepare factors
    a32_vector_t factors = a32_vector_new(6);

    // parse comma separated speeds
    char *ptr = strtok((char *)payload, ",");
    int i = 0;
    while (ptr != NULL && i < factors.length) {
      factors.values[i] = a32_constrain_d(a32_str2d(ptr), -1, 1);
      ptr = strtok(NULL, ",");
      i++;
    }

    // calculate model
    a32_vector_t result = mod_calc(factors);

    // set motors
    for (i = 0; i < 6; i++) {
      // get factor
      double factor = result.values[i];

      // apply modifier
      factor = factor * motor_map[i];

      // set motor
      mot_set(i, (float)factor);
    }

    // free vectors
    a32_vector_free(result);
    a32_vector_free(factors);

    return;
  }

  // set servo duty cycles "s1,s2" (0 to 1)
  if (scope == NAOS_LOCAL && strcmp(topic, "servos") == 0) {
    // prepare positions
    double positions[2] = {0};

    // parse comma separated speeds
    char *ptr = strtok((char *)payload, ",");
    int i = 0;
    while (ptr != NULL && i < 2) {
      positions[i] = a32_constrain_d(a32_str2d(ptr), 0, 1);
      ptr = strtok(NULL, ",");
      i++;
    }

    // set servos
    for (i = 0; i < 2; i++) {
      srv_set(i + 1, positions[i]);
    }

    return;
  }

  // set lights "r,g,b,w" (0 to 255)
  if (scope == NAOS_LOCAL && strcmp(topic, "lights") == 0) {
    // prepare colors
    int colors[4] = {0};

    // parse comma separated colors
    char *ptr = strtok((char *)payload, ",");
    int i = 0;
    while (ptr != NULL && i < 4) {
      colors[i] = a32_constrain_i(a32_str2i(ptr), 0, 255);
      ptr = strtok(NULL, ",");
      i++;
    }

    // set lights
    neo_set_all(colors[0], colors[1], colors[2], colors[3]);
    neo_show();

    return;
  }
}

static void loop() {
  // get battery data
  bat_data_t data = bat_data();

  // check battery
  if (data.charge < safety_off) {
    pwr_off();
  }

  // print battery
  naos_log("Battery: %.1f%% | %.3fV (%.3fV) | %.3fA (%.3fA)", data.charge * 100.f, data.voltage, data.avg_voltage,
           data.current, data.avg_current);

  // publish data
  char str[128];
  sprintf(str, "%f,%f,%f,%f,%f", data.charge, data.voltage, data.avg_voltage, data.current, data.avg_current);
  naos_publish("battery", str, 0, false, NAOS_LOCAL);
}

static float battery() {
  // read battery
  return bat_charge();
}

static naos_param_t params[] = {
    {.name = "safety-off", .type = NAOS_DOUBLE, .default_d = 0.3, .sync_d = &safety_off},
    {.name = "motor-map1", .type = NAOS_DOUBLE, .default_d = 1, .sync_d = &motor_map[0]},
    {.name = "motor-map2", .type = NAOS_DOUBLE, .default_d = 1, .sync_d = &motor_map[1]},
    {.name = "motor-map3", .type = NAOS_DOUBLE, .default_d = 1, .sync_d = &motor_map[2]},
    {.name = "motor-map4", .type = NAOS_DOUBLE, .default_d = 1, .sync_d = &motor_map[3]},
    {.name = "motor-map5", .type = NAOS_DOUBLE, .default_d = 1, .sync_d = &motor_map[4]},
    {.name = "motor-map6", .type = NAOS_DOUBLE, .default_d = 1, .sync_d = &motor_map[5]},
    {.name = "model-m1", .type = NAOS_STRING, .default_s = "0,0,0,0,0,0"},
    {.name = "model-m2", .type = NAOS_STRING, .default_s = "0,0,0,0,0,0"},
    {.name = "model-m3", .type = NAOS_STRING, .default_s = "0,0,0,0,0,0"},
    {.name = "model-m4", .type = NAOS_STRING, .default_s = "0,0,0,0,0,0"},
    {.name = "model-m5", .type = NAOS_STRING, .default_s = "0,0,0,0,0,0"},
    {.name = "model-m6", .type = NAOS_STRING, .default_s = "0,0,0,0,0,0"},
    {.name = "led-size", .type = NAOS_LONG, .default_l = 0, .sync_l = &led_size},
    {.name = "led-white", .type = NAOS_BOOL, .default_b = false, .sync_b = &led_white},
};

static naos_config_t config = {.device_type = "blimpy",
                               .firmware_version = "0.3.0",
                               .parameters = params,
                               .num_parameters = 15,
                               .ping_callback = ping,
                               .online_callback = online,
                               .update_callback = update,
                               .message_callback = message,
                               .loop_callback = loop,
                               .loop_interval = 300,
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
  pwr_init(power);
  bat_init();
  exp_init();
  mot_init();
  srv_init();

  // set led
  status(NAOS_DISCONNECTED);

  // initialize naos
  naos_init(&config);

  // initialize neo pixels
  neo_init(led_size, led_white);

  // prepare model
  mod_calculate();
}
