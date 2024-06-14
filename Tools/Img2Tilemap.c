#include <stdio.h>
#include <math.h>

typedef struct BMP_Header{
//  Struct to contain standard bitmap header data

    unsigned char Type[2];  //Ascii characters to identify origin of file [Check for "BM"]
    unsigned long FileSize; //Size of file in Bytes
    unsigned short Misc_1;
    unsigned short Misc_2;  //4 bytes used by some prgrams for ... stuff
    unsigned long ImgArrayOffset;    //Offset of bitmap data from start of file

} bmp_h;

typedef struct DIB_Type_BITMAPINFOHEADER{
//  struct for DIB header info

    unsigned long HeaderSize;    //==40.
    long ImgWidth;
    long ImgHeight;
    unsigned short NoColourPlanes;   //Must == 1
    unsigned short ImgBPP;   //Usually 1, 2, 4, 8, 16, 24, 32
    unsigned long CompressionType;   //Most commonly 0 (none)
    unsigned long ImgSize;   //Size of raw bitmap data
    long PrintWidth; //Pixels/meter of print
    long PrintHeight; //Pixels/meter of print
    unsigned long NoColours;  //Size of palette; can be 0, in which case there are 2^n colours
    unsigned long NoImportantColours;   //Usually ignored

} dib_h;

void Import_ImgData(FILE *Img, bmp_h *bmp, dib_h *dib);
void Print_ImgData(bmp_h *bmp, dib_h *dib);
void Extract_FontTiles(FILE *Font, bmp_h *bmp_h, dib_h *dib_h, char Tiles[256][8]);
void Extract_ImgTiles(FILE *Font, bmp_h *bmp_h, dib_h *dib_h, char Tiles[32][24][8], char Attributes[32][24]);
void Print_Font(char Tiles[256][8]);
void Print_ImgToFile(char Img_Tiles[32][24][8]);
void Match_Img2Font(char Img[32][24][8], char Font[256][8], unsigned char Tile[32][24]);
void Pack_Img(unsigned char Tiles[32][24], char Cols[32][24]);


int main(void){

    FILE *Image = fopen("./Menu Screen.bmp", "rb");
    FILE *Font = fopen("./MandelFont.bmp", "rb");

    bmp_h Img_BmpHeader;
    dib_h Img_DibHeader;

    bmp_h Font_BmpHeader;
    dib_h Font_DibHeader;
  
    char Img_Tiles[32][24][8];  //Tiles visible on screen
    char Img_Attributes[32][24];     //Attributes (same format as zx spectrum)  
    char Font_Tiles[256][8];
    unsigned char TileMap[32][24] = {0};
    
    Import_ImgData(Image, &Img_BmpHeader, &Img_DibHeader);
    Print_ImgData(&Img_BmpHeader, &Img_DibHeader);
    Import_ImgData(Font, &Font_BmpHeader, &Font_DibHeader);
    Print_ImgData(&Font_BmpHeader, &Font_DibHeader);

    Extract_FontTiles(Font, &Font_BmpHeader, &Font_DibHeader, Font_Tiles);
    Extract_ImgTiles(Image, &Img_BmpHeader, &Img_DibHeader, Img_Tiles, Img_Attributes);
    //Print_Font(Font_Tiles);
    //Print_ImgToFile(Img_Tiles);
    Match_Img2Font(Img_Tiles, Font_Tiles, TileMap);

    Pack_Img(TileMap, Img_Attributes);

    fclose(Image);
    fclose(Font);

    return 0;
}

void Import_ImgData(FILE *Img, bmp_h *bmp, dib_h *dib){

    bmp->Type[0] = fgetc(Img);
    bmp->Type[1] = fgetc(Img);
    fread(&bmp->FileSize, 4, 1, Img);
    fread(&bmp->Misc_1, 2, 1, Img);
    fread(&bmp->Misc_2, 2, 1, Img);
    fread(&bmp->ImgArrayOffset, 4, 1, Img);

    fread(&dib->HeaderSize, 4, 1, Img);
    fread(&dib->ImgWidth, 4, 1, Img);
    fread(&dib->ImgHeight, 4, 1, Img);
    fread(&dib->NoColourPlanes, 2, 1, Img);
    fread(&dib->ImgBPP, 2, 1, Img);
    fread(&dib->CompressionType, 4, 1, Img);
    fread(&dib->ImgSize, 4, 1, Img);
    fread(&dib->PrintWidth, 4, 1, Img);
    fread(&dib->PrintHeight, 4, 1, Img);
    fread(&dib->NoColours, 4, 1, Img);
    fread(&dib->NoImportantColours, 4, 1, Img);

    return;
}

