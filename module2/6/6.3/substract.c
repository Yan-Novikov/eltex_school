const char *op_name = "Substract";

int op_func(double a, double b, double *result) {
    *result = a - b;
    return 0;
}