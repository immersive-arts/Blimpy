#include <art32/matrix.h>
#include <art32/numbers.h>
#include <art32/strconv.h>
#include <math.h>
#include <naos.h>
#include <stdio.h>
#include <string.h>

#include "mod.h"

// max force, max torque and inverse matrix calculated by calc.py
static double FORCE_MAX_X = 2.82842712474619;
static double FORCE_MAX_Y = 2.0;
static double FORCE_MAX_Z = 2.82842712474619;
static double TORQUE_MAX_X = 7.778174593052022;
static double TORQUE_MAX_Z = 4.242640687119285;
static double MODEL[6][5] = {{3.53553391e-01, 1.54914841e-17, -3.53553391e-01, 9.86660625e-02, 2.35702260e-01},
                             {3.53553391e-01, -1.54914841e-17, 3.53553391e-01, -9.86660625e-02, 2.35702260e-01},
                             {3.53553391e-01, 1.54914841e-17, 3.53553391e-01, 9.86660625e-02, -2.35702260e-01},
                             {3.53553391e-01, -1.54914841e-17, -3.53553391e-01, -9.86660625e-02, -2.35702260e-01},
                             {0.00000000e+00, 5.00000000e-01, -3.92523115e-17, -1.64443437e-01, 1.94750590e-18},
                             {0.00000000e+00, 5.00000000e-01, -3.92523115e-17, 1.64443437e-01, -4.11998174e-17}};

static a32_matrix_t model = {0};
static a32_vector_t range = {0};

static a32_vector_t parse_config(char *config) {
  // allocate array
  a32_vector_t vec = a32_vector_new(6);

  // parse comma separated speeds
  char *ptr = strtok((char *)config, ",");
  int i = 0;
  while (ptr != NULL && i < vec.len) {
    vec.values[i] = a32_str2d(ptr);
    ptr = strtok(NULL, ",");
    i++;
  }

  return vec;
}

void mod_calculate() {
  // static model
  a32_matrix_t st = a32_matrix_use((double *)MODEL, 6, 5);
  a32_matrix_print(st);
  a32_matrix_free(st);

  // free existing model and range
  if (model.values != NULL) {
    a32_matrix_free(model);
  }
  if (range.values != NULL) {
    a32_vector_free(range);
  }

  // create configuration matrix
  a32_matrix_t config = a32_matrix_new(6, 6);

  // parse individual motor configuration
  for (int i = 0; i < 6; i++) {
    // get param name
    char param[8] = {0};
    sprintf(param, "model-m%d", i + 1);

    // parse string config
    a32_vector_t cfg = parse_config(naos_get(param));

    // vector has the layout: fx, fy, fz, dx, dy, dz

    // normalize forces
    a32_vector_norm(a32_vector_view(cfg, 0, 3));

    // calculate torques (mx = fy * dz + fz * dy), (my = fx * dz + fz * dx), (mz = fx * dy + fy * dx)
    double mx = cfg.values[1] * cfg.values[5] + cfg.values[2] * cfg.values[4];
    double my = cfg.values[0] * cfg.values[5] + cfg.values[2] * cfg.values[3];
    double mz = cfg.values[0] * cfg.values[4] + cfg.values[1] * cfg.values[3];

    // set torques
    cfg.values[3] = mx;
    cfg.values[4] = my;
    cfg.values[5] = mz;

    // vector now has the layout: fx, fy, fz, mx, my, mz

    // add to matrix
    a32_matrix_set_col(config, i, cfg);

    // free vector
    a32_vector_free(cfg);
  }

  // print matrix
  naos_log("configuration matrix:");
  a32_matrix_print(config);

  // calculate right pseudo inverse
  model = a32_matrix_pseudo_inverse(config);

  // print matrix
  naos_log("inverse model matrix:");
  a32_matrix_print(model);

  // allocate range vector
  range = a32_vector_new(6);

  // sum model rows to get input range
  for (size_t i = 0; i < 6; i++) {
    double sum = 0;
    for (size_t c = 0; c < 6; c++) {
      sum += fabs(A32_MAT(config, i, c));
    }
    range.values[i] = sum;
  }

  // print range vector
  naos_log("range vector:");
  a32_vector_print(range);

  // free configuration matrix
  a32_matrix_free(config);
}

// calculate motor duty cycles from forces and torques
mod_result_t mod_calc(double fx, double fy, double fz, double mx, double mz) {
  // get maximums
  double ffx = fx * FORCE_MAX_X;
  double ffy = fy * FORCE_MAX_Y;
  double ffz = fz * FORCE_MAX_Z;
  double mmx = mx * TORQUE_MAX_X;
  double mmz = mz * TORQUE_MAX_Z;

  // get motor duty cycles
  double m1 = MODEL[0][0] * ffx + MODEL[0][1] * ffy + MODEL[0][2] * ffz + MODEL[0][3] * mmx + MODEL[0][4] * mmz;
  double m2 = MODEL[1][0] * ffx + MODEL[1][1] * ffy + MODEL[1][2] * ffz + MODEL[1][3] * mmx + MODEL[1][4] * mmz;
  double m3 = MODEL[2][0] * ffx + MODEL[2][1] * ffy + MODEL[2][2] * ffz + MODEL[2][3] * mmx + MODEL[2][4] * mmz;
  double m4 = MODEL[3][0] * ffx + MODEL[3][1] * ffy + MODEL[3][2] * ffz + MODEL[3][3] * mmx + MODEL[3][4] * mmz;
  double m5 = MODEL[4][0] * ffx + MODEL[4][1] * ffy + MODEL[4][2] * ffz + MODEL[4][3] * mmx + MODEL[4][4] * mmz;
  double m6 = MODEL[5][0] * ffx + MODEL[5][1] * ffy + MODEL[5][2] * ffz + MODEL[5][3] * mmx + MODEL[5][4] * mmz;

  // clamp results
  m1 = a32_constrain_d(m1, -1, 1);
  m2 = a32_constrain_d(m2, -1, 1);
  m3 = a32_constrain_d(m3, -1, 1);
  m4 = a32_constrain_d(m4, -1, 1);
  m5 = a32_constrain_d(m5, -1, 1);
  m6 = a32_constrain_d(m6, -1, 1);

  // print calculation
  naos_log("model: %f | %f | %f | %f | %f | %f", m1, m2, m3, m4, m5, m6);

  // prepare result
  mod_result_t res = {
      .a = {m1, m2, m3, m4, m5, m6},
  };

  return res;
}
