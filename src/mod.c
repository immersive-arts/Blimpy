#include <art32/matrix.h>
#include <art32/strconv.h>
#include <naos.h>
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

static a32_matrix_t model;

static a32_vector_t parse_config(char *config) {
  // allocate array
  a32_vector_t vec = a32_vector_new(6);

  // parse comma separated speeds
  char *ptr = strtok((char *)config, ",");
  int i = 0;
  while (ptr != NULL && i < vec.len) {
    vec.data[i] = a32_str2f(ptr);
    ptr = strtok(NULL, ",");
    i++;
  }

  return vec;
}

static void calculate_torque(a32_vector_t vec) {
  // mx = fy * dz + fz * dy;
  float mx = vec.data[1] * vec.data[5] + vec.data[2] * vec.data[4];

  // my = fx * dz + fz * dx;
  float my = vec.data[0] * vec.data[5] + vec.data[2] * vec.data[3];

  // mz = fx * dy + fy * dx;
  float mz = vec.data[0] * vec.data[4] + vec.data[1] * vec.data[3];

  // replace distances with torques
  vec.data[3] = mx;
  vec.data[4] = my;
  vec.data[5] = mz;
}

void mod_calculate() {
  // set static configuration
  // char m1cs[] = "1, 0, -1, 0,  75,  25";
  // char m2cs[] = "1, 0,  1, 0,  75, -25";
  // char m3cs[] = "1, 0,  1, 0, -75,  25";
  // char m4cs[] = "1, 0, -1, 0, -75, -25";

  // load motor configurations
  char *m1cs = strdup(naos_get("model-m1"));
  char *m2cs = strdup(naos_get("model-m2"));
  char *m3cs = strdup(naos_get("model-m3"));
  char *m4cs = strdup(naos_get("model-m4"));

  // vectors have the layout: fx,fy,fz,dx,dy,dz

  // parse configurations
  a32_vector_t m1c = parse_config(m1cs);
  a32_vector_t m2c = parse_config(m2cs);
  a32_vector_t m3c = parse_config(m3cs);
  a32_vector_t m4c = parse_config(m4cs);

  // get force vector views
  a32_vector_t m1cf = a32_vector_view(m1c, 0, 3);
  a32_vector_t m2cf = a32_vector_view(m2c, 0, 3);
  a32_vector_t m3cf = a32_vector_view(m3c, 0, 3);
  a32_vector_t m4cf = a32_vector_view(m4c, 0, 3);

  // normalize force vectors
  a32_vector_norm(m1cf);
  a32_vector_norm(m2cf);
  a32_vector_norm(m3cf);
  a32_vector_norm(m4cf);

  // calculate torques
  calculate_torque(m1c);
  calculate_torque(m2c);
  calculate_torque(m3c);
  calculate_torque(m4c);

  // vectors now have the layout: fx,fy,fz,mx,my,mz

  // TODO: Normalize all motion matrix to range -1..1 (per column)?

  // create matrix
  a32_matrix_t mat = a32_matrix_new(6, 4);
  a32_matrix_set_col(mat, 0, m1c);
  a32_matrix_set_col(mat, 1, m2c);
  a32_matrix_set_col(mat, 2, m3c);
  a32_matrix_set_col(mat, 3, m4c);

  // print matrix
  naos_log("configuration matrix:");
  a32_matrix_print(mat);

  // TODO: Null column issue, use SVD algorithm?

  // calculate right pseudo inverse
  model = a32_matrix_right_pseudo_inverse(mat);

  // TODO: Calculate force max.
  // TODO: Calculate torque max.

  // print matrix
  naos_log("inverse model matrix:");
  a32_matrix_print(model);

  // free objects
  a32_matrix_free(mat);
  a32_vector_free(m1c);
  a32_vector_free(m2c);
  a32_vector_free(m3c);
  a32_vector_free(m4c);
}

// calculate motor duty cycles from forces and torques
mod_result_t mod_calc(double fx, double fy, double fz, double mx, double mz) {
  // get forces
  double ffx = fx * FORCE_MAX_X;
  double ffy = fy * FORCE_MAX_Y;
  double ffz = fz * FORCE_MAX_Z;

  // get torque
  double mmx = mx * TORQUE_MAX_X;
  double mmz = mz * TORQUE_MAX_Z;

  // get motor duty cycles
  double m1 = MODEL[0][0] * ffx + MODEL[0][1] * ffy + MODEL[0][2] * ffz + MODEL[0][3] * mmx + MODEL[0][4] * mmz;
  double m2 = MODEL[1][0] * ffx + MODEL[1][1] * ffy + MODEL[1][2] * ffz + MODEL[1][3] * mmx + MODEL[1][4] * mmz;
  double m3 = MODEL[2][0] * ffx + MODEL[2][1] * ffy + MODEL[2][2] * ffz + MODEL[2][3] * mmx + MODEL[2][4] * mmz;
  double m4 = MODEL[3][0] * ffx + MODEL[3][1] * ffy + MODEL[3][2] * ffz + MODEL[3][3] * mmx + MODEL[3][4] * mmz;
  double m5 = MODEL[4][0] * ffx + MODEL[4][1] * ffy + MODEL[4][2] * ffz + MODEL[4][3] * mmx + MODEL[4][4] * mmz;
  double m6 = MODEL[5][0] * ffx + MODEL[5][1] * ffy + MODEL[5][2] * ffz + MODEL[5][3] * mmx + MODEL[5][4] * mmz;

  naos_log("mod  | %f | %f | %f | %f | %f | %f |", m1, m2, m3, m4, m5, m6);

  // prepare result
  mod_result_t res = {
      .a = {m1, m2, m3, m4, m5, m6},
  };

  return res;
}
