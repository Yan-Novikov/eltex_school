#ifndef CALCULATOR_H
#define CALCULATOR_H

typedef struct {
    const char *name;
    int (*func)(double, double, double*);
} Operation;

extern const Operation operations[];
extern const int num_operations;

int add(double a, double b, double *result);
int substract(double a, double b, double *result);
int multiply(double a, double b, double *result);
int divide(double a, double b, double *result);
int max(double a, double b, double *result);
int min(double a, double b, double *result);

#endif