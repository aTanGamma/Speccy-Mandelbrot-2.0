#include <stdio.h>

int main(void){

    FILE *fp = fopen("SquareTable.bin", "wb");

    unsigned short Table[256];

    for(int i = 0; i<256; i++){

        Table[i] = (unsigned short) (i*i);

    }

    fwrite(Table, 2, 256, fp);
    fclose(fp);

}