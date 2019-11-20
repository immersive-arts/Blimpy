typedef union {
  struct {
    double m1;
    double m2;
    double m3;
    double m4;
    double m5;
    double m6;
  } s;
  double a[6];
} mod_result_t;

void mod_calculate();

mod_result_t mod_calc(double fx, double fy, double fz, double mx, double mz);
