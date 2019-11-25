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
#include "srv.h"

static naos_status_t last_status = NAOS_DISCONNECTED;

static int32_t min_duty = 0;

static double motor_map[6] = {0};

static void status(naos_status_t status) {
  // set last status
  last_status = status;

  // set led accordingly
  switch (status) {
    case NAOS_NETWORKED:
      led_set(0, 1024, 0);
      break;
    case NAOS_CONNECTED:
      led_set(0, 0, 1024);
      break;
    case NAOS_DISCONNECTED:
    default:
      led_set(1024, 0, 0);
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
  // set motors duty cycles "m1,m2,m3,m4,m5,m6,m7,m8" (-255 to 255)
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

      // convert to duty cycle
      int duty = (int)a32_constrain_d(factor * 255, -255, 255);

      // apply duty threshold
      if (duty > -min_duty && duty < min_duty) {
        duty = 0;
      }

      // get direction
      bool fwd = true;
      if (duty < 0) {
        duty *= -1;
        fwd = false;
      }

      // set motor
      exp_motor(i + 1, fwd, duty);
    }

    // free vectors
    a32_vector_free(result);
    a32_vector_free(factors);

    return;
  }

  // set motors duty cycles "s1,s2,s3,s4" (0 to 1)
  if (scope == NAOS_LOCAL && strcmp(topic, "servos") == 0) {
    // prepare positions
    double positions[4] = {0};

    // parse comma separated speeds
    char *ptr = strtok((char *)payload, ",");
    int i = 0;
    while (ptr != NULL && i < 8) {
      positions[i] = a32_constrain_d(a32_str2d(ptr), 0, 1);
      ptr = strtok(NULL, ",");
      i++;
    }

    // set servos
    for (i = 0; i < 4; i++) {
      srv_set(i + 1, positions[i]);
    }

    return;
  }
}

static void loop() {
  // check fault
  uint8_t fault = exp_fault();
  if (fault != 0) {
    naos_log("fault: %x", fault);
  }
}

static float battery() {
  // read battery
  return bat_read_factor();
}

static naos_param_t params[] = {
    {.name = "min-duty", .type = NAOS_LONG, .default_l = 0, .sync_l = &min_duty},
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
};

static naos_config_t config = {.device_type = "blimpy",
                               .firmware_version = "0.2.0",
                               .parameters = params,
                               .num_parameters = 13,
                               .ping_callback = ping,
                               .online_callback = online,
                               .update_callback = update,
                               .message_callback = message,
                               .loop_callback = loop,
                               .loop_interval = 10,
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
  srv_init();

  // set led
  status(NAOS_DISCONNECTED);

  // initialize naos
  naos_init(&config);

  // prepare model
  mod_calculate();
}
