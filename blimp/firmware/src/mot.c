#include <driver/mcpwm.h>

#define MOT_0A 32
#define MOT_0B 33
#define MOT_1A 25
#define MOT_1B 26
#define MOT_2A 27
#define MOT_2B 13
#define MOT_3A 4
#define MOT_3B 16
#define MOT_4A 17
#define MOT_4B 18
#define MOT_5A 19
#define MOT_5B 23

static mcpwm_unit_t mot_units[] = {MCPWM_UNIT_0, MCPWM_UNIT_0, MCPWM_UNIT_0, MCPWM_UNIT_1, MCPWM_UNIT_1, MCPWM_UNIT_1};

static mcpwm_timer_t mot_timers[] = {MCPWM_TIMER_0, MCPWM_TIMER_1, MCPWM_TIMER_2,
                                     MCPWM_TIMER_0, MCPWM_TIMER_1, MCPWM_TIMER_2};

void mot_init() {
  // configure pins
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, MOT_0A);
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, MOT_0B);
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1A, MOT_1A);
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1B, MOT_1B);
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM2A, MOT_2A);
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM2B, MOT_2B);
  mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM0A, MOT_3A);
  mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM0B, MOT_3B);
  mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM1A, MOT_4A);
  mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM1B, MOT_4B);
  mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM2A, MOT_5A);
  mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM2B, MOT_5B);

  // prepare config
  mcpwm_config_t cfg = {
      .frequency = 20000,  // 500Hz
      .cmpr_a = 0,
      .cmpr_b = 0,
      .counter_mode = MCPWM_UP_COUNTER,
      .duty_mode = MCPWM_DUTY_MODE_0,
  };

  // configure units
  mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &cfg);
  mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &cfg);
  mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_2, &cfg);
  mcpwm_init(MCPWM_UNIT_1, MCPWM_TIMER_0, &cfg);
  mcpwm_init(MCPWM_UNIT_1, MCPWM_TIMER_1, &cfg);
  mcpwm_init(MCPWM_UNIT_1, MCPWM_TIMER_2, &cfg);
}

void mot_set(int num, float speed) {
  // get unit and timer
  mcpwm_unit_t unit = mot_units[num];
  mcpwm_timer_t timer = mot_timers[num];

  // set signals
  if (speed == 0) {
    mcpwm_set_signal_low(unit, timer, MCPWM_OPR_A);
    mcpwm_set_signal_low(unit, timer, MCPWM_OPR_B);
  } else if (speed < 0) {
    mcpwm_set_signal_low(unit, timer, MCPWM_OPR_B);
    mcpwm_set_duty(unit, timer, MCPWM_OPR_A, speed * 100);
    mcpwm_set_duty_type(unit, timer, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
  } else {
    mcpwm_set_signal_low(unit, timer, MCPWM_OPR_A);
    mcpwm_set_duty(unit, timer, MCPWM_OPR_B, speed * -100);
    mcpwm_set_duty_type(unit, timer, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);
  }
}
