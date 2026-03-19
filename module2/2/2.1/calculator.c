#include "calculator.h"

int add(double a, double b, double *result){
    *result = a + b;
    return 0;
}
int substract(double a, double b, double *result){
    *result = a - b;
    return 0;
}
int multiply(double a, double b, double *result){
    *result = a * b;
    return 0;
}
int divide(double a, double b, double *result){
    *result = a / b;
    if(b == 0)
        return 1;
    
    return 0;
}
int max(double a, double b, double *result){
    if(a > b)
        *result = a;
    else if(b > a)
        *result = b;
    else
        return 1;
    
    return 0;
}
int min(double a, double b, double *result){
    if(a < b)
        *result = a;
    else if(b < a)
        *result = b;
    else
        return 1;
    
    return 0;
}