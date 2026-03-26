#include <stdio.h>
#include "calculator.h"

void scan_arg(double *a, double *b){
    printf("Type arguments a and b with space between them: ");
    scanf("%lf %lf", a, b);
}

int main(){
    double result, a, b;
    int choice;

    while(1){
        printf("\nChoose your calculator operation.\n");
        for(int i = 0; i < num_operations; i++){
            printf("%d. %s\n", i + 1, operations[i].name);
        }
        printf("%d. Exit\n", num_operations + 1);
        printf(">");

        scanf("%d", &choice);

        if(choice == num_operations + 1){
            return 0;
        }

        if(choice < 1 || choice > num_operations){
            printf("Invalid choice. Please try again.\n");
            continue;
        }

        const Operation *op = &operations[choice - 1];

        scan_arg(&a, &b);

        int status = op->func(a, b, &result);

        if(status != 0){
            printf("Error: operation failed (division by zero or equal arguments for min/max).\n");
        } else {
            printf("Result: %lf\n", result);
        }
    }
}