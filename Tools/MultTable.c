#include <stdio.h>

int main(void){

    unsigned short SumTable[512];
    unsigned short DiffTable[256];

    for (int n = 0; n < 512; n++){
        SumTable[n] = (unsigned short) ((n*n)/4);
    }
    
    for (int n = 0; n < 256; n++){
        DiffTable[n] = (unsigned short) ((n*n)/4);
    }

    for (int A = 0; A <= 255; A++){
        DiffTable[A] = (unsigned short) ((A*A) / 4);
    }

    for (int i = 0; i < 16; i++){
        for (int j = 0; j < 32; j++){
            printf("%X\t", SumTable[16*j + i]);
        }
        printf("\n");
    }

    for (int i = 0; i < 16; i++){
        for (int j = 0; j < 16; j++){
            printf("%X\t", DiffTable[16*j + i]);
        }
        printf("\n");
    }

    FILE *output = fopen("./MultTable.bin", "wb");

    fwrite(SumTable, 2, 512, output);
    fwrite(DiffTable, 2, 256, output);
    fclose (output);

    return 0;
}