#include <art32/numbers.h>
#include <driver/adc.h>

void bat_init() {
  // set attenuation
  ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_11db));
}

float bat_read_voltage() {
  // read voltage
  float v = adc1_get_raw(ADC1_CHANNEL_6);

  // reverse voltage divider
  v *= 2;

  // convert into voltage
  v *= 3.6;
  v /= 4095;

  return v;
}

float bat_read_factor() {
  // take 4.2V as 1 down to 3.4 as 0
  return a32_safe_map_f(bat_read_voltage(), 3.4, 4.2, 0, 1);
}
