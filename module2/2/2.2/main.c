#include <stdio.h>
#include "calculator.h"

void scan_arg(double *a, double *b){
    printf("Type arguments a and b with space between them: ");
    scanf("%lf %lf", a, b);
}

int main(){
    double result, a, b;
    while(1){
        printf("\nChoose your calculator operation.\n");
        printf("1. Add\n");
        printf("2. Substract\n");
        printf("3. Multiply\n");
        printf("4. Divide\n");
        printf("5. Min\n");
        printf("6. Max\n");
        printf("7. Exit\n");
        printf(">");

        int choice;
        scanf("%d", &choice);

        switch (choice) {
        case 1:
            scan_arg(&a, &b);
            add(a, b, &result);
            printf("Result: %lf", result);
            break;

        case 2:
            scan_arg(&a, &b);
            substract(a, b, &result);
            printf("Result: %lf", result);
            break;

        case 3:
            scan_arg(&a, &b);
            multiply(a, b, &result);
            printf("Result: %lf", result);
            break;

        case 4:
            scan_arg(&a, &b);
            if(divide(a, b, &result))
                printf("You cannot divide by 0");
            else
                printf("Result: %lf", result);

            break;
        
        case 5:
            scan_arg(&a, &b);
            if(max(a, b, &result))
                printf("Arguments is equal");
            else
                printf("Result: %lf", result);
            break;

        case 6:
            scan_arg(&a, &b);
            if(min(a, b, &result))
                printf("Arguments is equal");
            else
                printf("Result: %lf", result);
            break;

        case 7:
            return 0;
            break;
        }
    }
}