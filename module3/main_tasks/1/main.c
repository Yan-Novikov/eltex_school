#include "proc.h"
#include <stdio.h>

int main(int argc, char* argv[]){
    if(argc <= 1){
        printf("Программе нужно передать аргументы!\n");
        return 0;
    }
    arg_processing(argc, argv);
}