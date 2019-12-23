#include <stdio.h>


int main(int argc, char* argv[]){
    FILE* input;
    FILE* output;
    char address[20];
    char function[100];

    input = fopen("bin/kernel.sym", "r");
    output = fopen("bin/symfile.s", "w");

    fprintf(output, "[BITS 32]\n");
    fprintf(output, "SECTION .symbols\n");

    while( fscanf(input, "0x00000000%s %s\n", address, function) == 2 ){
        fprintf(output, "dd 0x%s\n", address);
        fprintf(output, "dd %s\n", function);
    }

    fprintf(output, "dd 0x0\n");
    fprintf(output, "dd 0x0\n");

    fseek(input, 0, SEEK_SET);

    while( fscanf(input, "0x00000000%s %s\n", address, function) == 2 ){
        fprintf(output, "%s:\n", function);
        fprintf(output, "db \"%s\", 0\n", address);
    }

    fclose(input);
    fclose(output);
    return 0;
}