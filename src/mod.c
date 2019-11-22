#include <art32/matrix.h>
#include <art32/numbers.h>
#include <art32/strconv.h>
#include <math.h>
#include <naos.h>
#include <stdio.h>
#include <string.h>

#include "mod.h"

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

    // calculate torques (mx = -fy * dz + fz * dy), (my = -fx * dz + fz * dx), (mz = -fx * dy + fy * dx)
    double mx = -cfg.values[1] * cfg.values[5] + cfg.values[2] * cfg.values[4];
    double my = -cfg.values[0] * cfg.values[5] + cfg.values[2] * cfg.values[3];
    double mz = -cfg.values[0] * cfg.values[4] + cfg.values[1] * cfg.values[3];

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

  // TODO: Provide feedback on model calculations (singular matrices..).

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
a32_vector_t mod_calc(a32_vector_t values) {
  // multiply values with range
  a32_vector_multiply(values, range);

  // multiply vector with matrix
  a32_vector_t output = a32_vector_multiply_matrix(values, model);

  // clamp results
  for (size_t e = 0; e < output.len; e++) {
    output.values[e] = a32_constrain_d(output.values[e], -1, 1);
  }

  // print calculation
  naos_log("calculated values:");
  a32_vector_print(output);

  return output;
}
