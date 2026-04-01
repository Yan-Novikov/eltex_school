const char *op_name = "Multiply";

int op_func(double a, double b, double *result) {
    *result = a * b;
    return 0;
}