void Print_ImgData(bmp_h *bmp, dib_h *dib){

    printf("ID: %c%c\nFile size: %d\nBitmap offset: %d\n", bmp->Type[0], bmp->Type[1], bmp->FileSize, bmp->ImgArrayOffset);
    printf("DIB header size: %d\nPx Width: %d\nPx Height: %d\n",dib->HeaderSize, dib->ImgWidth, dib->ImgHeight);
    printf("# Col Planes: %d\nbpp: %d\nCompression: %d\n", dib->NoColourPlanes, dib->ImgBPP, dib->CompressionType);
    printf("Print width: %d\nPrint height: %d\n", dib->PrintWidth, dib->PrintHeight);
    printf("# Colours: %d\n# Important colours: %d\n\n", dib->NoColours, dib->NoImportantColours);

    return;
}

void Extract_FontTiles(FILE *Font, bmp_h *bmp_h, dib_h *dib_h, char Tiles[256][8]){

    int LineLen = (int)(ceil(dib_h->ImgBPP * dib_h->ImgWidth / 32.0)) * 4;

    for(int l = 0; l < 16; l++){ 
        for(int t = 0; t < 16; t++){

            fseek(Font, bmp_h->ImgArrayOffset + (l * 8 * LineLen) + (15 - t), SEEK_SET);

            for(int i = 7; i>=0; i--){

                Tiles[255 - (16*l + t)][i] = fgetc(Font);
                fseek(Font, LineLen-1, SEEK_CUR);
            }
        }
    }

    return;
}

void Extract_ImgTiles(FILE *Img, bmp_h *bmp_h, dib_h *dib_h, char Tiles[32][24][8], char Attributes[32][24]){
   
    int LineLen = (int)(ceil(dib_h->ImgBPP * dib_h->ImgWidth / 32.0)) * 4;
    unsigned char DataBuffer[8][4];
    char PxMask[8];
    char ColourBuffer[8][8];
    char fg, bg, bright;


    for(int y = 23; y >= 0; y--){ 
        for(int x = 0; x < 32; x++){

            fseek(Img, bmp_h->ImgArrayOffset + ((23-y) * 8 * LineLen) + (4*x), SEEK_SET);

            for(int i = 7; i>=0; i--){   //Read in all of tile
                
                DataBuffer[i][0] = fgetc(Img);
                DataBuffer[i][1] = fgetc(Img);
                DataBuffer[i][2] = fgetc(Img);
                DataBuffer[i][3] = fgetc(Img);
                fseek(Img, LineLen-4, SEEK_CUR);
            }



            for(int i = 0; i<8; i++){   //Separate each px colour index

                ColourBuffer[i][0] = (DataBuffer[i][0] & 0xF0) >> 4;
                ColourBuffer[i][1] = DataBuffer[i][0] & 0x0F;
                ColourBuffer[i][2] = (DataBuffer[i][1] & 0xF0) >> 4;
                ColourBuffer[i][3] = DataBuffer[i][1] & 0x0F;
                ColourBuffer[i][4] = (DataBuffer[i][2] & 0xF0) >> 4;
                ColourBuffer[i][5] = DataBuffer[i][2] & 0x0F;
                ColourBuffer[i][6] = (DataBuffer[i][3] & 0xF0) >> 4;
                ColourBuffer[i][7] = DataBuffer[i][3] & 0x0F;

            }

            for(int i=0; i<8; i++){
                Tiles[x][y][i] = 0;
                for(int j=0; j<8; j++){

                    if((ColourBuffer[i][j] & 0x07) != 0){
                        Tiles[x][y][i] = Tiles[x][y][i] | (0x80 >> j);
                    }
                
                }
            }

            
            bg = ColourBuffer[0][0];
            for(int i=0;i<64; i++){
                if(ColourBuffer[i>>3][i&7] != bg){
                    fg = ColourBuffer[i>>3][i&7];
                    break;
                }
            }
            
            bright = (bg >= 8 || fg >= 8) ? 1:0;

            Attributes[x][y] = 0;
            if(bright){
                Attributes[x][y] = Attributes[x][y] | 0b01000000;
            }
            Attributes[x][y] = Attributes[x][y] | (bg & 7) << 3 | fg & 7;

        }
    }

    return;
}

