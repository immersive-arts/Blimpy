#include <art32/numbers.h>
#include <driver/ledc.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <naos.h>
#include <naos/sys.h>

#include "srv.h"

#define SRV_1 GPIO_NUM_15
#define SRV_2 GPIO_NUM_5
#define SRV_3 GPIO_NUM_26
#define SRV_4 GPIO_NUM_25
#define SRV_5 GPIO_NUM_33
#define SRV_6 GPIO_NUM_32

static bool srv_avl[SRV_NUM] = {0};

static ledc_channel_t srv_chans[SRV_NUM] = {
    LEDC_CHANNEL_0, LEDC_CHANNEL_1, LEDC_CHANNEL_2, LEDC_CHANNEL_3, LEDC_CHANNEL_4, LEDC_CHANNEL_5,
};

typedef struct {
  bool on;
  double min;
  double max;
  double step;
  double pos;
  bool rev;
} srv_state_t;

static srv_state_t srv_motions[SRV_NUM] = {0};

static void srv_write(uint8_t num, double pos) {
  // calculate duty cycle range
  double min_us = a32_map_d(750, 0, 20000, 0, 1024);
  double max_us = a32_map_d(2250, 0, 20000, 0, 1024);

  // calculate duty
  uint32_t duty = a32_safe_map_d(pos, 0, 1, min_us, max_us);

  // configure pwm
  ESP_ERROR_CHECK(ledc_set_duty(LEDC_HIGH_SPEED_MODE, srv_chans[num], duty));
  ESP_ERROR_CHECK(ledc_update_duty(LEDC_HIGH_SPEED_MODE, srv_chans[num]));
}

static void srv_task(void *p) {
  // loop forever
  for (;;) {
    // check all motions
    for (int i = 0; i < SRV_NUM; i++) {
      // get motion
      srv_state_t *state = srv_motions + i;

      // check state
      if (!state->on) {
        continue;
      }

      // get target
      double target = state->pos + state->step;
      if (state->rev) {
        target = state->pos - state->step;
      }

      // check overshoots
      if (target < state->min) {
        target = state->min;
        state->rev = false;
      } else if (target > state->max) {
        target = state->max;
        state->rev = true;
      }

      // set position
      srv_write(i, target);

      // store position
      state->pos = target;
    }

    // delay
    naos_delay(20);  // 50Hz
  }
}

void srv_init(bool s12, bool s34, bool s56) {
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

  if (s12) {
    // configure servo 1
    c.gpio_num = SRV_1;
    c.channel = srv_chans[0];
    ESP_ERROR_CHECK(ledc_channel_config(&c));
    srv_avl[0] = true;

    // configure servo 2
    c.gpio_num = SRV_2;
    c.channel = srv_chans[1];
    ESP_ERROR_CHECK(ledc_channel_config(&c));
    srv_avl[1] = true;
  }

  if (s34) {
    // configure servo 3
    c.gpio_num = SRV_3;
    c.channel = srv_chans[2];
    ESP_ERROR_CHECK(ledc_channel_config(&c));
    srv_avl[2] = true;

    // configure servo 4
    c.gpio_num = SRV_4;
    c.channel = srv_chans[3];
    ESP_ERROR_CHECK(ledc_channel_config(&c));
    srv_avl[3] = true;
  }

  if (s56) {
    // configure servo 5
    c.gpio_num = SRV_5;
    c.channel = srv_chans[4];
    ESP_ERROR_CHECK(ledc_channel_config(&c));
    srv_avl[4] = true;

    // configure servo 6
    c.gpio_num = SRV_6;
    c.channel = srv_chans[5];
    ESP_ERROR_CHECK(ledc_channel_config(&c));
    srv_avl[5] = true;
  }

  // start task
  xTaskCreatePinnedToCore(&srv_task, "srv", 8192, NULL, 2, NULL, 1);
}

void srv_set(uint8_t num, double pos) {
  // check number
  if (num >= SRV_NUM || !srv_avl[num]) {
    return;
  }

  // disable motion and update state
  srv_motions[num].on = false;
  srv_motions[num].rev = pos < srv_motions[num].pos;
  srv_motions[num].pos = pos;

  // write position
  srv_write(num, pos);
}

void srv_motion(uint8_t num, double min, double max, double step) {
  // check number
  if (num >= SRV_NUM || !srv_avl[num]) {
    return;
  }

  // enable motion
  if (step > 0) {
    srv_motions[num].on = true;
    srv_motions[num].min = min;
    srv_motions[num].max = max;
    srv_motions[num].step = step;
  } else {
    srv_motions[num].on = false;
  }
}
