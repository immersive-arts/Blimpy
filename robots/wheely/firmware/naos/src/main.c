#include <naos.h>
#include <string.h>
#include <art32/numbers.h>
#include <art32/strconv.h>

#include "cmd.h"

void online() {
	naos_subscribe("forces", 0, NAOS_LOCAL);
}

void message(const char* topic, uint8_t* payload, size_t len, naos_scope_t scope) {
	 // set model forces and torques "fx,fy,fz,mx,my,mz" (-1 to 1)
	  if (scope == NAOS_LOCAL && strcmp(topic, "forces") == 0) {
	        // parse comma separated speeds
	        double data[6];
	        char *ptr = strtok((char *)payload, ",");
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

static naos_config_t config = {
    .device_type = "wheely",
    .firmware_version = "0.1.0",
    .online_callback = online,
    .message_callback = message,
};

void app_main() {
  cmd_init();

  // initialize naos
  naos_init(&config);
}
