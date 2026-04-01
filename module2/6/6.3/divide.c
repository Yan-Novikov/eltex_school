const char *op_name = "Divide";

int op_func(double a, double b, double *result) {
    if (b == 0)
        return 1;
    *result = a / b;
    return 0;
}