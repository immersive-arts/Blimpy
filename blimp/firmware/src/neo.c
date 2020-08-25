#include <driver/rmt.h>

#include "neo.h"

static rmt_item32_t* neo_items = NULL;

static neo_pixel_t* neo_pixels = NULL;

static bool neo_white = false;

static int neo_count = 0;

void neo_init(int num_pixel, bool has_white) {
  // allocate items
  if (has_white) {
    neo_items = calloc(sizeof(rmt_item32_t), num_pixel * 32 + 1);
  } else {
    neo_items = calloc(sizeof(rmt_item32_t), num_pixel * 24 + 1);
  }

  // allocate pixels
  neo_pixels = calloc(sizeof(neo_pixel_t), num_pixel);

  // set flag
  neo_white = has_white;
  neo_count = num_pixel;

  // prepare config
  rmt_config_t config;
  config.rmt_mode = RMT_MODE_TX;
  config.channel = RMT_CHANNEL_0;
  config.gpio_num = GPIO_NUM_14;
  config.mem_block_num = 1;
  config.clk_div = 8;
  config.tx_config.loop_en = 0;
  config.tx_config.carrier_en = 0;
  config.tx_config.idle_output_en = 1;
  config.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;
  config.tx_config.carrier_freq_hz = 100;
  config.tx_config.carrier_level = RMT_CARRIER_LEVEL_LOW;
  config.tx_config.carrier_duty_percent = 50;

  // configure rmt controller
  ESP_ERROR_CHECK(rmt_config(&config));

  // install rmt driver
  ESP_ERROR_CHECK(rmt_driver_install(RMT_CHANNEL_0, 0, 0));

  // clear all pixels
  neo_set_all(0, 0, 0, 0);
}

void neo_set_one(int i, uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  neo_pixels[i].r = r;
  neo_pixels[i].g = g;
  neo_pixels[i].b = b;
  neo_pixels[i].w = w;
}

void neo_set_all(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  for (int i = 0; i < neo_count; i++) {
    neo_set_one(i, r, g, b, w);
  }
}

void neo_show() {
  // prepare iterator
  rmt_item32_t* item = neo_items;

  for (int i = 0; i < neo_count; i++) {
    // get pixel value
    uint32_t pixel = (neo_pixels[i].g << 24) | (neo_pixels[i].r << 16) | (neo_pixels[i].b << 8) | neo_pixels[i].w;
    if (!neo_white) {
      pixel >>= 8u;
    }

    // update rmt items
    for (int j = (neo_white ? 31 : 23); j >= 0; j--) {
      if (pixel & (1 << j)) {
        // write high
        item->level0 = 1;
        item->duration0 = 9;  // 900ns (900ns +/- 150ns per datasheet)
        item->level1 = 0;
        item->duration1 = 3;  // 300ns (350ns +/- 150ns per datasheet)
      } else {
        // write low
        item->level0 = 1;
        item->duration0 = 3;  // 300ns (350ns +/- 150ns per datasheet)
        item->level1 = 0;
        item->duration1 = 9;  // 900ns (900ns +/- 150ns per datasheet)
      }

      // advance iterator
      item++;
    }
  }

  // write terminator
  item->level0 = 0;
  item->duration0 = 0;
  item->level1 = 0;
  item->duration1 = 0;

  // show the pixels
  if (neo_white) {
    ESP_ERROR_CHECK(rmt_write_items(RMT_CHANNEL_0, neo_items, neo_count * 32, true));
  } else {
    ESP_ERROR_CHECK(rmt_write_items(RMT_CHANNEL_0, neo_items, neo_count * 24, true));
  }
}
