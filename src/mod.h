typedef union{
    struct {
        int m1;
        int m2;
        int m3;
        int m4;
    } v;
    int * a;
} mod_result_t;

mod_result_t mod_calc(double fx, double fz, double mx, double mz);
