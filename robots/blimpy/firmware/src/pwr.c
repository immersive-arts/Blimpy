#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>
#include <naos.h>

#include "pwr.h"

#define PWR_BIT (1 << 0)

#define PWR_TIME 1000

static EventGroupHandle_t pwr_group;

static pwr_callback_t pwr_callback;

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

    // get time
    uint32_t now = naos_millis();

    // call callback
    naos_acquire();
    pwr_callback(true);
    naos_release();

    // prepare flag
    bool shutdown = false;

    // check button
    for (;;) {
      // wait a bit
      naos_delay(100);

      // set flag if held for some time
      if (!shutdown && naos_millis() - now > PWR_TIME) {
        // set flag
        shutdown = true;

        // call callback
        naos_acquire();
        pwr_callback(false);
        naos_release();
      }

      // clear if released
      if (gpio_get_level(GPIO_NUM_34) == 1) {
        break;
      }
    }

    // call callback
    if (!shutdown) {
      naos_acquire();
      pwr_callback(false);
      naos_release();
    }

    // shutdown if flag is set
    if (shutdown) {
      naos_log("pwr: shutdown");
      ESP_ERROR_CHECK(gpio_set_level(GPIO_NUM_12, 0));
      naos_delay(1000);
      ESP_ERROR_CHECK(gpio_set_level(GPIO_NUM_12, 1));
    }

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
      .intr_type = GPIO_PIN_INTR_NEGEDGE,
  };

  // configure button pin
  ESP_ERROR_CHECK(gpio_config(&btn));

  // start task
  xTaskCreatePinnedToCore(&pwr_task, "pwr", 8192, NULL, 2, NULL, 1);

  // attach isr handler for button pin
  ESP_ERROR_CHECK(gpio_isr_handler_add(GPIO_NUM_34, pwr_handler, NULL));
}

void pwr_off() {
  // power off
  ESP_ERROR_CHECK(gpio_set_level(GPIO_NUM_12, 0));
  naos_delay(1000);
  ESP_ERROR_CHECK(gpio_set_level(GPIO_NUM_12, 1));
}