void Print_Font(char Tiles[256][8]){

    for(int t = 0; t<256; t++){
        for(int i = 0; i<8; i++){

            printf("%c", (Tiles[t][i] & 0x80 ? '#':' '));
            printf("%c", (Tiles[t][i] & 0x40 ? '#':' '));
            printf("%c", (Tiles[t][i] & 0x20 ? '#':' '));
            printf("%c", (Tiles[t][i] & 0x10 ? '#':' '));
            printf("%c", (Tiles[t][i] & 0x08 ? '#':' '));
            printf("%c", (Tiles[t][i] & 0x04 ? '#':' '));
            printf("%c", (Tiles[t][i] & 0x02 ? '#':' '));
            printf("%c", (Tiles[t][i] & 0x01 ? '#':' '));
            printf("\n");

        }
        printf("\n");
    }
    return;
}

void Print_ImgToFile(char Img_Tiles[32][24][8]){

    FILE *fp = fopen("./test.txt", "w");

    for(int y = 0; y<24; y++){
        for(int i = 0; i<8; i++){
            for(int x = 0; x<32; x++){

                //printf("%X", Img_Tiles[x][y][i]);

                fputc((Img_Tiles[x][y][i] & 0x80 ? '#':' '), fp);
                fputc((Img_Tiles[x][y][i] & 0x40 ? '#':' '), fp);
                fputc((Img_Tiles[x][y][i] & 0x20 ? '#':' '), fp);
                fputc((Img_Tiles[x][y][i] & 0x10 ? '#':' '), fp);
                fputc((Img_Tiles[x][y][i] & 0x08 ? '#':' '), fp);
                fputc((Img_Tiles[x][y][i] & 0x04 ? '#':' '), fp);
                fputc((Img_Tiles[x][y][i] & 0x02 ? '#':' '), fp);
                fputc((Img_Tiles[x][y][i] & 0x01 ? '#':' '), fp);

            }
            fputc('\n', fp);
        }
    }

    fclose(fp);

    return;
}

void Match_Img2Font(char Img[32][24][8], char Font[256][8], unsigned char Tile[32][24]){

    unsigned char tmp[8];

    for(int y = 0; y < 24; y++){
        for(int x = 0; x < 32; x++){

            for(int t = 0; t < 256; t++){

                for(int i = 0; i < 8; i++){
                    tmp[i] = Img[x][y][i] ^ Font[t][i];
                }
                if(!(tmp[0] | tmp[1] | tmp[2] | tmp[3] | tmp[4] | tmp[5] | tmp[6] | tmp[7])){
                    Tile[x][y] = t;
                    break;
                }
                if(t > 250){
                    Tile[x][y] = 0x80;
                    break;
                }
            }
        }
    }

/*
    for(int y = 0; y < 24; y++){
        for(int x = 0; x < 32; x++){
    
            if(Tile[x][y] == 0x5B){
                printf("  ");
            }else{
                printf("%.2X", Tile[x][y]);
            }

        }
        printf("\n");
    }
*/

    return;
}

void Pack_Img(unsigned char Tiles[32][24], char Cols[32][24]){

    FILE *out = fopen("./out.bin", "wb");

    for(int y = 0; y < 24; y++){
        for(int x = 0; x < 32; x++){
            fputc(Tiles[x][y], out);
        }
    }
    for(int y = 0; y < 24; y++){
        for(int x = 0; x < 32; x++){
            fputc(Cols[x][y], out);
        }
    }

    fclose(out);
    return;
}




