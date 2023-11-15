#include <naos.h>
#include <naos/ble.h>
#include <naos/wifi.h>
#include <naos/mqtt.h>
#include <naos/manager.h>
#include <string.h>
#include <art32/numbers.h>
#include <art32/convert.h>
#include <stdio.h>

#include "cmd.h"

void online() {
  // subscribe topics
  naos_subscribe("forces", 0, NAOS_LOCAL);
}

void message(const char* topic, const uint8_t* payload, size_t len, naos_scope_t scope) {
  // set model forces and torques "fx,fy,fz,mx,my,mz" (-1 to 1)
  if (scope == NAOS_LOCAL && strcmp(topic, "forces") == 0) {
    // parse comma separated speeds
    double data[6];
    char* ptr = strtok((char*)payload, ",");
    int i = 0;
    while (ptr != NULL && i < 6) {
      data[i] = a32_constrain_d(a32_str2d(ptr), -1, 1);
      ptr = strtok(NULL, ",");
      i++;
    }

    naos_log("Message: %f %f %f %f %f %f", data[0], data[1], data[2], data[3], data[4], data[5]);
    cmd_update(data[0], data[1], data[2], data[3], data[4], data[5]);
  }
}

static void loop() {
  // publish data
  char str[128];
  sprintf(str, "%f,%f,%f,%f,%f", 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  naos_publish_s("battery", str, 0, false, NAOS_LOCAL);
}

static naos_config_t config = {
    .device_type = "wheely",
    .device_version = "0.1.0",
    .online_callback = online,
    .message_callback = message,
    .loop_callback = loop,
    .loop_interval = 300,
};

void app_main() {
  cmd_init();

  // initialize naos
  naos_init(&config);
  naos_ble_init((naos_ble_config_t){});
  naos_wifi_init();
  naos_mqtt_init(1);
  naos_manager_init();
  naos_start();
}
