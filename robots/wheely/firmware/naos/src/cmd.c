#include "cmd.h"

#include <driver/gpio.h>
#include <driver/spi_slave.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define GPIO_MOSI 13
#define GPIO_MISO 12
#define GPIO_SCLK 15
#define GPIO_CS 14

#define RCV_HOST HSPI_HOST

static uint8_t left_right;
static uint8_t rotate;
static uint8_t up_down;
TaskHandle_t task;

unsigned char reverse(unsigned char b) {
  b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
  b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
  b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
  return b;
}

static void cmd_task(void *data) {
  WORD_ALIGNED_ATTR char sendbuf[9] = "";
  WORD_ALIGNED_ATTR char recvbuf[9] = "";

  spi_slave_transaction_t t = {0};

  left_right = 128;
  rotate = 128;
  up_down = 128;

  while (1) {
    // Clear receive buffer, set send buffer to something sane
    for (int i = 0; i < 9; i++) {
      recvbuf[i] = 0x12;
      sendbuf[i] = 0xFF;
    }
    sendbuf[1] = reverse(0x73);
    sendbuf[2] = reverse(0x5A);
    sendbuf[3] = 0xFF;                 // SLCT JOYR JOYL STRT UP   RGHT DOWN LEFT
    sendbuf[4] = 0xFF;                 // L2   R2   L1   R1   /   O    X    |_|
    sendbuf[5] = reverse(rotate);      // Right Joy 0x00 = Left  0xFF = Right
    sendbuf[6] = reverse(128);         // Right Joy 0x00 = Up    0xFF = Down
    sendbuf[7] = reverse(left_right);  // Left Joy  0x00 = Left  0xFF = Right
    sendbuf[8] = reverse(up_down);     // Left Joy  0x00 = Up    0xFF = Down

    // Set up a transaction of 9 bytes to send/receive
    t.length = 9 * 8;
    t.tx_buffer = sendbuf;
    t.rx_buffer = recvbuf;
    /* This call enables the SPI slave interface to send/receive to the sendbuf and recvbuf. The transaction is
    initialized by the SPI master, however, so it will not actually happen until the master starts a hardware
    transaction by pulling CS low and pulsing the clock etc. In this specific example, we use the handshake line, pulled
    up by the .post_setup_cb callback that is called as soon as a transaction is ready, to let the master know it is
    free to transfer data.
    */
    ESP_ERROR_CHECK(spi_slave_transmit(RCV_HOST, &t, portMAX_DELAY));
  }
}
void cmd_init(void) {
  // Configuration for the SPI bus
  spi_bus_config_t buscfg = {
      .mosi_io_num = GPIO_MOSI,
      .miso_io_num = GPIO_MISO,
      .sclk_io_num = GPIO_SCLK,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
  };

  // Configuration for the SPI slave interface
  spi_slave_interface_config_t slvcfg = {
      .mode = 3,
      .spics_io_num = GPIO_CS,
      .queue_size = 3,
      .flags = 0,
  };

  // Enable pull-ups on SPI lines so we don't detect rogue pulses when no master is connected.
  ESP_ERROR_CHECK(gpio_set_pull_mode(GPIO_MOSI, GPIO_PULLUP_ONLY));
  ESP_ERROR_CHECK(gpio_set_pull_mode(GPIO_SCLK, GPIO_PULLUP_ONLY));
  ESP_ERROR_CHECK(gpio_set_pull_mode(GPIO_CS, GPIO_PULLUP_ONLY));

  // Initialize SPI slave interface
  ESP_ERROR_CHECK(spi_slave_initialize(RCV_HOST, &buscfg, &slvcfg, 0));

  // Start task
  xTaskCreatePinnedToCore(cmd_task, "cmd", 2048, NULL, 2, &task, 1);
}

void cmd_update(double fx, double fy, double fz, double mx, double my, double mz) {
  rotate = -(uint8_t)(mz * 127.5f + 127.5f);
  up_down = (uint8_t)(fx * 127.5f + 127.5f);
  left_right = (uint8_t)(fy * 127.5f + 127.5f);
}
