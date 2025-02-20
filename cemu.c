#include <stdio.h>
#include <time.h>
#include <stdlib.h>


#include <raylib.h>

static int const PIXEL_SIZE = 10;
static int const WIDTH      = 64;
static int const HEIGHT     = 32;

unsigned long long display[32] = {0};

unsigned char memory[4096] = {0};

unsigned char regV[16] = {0};
unsigned short regI = 0;
unsigned char regDelay = 0;
unsigned char regSound = 0;
unsigned short regPC = 0;
unsigned short regSP = 0;
unsigned short stack[16] = {0};

long const max_rom_len = (0xFFF - 0x200 + 1);

int main(int argc, char * argv[]){
    if (argc != 2) {
        printf("usage: ./cemu ROM_FILE\n");
        return 1;
    }
    char const * rom_file = argv[1];

    srand(time(NULL));

    InitWindow(WIDTH*PIXEL_SIZE, HEIGHT*PIXEL_SIZE, "cemu");
    SetTargetFPS(60);

    printf("Hello, CEMU-8!\n");

    printf("Loading file: \'%s\' into memory!\n", rom_file);
    FILE * fileptr = fopen(rom_file, "rb");
    fseek(fileptr, 0, SEEK_END);
    long filelen = ftell(fileptr);
    rewind(fileptr);
    if (filelen > max_rom_len) {
        printf("usage: ./cemu-8 ROM_FILE\n");
        return 2;
    }
    fread(&memory[0x200], 1, filelen, fileptr);
    fclose(fileptr);

    printf("Setting regPC to 0x200\n");
    regPC = 0x200;

    while(!WindowShouldClose()){
        for(int k = 0; k < 10; k++){
            unsigned short const inst = (memory[regPC] << 8) | memory[regPC+1];
            switch (inst>>(3*4)) {
                case 0x1: {
                    unsigned short nnn = (inst & 0x0FFF) >> 0;
                    regPC = nnn-2;
                    break;
                }
                case 0x3: {
                    unsigned char destReg = (inst & 0x0F00) >> 8;
                    unsigned char cmpVal  =  inst & 0x00FF;
                    if (regV[destReg] == cmpVal) {
                        regPC += 2;
                    }

                    break;
                }
                case 0x4: {
                    unsigned char destReg = (inst & 0x0F00) >> 8;
                    unsigned char cmpVal  =  inst & 0x00FF;
                    if (regV[destReg] != cmpVal) {
                        regPC += 2;
                    }

                    break;
                }
                case 0x6: {
                    unsigned char x  = (inst & 0x0F00) >> 8;
                    unsigned char kk  = (inst & 0x00FF) >> 0;
                    regV[x] = kk;

                    break;
                }
                case 0x7: {
                    unsigned char x  = (inst & 0x0F00) >> 8;
                    unsigned char kk  = (inst & 0x00FF) >> 0;
                    regV[x] = regV[x] + kk;

                    break;
                }
                case 0x8: {
                    unsigned char x  = (inst & 0x0F00) >> 8;
                    unsigned char y  = (inst & 0x00F0) >> 4;
                    switch ((inst & 0x000F)){
                        case 0x0: {
                            regV[x] = regV[y];
                            break;
                        }
                        default:
                            printf("unknown inst: %04X\n", inst);
                            return 4;
                    }
                    break;
                }
                case 0xA:
                    regI = inst & 0x0FFF;
                    break;
                case 0xC: {
                    int const r = rand() & 0xFF;
                    unsigned char destReg  = (inst & 0x0F00) >> 8;
                    unsigned char instMask =  inst & 0x00FF;
                    regV[destReg] = instMask & r;
                    break;
                }
                case 0xD: {
                    unsigned char x  = regV[(inst & 0x0F00) >> 8];
                    unsigned char y  = regV[(inst & 0x00F0) >> 4];
                    unsigned char n  = (inst & 0x000F) >> 0;
                    unsigned char collision = 0;
                    for (unsigned char lin = 0; lin < n; lin++) {
                        for (unsigned char col = 0; col < 8; col++) {
                            unsigned char memXshift = 7-col;
                            unsigned char pixel = (memory[regI+lin] >> memXshift) & 1;
                            unsigned char displayX = (x + col) % 64;
                            unsigned char displayXshift = 63-displayX;
                            unsigned char displayY = (y + lin) % 32;
                            unsigned char displayPixel = (((display[displayY])>>displayXshift)&1);
                            if (pixel){
                                if (displayPixel) {
                                    display[displayY] = display[displayY] & (~(1<<displayXshift));
                                    collision = 1;
                                }
                                else {
                                    display[displayY] = display[displayY] | ((long long unsigned)1<<displayXshift);
                                }
                            }
                        }
                    }
                    regV[0xF] = collision;
                    break;
                }
                default:
                    printf("unknown inst: %04X\n", inst);
                    return 3;
            }
            //printf("inst: %04X\n", inst);
            regPC += 2;
        }

        BeginDrawing();
        ClearBackground(BLACK);
        for (int lin = 0; lin < HEIGHT; lin++){
            for (int col = 0; col < WIDTH; col++){
                unsigned char displayX = col;
                unsigned char displayXshift = 63-col;
                unsigned char displayY = lin;
                unsigned char displayPixel = (((display[displayY])>>displayXshift)&1);
                if (displayPixel)
                    DrawRectangle(
                        PIXEL_SIZE*displayX,
                        PIXEL_SIZE*displayY,
                        PIXEL_SIZE,
                        PIXEL_SIZE,
                        WHITE
                );

            }
        }
        EndDrawing();
    }

    return 0;
}
