#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>
#include <naos.h>

#include "pwr.h"

#define PWR_BIT (1 << 0)

#define PWR_DELAY 1

#define PWR_TIME 2000

static EventGroupHandle_t pwr_group;

static pwr_callback_t pwr_callback;

static uint32_t pwr_state;

static void pwr_handler(void *args) {
  // send event
  xEventGroupSetBitsFromISR(pwr_group, PWR_BIT, NULL);
}

static void pwr_task(void *p) {
  // loop forever
  for (;;) {
    // wait for bit
    EventBits_t bits = xEventGroupWaitBits(pwr_group, PWR_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    if ((bits & PWR_BIT) != PWR_BIT) {
      continue;
    }

    // read button
    bool pressed = gpio_get_level(GPIO_NUM_34) == 0;

    // check time when released
    if (!pressed) {
      if (naos_millis() - pwr_state > PWR_TIME) {
        ESP_ERROR_CHECK(gpio_set_level(GPIO_NUM_12, 0));
      }
    }

    // set state
    pwr_state = naos_millis();

    // call callback
    naos_acquire();
    pwr_callback(pressed);
    naos_release();

    // delay next reading
    naos_delay(PWR_DELAY);

    // clear bit
    xEventGroupClearBits(pwr_group, PWR_BIT);
  }
}

void pwr_init(pwr_callback_t cb) {
  // set callback
  pwr_callback = cb;

  // create mutex
  pwr_group = xEventGroupCreate();

  // prepare power config
  gpio_config_t pwr = {
      .pin_bit_mask = GPIO_SEL_12,
      .mode = GPIO_MODE_OUTPUT,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
  };

  // configure power pin
  ESP_ERROR_CHECK(gpio_config(&pwr));

  // keep power on
  ESP_ERROR_CHECK(gpio_set_level(GPIO_NUM_12, 1));

  // prepare button config
  gpio_config_t btn = {
      .pin_bit_mask = GPIO_SEL_34,
      .mode = GPIO_MODE_INPUT,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .intr_type = GPIO_PIN_INTR_ANYEDGE,
  };

  // configure button pin
  ESP_ERROR_CHECK(gpio_config(&btn));

  // start task
  xTaskCreatePinnedToCore(&pwr_task, "pwr", 8192, NULL, 2, NULL, 1);

  // attach isr handler for button pin
  ESP_ERROR_CHECK(gpio_isr_handler_add(GPIO_NUM_34, pwr_handler, NULL));
}
