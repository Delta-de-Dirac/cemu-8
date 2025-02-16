#include <stdio.h>
#include <time.h>
#include <stdlib.h>

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
    while(1){
        unsigned short const inst = (memory[regPC] << 8) | memory[regPC+1];
        switch (inst>>12) {
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
            default:
                printf("unknown inst: %04X\n", inst);
                return 3;
                // code block
        }
        regPC += 2;
    }

    return 0;
}
