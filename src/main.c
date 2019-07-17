#include <driver/adc.h>
#include <driver/i2c.h>
#include <naos.h>

#include "bat.h"
#include "imu.h"
#include "led.h"
#include "exp.h"

static int32_t interval = 0;
static bool use_acc = false;
static bool use_gyr = false;
static bool use_mag = false;
static bool use_tmp = false;

static naos_status_t last_status = NAOS_DISCONNECTED;

static uint32_t last_interval = 0;

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

uint8_t pos = 0;
bool add = true;
bool fwd = true;

static void loop() {
  // move
  pos = add ? pos + 1 : pos - 1;

  // set dir
  if (add && pos >= 255) {
    add = false;
  } else if (!add && pos <= 0) {
    add = true;
    fwd = !fwd;
  }

  // print
  naos_log("pos: %d, dir: %d, fwd: %d", pos, add, fwd);

  // set servos
  for (int i=1; i<=4; i++) {
    exp_servo(i, pos);
  }

  // set motors
  for (int i=1; i<=6; i++) {
    exp_motor(i, fwd, pos);
  }

  // return if interval is not yet met
  if (last_interval + interval > naos_millis()) {
    return;
  }

  // publish accelerometer if enabled
  if (use_acc) {
    imu_vector_t acc = imu_read_accelerometer();
    char buf[64];
    sprintf(buf, "%f,%f,%f", acc.x, acc.y, acc.z);
    naos_publish("acc", buf, 0, false, NAOS_LOCAL);
  }

  // publish gyroscope if enabled
  if (use_gyr) {
    imu_vector_t gyr = imu_read_gyroscope();
    char buf[64];
    sprintf(buf, "%f,%f,%f", gyr.x, gyr.y, gyr.z);
    naos_publish("gyr", buf, 0, false, NAOS_LOCAL);
  }

  // publish magnetometer if enabled
  if (use_mag) {
    imu_vector_t mag = imu_read_magnetometer();
    char buf[64];
    sprintf(buf, "%f,%f,%f", mag.x, mag.y, mag.z);
    naos_publish("mag", buf, 0, false, NAOS_LOCAL);
  }

  // save time
  last_interval = naos_millis();
}

static naos_param_t params[] = {
    {.name = "interval", .type = NAOS_LONG, .default_l = 100, .sync_l = &interval},
    {.name = "accelerometer", .type = NAOS_BOOL, .default_b = false, .sync_b = &use_acc},
    {.name = "gyroscope", .type = NAOS_BOOL, .default_b = false, .sync_b = &use_gyr},
    {.name = "magnetometer", .type = NAOS_BOOL, .default_b = false, .sync_b = &use_mag},
    {.name = "temperature", .type = NAOS_BOOL, .default_b = false, .sync_b = &use_tmp},
};

static float battery() { return bat_read_factor(); }

static naos_config_t config = {.device_type = "blimpy",
                               .firmware_version = "0.2.1",
                               .parameters = params,
                               .num_parameters = 5,
                               .ping_callback = ping,
                               .loop_callback = loop,
                               .loop_interval = 10,
                               .battery_level = battery,
                               .status_callback = status};

void app_main() {
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

  // init exp
  exp_init();

  // initialize inertial measurement unit
  imu_init();

  // set led
  status(NAOS_DISCONNECTED);

  // initialize naos
  naos_init(&config);
}
