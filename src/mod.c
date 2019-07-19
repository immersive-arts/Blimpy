#include <math.h>

#include "mod.h"

// max force, max torque and inverse matrix calculated by calc.py
static double FORCE_MAX = 721.24891681;
static double TORQUE_MAX = 360.6244584051392;
static double MODEL[4][4] = {{ 0.35355339, -0.35355339, -0.70710678,  0.70710678},
                             { 0.35355339,  0.35355339,  0.70710678,  0.70710678},
                             { 0.35355339,  0.35355339, -0.70710678, -0.70710678},
                             { 0.35355339, -0.35355339,  0.70710678, -0.70710678}};



static int convert(double v) {
  // clamp to duty cycle range
  v = fmin(fmax(v, -255), 255);

  // reduce low values to zero
  if (fabs(v) < 50) {
    return 0;
  }

  return (int)v;
}

// calculate motor duty cycles from forces and torques
mod_result_t mod_calc(double fx, double fz, double mx, double mz) {
  // get forces
  double ffx = fx * FORCE_MAX;
  double ffz = fz * FORCE_MAX;

  // get torque
  double mmx = mx * TORQUE_MAX;
  double mmz = mz * TORQUE_MAX;

  // get motor duty cycles
  double m1 = MODEL[0][0] * ffx + MODEL[0][1] * ffz + MODEL[0][2] * mmx + MODEL[0][3] * mmz;
  double m2 = MODEL[1][0] * ffx + MODEL[1][1] * ffz + MODEL[1][2] * mmx + MODEL[1][3] * mmz;
  double m3 = MODEL[2][0] * ffx + MODEL[2][1] * ffz + MODEL[2][2] * mmx + MODEL[2][3] * mmz;
  double m4 = MODEL[3][0] * ffx + MODEL[3][1] * ffz + MODEL[3][2] * mmx + MODEL[3][3] * mmz;

  // prepare result
  mod_result_t res = {
      .v = {
          .m1 = convert(m1),
          .m2 = convert(m2),
          .m3 = convert(m3),
          .m4 = convert(m4),
      }
  };

  return res;
}
