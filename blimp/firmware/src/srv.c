#include <art32/numbers.h>
#include <driver/ledc.h>
#include <naos.h>

#include "srv.h"

#define S1_PIN GPIO_NUM_15
#define S2_PIN GPIO_NUM_5

static ledc_channel_t pin_map[] = {
    LEDC_CHANNEL_0,
    LEDC_CHANNEL_1,
};

void srv_init() {
  // prepare ledc timer config
  ledc_timer_config_t t = {
      .timer_num = LEDC_TIMER_1,
      .duty_resolution = LEDC_TIMER_10_BIT,
      .freq_hz = 50,
      .speed_mode = LEDC_HIGH_SPEED_MODE,
  };

  // configure ledc timer
  ESP_ERROR_CHECK(ledc_timer_config(&t));

  // prepare ledc channel config
  ledc_channel_config_t c = {
      .duty = 0,
      .intr_type = LEDC_INTR_DISABLE,
      .speed_mode = LEDC_HIGH_SPEED_MODE,
      .timer_sel = LEDC_TIMER_1,
  };

  // configure servo 1
  c.gpio_num = S1_PIN;
  c.channel = pin_map[0];
  ESP_ERROR_CHECK(ledc_channel_config(&c));

  // configure servo 2
  c.gpio_num = S2_PIN;
  c.channel = pin_map[1];
  ESP_ERROR_CHECK(ledc_channel_config(&c));
}

void srv_set(uint8_t num, double pos) {
  // print configuration
  naos_log("servo: num %d, pos %+.3f", num, pos);

  // calculate duty cycle range
  double min_us = a32_map_d(750, 0, 20000, 0, 1024);
  double max_us = a32_map_d(2250, 0, 20000, 0, 1024);

  // calculate duty
  uint32_t duty = a32_safe_map_d(pos, 0, 1, min_us, max_us);

  // configure pwm
  ESP_ERROR_CHECK(ledc_set_duty(LEDC_HIGH_SPEED_MODE, pin_map[num - 1], duty));
  ESP_ERROR_CHECK(ledc_update_duty(LEDC_HIGH_SPEED_MODE, pin_map[num - 1]));
}
