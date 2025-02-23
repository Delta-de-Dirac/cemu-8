#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <raylib.h>

#define PIXEL_SIZE 10
#define WIDTH 64
#define HEIGHT 32

uint64_t display[HEIGHT] = {0};

uint8_t memory[4096] = {0};

uint8_t regV[16] = {0};
uint16_t regI = 0;
uint8_t regDelay = 0;
uint8_t regSound = 0;
uint16_t regPC = 0;
uint16_t regSP = 0;
uint16_t stack[16] = {0};

uint8_t const tone_data[] = {
    #embed "tone.ogg"
};

uint32_t const maxRomLen           = (0xFFF - 0x200 + 1);
uint16_t const programStartAddr    = 0x200;
uint16_t const digitSpriteAddr[16] = {
    0x000,
    0x005,
    0x00A,
    0x00F,
    0x014,
    0x019,
    0x01E,
    0x023,
    0x028,
    0x02D,
    0x032,
    0x037,
    0x03C,
    0x041,
    0x046,
    0x04B
};

uint8_t awaitingKeyPress = 0;
uint8_t pressedKey = 0x10;

uint32_t const displayFPS = 60;
uint32_t const instPerFrame = 10;

float timeElapsed = 0;
float timerTick   = 0;

uint16_t const keyMap[16] = {
    KEY_X,
    KEY_A,
    KEY_S,
    KEY_D,
    KEY_Q,
    KEY_W,
    KEY_E,
    KEY_ONE,
    KEY_TWO,
    KEY_THREE,
    KEY_Z,
    KEY_C,
    KEY_FOUR,
    KEY_R,
    KEY_F,
    KEY_V
};

int main(int argc, char * argv[]){
    if (argc != 2) {
        printf("usage: ./cemu ROM_FILE\n");
        return 1;
    }
    char const * rom_file = argv[1];

    srand(time(NULL));

    InitWindow(WIDTH*PIXEL_SIZE, HEIGHT*PIXEL_SIZE, "cemu");
    InitAudioDevice();
    Music music = LoadMusicStreamFromMemory(".ogg", tone_data, (int) sizeof tone_data); 
    SetMusicVolume(
        music,
        0.5
    );
    music.looping = true;
    PlayMusicStream(music);
    SetTargetFPS(displayFPS);

    printf("Hello, CEMU-8!\n");

    printf("Loading file: \'%s\' into memory!\n", rom_file);
    FILE * fileptr = fopen(rom_file, "rb");
    fseek(fileptr, 0, SEEK_END);
    uint32_t filelen = ftell(fileptr);
    rewind(fileptr);
    if (filelen > maxRomLen) {
        printf("usage: ./cemu-8 ROM_FILE\n");
        return 2;
    }
    fread(&memory[programStartAddr], 1, filelen, fileptr);
    fclose(fileptr);
    
    printf("Populating digits sprite data");
    for (int i = 0; i < 16; i++){
            switch(i){
                case 0x0:
                    memory[digitSpriteAddr[i]+0] = 0xF0;
                    memory[digitSpriteAddr[i]+1] = 0x90;
                    memory[digitSpriteAddr[i]+2] = 0x90;
                    memory[digitSpriteAddr[i]+3] = 0x90;
                    memory[digitSpriteAddr[i]+4] = 0xF0;
                    break;
                case 0x1:
                    memory[digitSpriteAddr[i]+0] = 0x20;
                    memory[digitSpriteAddr[i]+1] = 0x60;
                    memory[digitSpriteAddr[i]+2] = 0x20;
                    memory[digitSpriteAddr[i]+3] = 0x20;
                    memory[digitSpriteAddr[i]+4] = 0x70;
                    break;
                case 0x2:
                    memory[digitSpriteAddr[i]+0] = 0xF0;
                    memory[digitSpriteAddr[i]+1] = 0x10;
                    memory[digitSpriteAddr[i]+2] = 0xF0;
                    memory[digitSpriteAddr[i]+3] = 0x80;
                    memory[digitSpriteAddr[i]+4] = 0xF0;
                    break;
                case 0x3:
                    memory[digitSpriteAddr[i]+0] = 0xF0;
                    memory[digitSpriteAddr[i]+1] = 0x10;
                    memory[digitSpriteAddr[i]+2] = 0xF0;
                    memory[digitSpriteAddr[i]+3] = 0x10;
                    memory[digitSpriteAddr[i]+4] = 0xF0;
                    break;
                case 0x4:
                    memory[digitSpriteAddr[i]+0] = 0x90;
                    memory[digitSpriteAddr[i]+1] = 0x90;
                    memory[digitSpriteAddr[i]+2] = 0xF0;
                    memory[digitSpriteAddr[i]+3] = 0x10;
                    memory[digitSpriteAddr[i]+4] = 0x10;
                    break;
                case 0x5:
                    memory[digitSpriteAddr[i]+0] = 0xF0;
                    memory[digitSpriteAddr[i]+1] = 0x80;
                    memory[digitSpriteAddr[i]+2] = 0xF0;
                    memory[digitSpriteAddr[i]+3] = 0x10;
                    memory[digitSpriteAddr[i]+4] = 0xF0;
                    break;
                case 0x6:
                    memory[digitSpriteAddr[i]+0] = 0xF0;
                    memory[digitSpriteAddr[i]+1] = 0x80;
                    memory[digitSpriteAddr[i]+2] = 0xF0;
                    memory[digitSpriteAddr[i]+3] = 0x90;
                    memory[digitSpriteAddr[i]+4] = 0xF0;
                    break;
                case 0x7:
                    memory[digitSpriteAddr[i]+0] = 0xF0;
                    memory[digitSpriteAddr[i]+1] = 0x10;
                    memory[digitSpriteAddr[i]+2] = 0x20;
                    memory[digitSpriteAddr[i]+3] = 0x40;
                    memory[digitSpriteAddr[i]+4] = 0x40;
                    break;
                case 0x8:
                    memory[digitSpriteAddr[i]+0] = 0xF0;
                    memory[digitSpriteAddr[i]+1] = 0x90;
                    memory[digitSpriteAddr[i]+2] = 0xF0;
                    memory[digitSpriteAddr[i]+3] = 0x90;
                    memory[digitSpriteAddr[i]+4] = 0xF0;
                    break;
                case 0x9:
                    memory[digitSpriteAddr[i]+0] = 0xF0;
                    memory[digitSpriteAddr[i]+1] = 0x90;
                    memory[digitSpriteAddr[i]+2] = 0xF0;
                    memory[digitSpriteAddr[i]+3] = 0x10;
                    memory[digitSpriteAddr[i]+4] = 0xF0;
                    break;
                case 0xA:
                    memory[digitSpriteAddr[i]+0] = 0xF0;
                    memory[digitSpriteAddr[i]+1] = 0x90;
                    memory[digitSpriteAddr[i]+2] = 0xF0;
                    memory[digitSpriteAddr[i]+3] = 0x90;
                    memory[digitSpriteAddr[i]+4] = 0x90;
                    break;
                case 0xB:
                    memory[digitSpriteAddr[i]+0] = 0xE0;
                    memory[digitSpriteAddr[i]+1] = 0x90;
                    memory[digitSpriteAddr[i]+2] = 0xE0;
                    memory[digitSpriteAddr[i]+3] = 0x90;
                    memory[digitSpriteAddr[i]+4] = 0xE0;
                    break;
                case 0xC:
                    memory[digitSpriteAddr[i]+0] = 0xF0;
                    memory[digitSpriteAddr[i]+1] = 0x80;
                    memory[digitSpriteAddr[i]+2] = 0x80;
                    memory[digitSpriteAddr[i]+3] = 0x80;
                    memory[digitSpriteAddr[i]+4] = 0xF0;
                    break;
                case 0xD:
                    memory[digitSpriteAddr[i]+0] = 0xE0;
                    memory[digitSpriteAddr[i]+1] = 0x90;
                    memory[digitSpriteAddr[i]+2] = 0x90;
                    memory[digitSpriteAddr[i]+3] = 0x90;
                    memory[digitSpriteAddr[i]+4] = 0xE0;
                    break;
                case 0xE:
                    memory[digitSpriteAddr[i]+0] = 0xF0;
                    memory[digitSpriteAddr[i]+1] = 0x80;
                    memory[digitSpriteAddr[i]+2] = 0xF0;
                    memory[digitSpriteAddr[i]+3] = 0x80;
                    memory[digitSpriteAddr[i]+4] = 0xF0;
                    break;
                case 0xF:
                    memory[digitSpriteAddr[i]+0] = 0xF0;
                    memory[digitSpriteAddr[i]+1] = 0x80;
                    memory[digitSpriteAddr[i]+2] = 0xF0;
                    memory[digitSpriteAddr[i]+3] = 0x80;
                    memory[digitSpriteAddr[i]+4] = 0x80;
                    break;
                default:
                    printf("UNREACHABLE\n");
                    return 10;
                    break;
        }
    }
    
    

    printf("Setting regPC to %04X\n", programStartAddr);
    regPC = programStartAddr;

    printf("Running program\n");
    while(!WindowShouldClose()) {
        
        printf("%d\n", regSound);
        
        if (regSound > 0) {
            ResumeMusicStream(music);
        } else {
            PauseMusicStream(music);
        }
        
        UpdateMusicStream(music);
        
        for(uint32_t k = 0; k < instPerFrame; k++){
            timeElapsed += 1.0/(displayFPS*instPerFrame);
            timerTick += 1.0/(displayFPS*instPerFrame);
            if (timerTick > 1.0/60.0) {
                timerTick -= 1.0/60.0;
                if (regDelay > 0) {
                    regDelay -= 1;
                }
                if (regSound > 0) {
                    regSound -= 1;
                }
            }

            if (awaitingKeyPress) {
                for (int i = 0; i < 16; i++) {
                    if (IsKeyDown(keyMap[i])) {
                        pressedKey = i;
                        awaitingKeyPress = 0;
                    }
                }
            }

            uint16_t const inst = (memory[regPC] << 8) | memory[regPC+1];
            //printf("[%04X] %04X\n", regPC, inst);
            switch (inst>>(3*4)) {
                case 0x0: {
                    switch ((inst & 0x00FF)){
                        case 0xE0: {
                            memset(display, (uint64_t)0, HEIGHT*sizeof(uint64_t));
                            break;
                        }
                        case 0xEE: {
                            regPC = stack[regSP-1];
                            regSP -= 1;
                            break;
                        }
                        default:
                            printf("unknown inst: %04X\n", inst);
                            return 4;
                    }
                    break;
                }
                case 0x1: {
                    uint16_t nnn = (inst & 0x0FFF) >> 0;
                    regPC = nnn-2;
                    break;
                }
                case 0x2: {
                    uint16_t nnn = (inst & 0x0FFF) >> 0;
                    regSP += 1;
                    stack[regSP-1] = regPC;
                    regPC = nnn-2;
                    break;
                }
                case 0x3: {
                    uint8_t destReg = (inst & 0x0F00) >> 8;
                    uint8_t cmpVal  =  inst & 0x00FF;
                    if (regV[destReg] == cmpVal) {
                        regPC += 2;
                    }

                    break;
                }
                case 0x4: {
                    uint8_t destReg = (inst & 0x0F00) >> 8;
                    uint8_t cmpVal  =  inst & 0x00FF;
                    if (regV[destReg] != cmpVal) {
                        regPC += 2;
                    }

                    break;
                }
                case 0x6: {
                    uint8_t x  = (inst & 0x0F00) >> 8;
                    uint8_t kk  = (inst & 0x00FF) >> 0;
                    regV[x] = kk;

                    break;
                }
                case 0x7: {
                    uint8_t x  = (inst & 0x0F00) >> 8;
                    uint8_t kk  = (inst & 0x00FF) >> 0;
                    regV[x] = regV[x] + kk;

                    break;
                }
                case 0x8: {
                    uint8_t x  = (inst & 0x0F00) >> 8;
                    uint8_t y  = (inst & 0x00F0) >> 4;
                    switch ((inst & 0x000F)){
                        case 0x0: {
                            regV[x] = regV[y];
                            break;
                        }
                        case 0x2: {
                            regV[x] = regV[x] & regV[y];
                            break;
                        }
                        case 0x3: {
                            regV[x] = regV[x] ^ regV[y];
                            break;
                        }
                        case 0x4: {
                            uint16_t sum = (uint16_t)regV[x] + (uint16_t)regV[y];
                            if (sum > 255){
                                regV[0xF] = 1;
                                regV[x] = (uint8_t)(sum&0xFF);
                            } else {
                                regV[0xF] = 0;
                                regV[x] = (uint8_t)(sum&0xFF);
                            }
                            break;
                        }
                        case 0x5: {
                            if (regV[x] > regV[y]){
                                regV[0xF] = 1;
                                regV[x] = regV[x] - regV[y];
                            } else {
                                regV[0xF] = 0;
                                regV[x] = regV[y] - regV[x];
                            }
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
                    uint8_t destReg  = (inst & 0x0F00) >> 8;
                    uint8_t instMask =  inst & 0x00FF;
                    regV[destReg] = instMask & r;
                    break;
                }
                case 0xD: {
                    uint8_t Vx  = regV[(inst & 0x0F00) >> 8];
                    uint8_t Vy  = regV[(inst & 0x00F0) >> 4];
                    uint8_t n  = (inst & 0x000F) >> 0;
                    uint8_t collision = 0;
                    for (uint8_t lin = 0; lin < n; lin++) {
                        for (uint8_t col = 0; col < 8; col++) {
                            uint8_t memXshift = 7-col;
                            uint8_t pixel = (memory[regI+lin] >> memXshift) & 1;
                            uint8_t displayX = (Vx + col) % 64;
                            uint8_t displayXshift = 63-displayX;
                            uint8_t displayY = (Vy + lin) % 32;
                            uint8_t displayPixel = (((display[displayY])>>displayXshift)&1);
                            if (pixel){
                                if (displayPixel) {
                                    display[displayY] = display[displayY] & (~((uint64_t)1<<displayXshift));
                                    collision = 1;
                                }
                                else {
                                    display[displayY] = display[displayY] | ((uint64_t)1<<displayXshift);
                                }
                            }
                        }
                    }
                    regV[0xF] = collision;
                    break;
                }
                case 0xE:{
                    uint8_t x  = (inst & 0x0F00) >> 8;
                    switch ((inst & 0x00FF)){
                        case 0x9E: {
                            if (IsKeyDown(keyMap[regV[x]])){
                                regPC += 2;
                            }
                            break;
                        }
                        case 0xA1: {
                            if (!IsKeyDown(keyMap[regV[x]])){
                                regPC += 2;
                            }
                            break;
                        }
                        default:
                            printf("unknown inst: %04X\n", inst);
                            return 4;
                    }
                    break;
                }
                case 0xF:{
                    uint8_t x  = (inst & 0x0F00) >> 8;
                    switch ((inst & 0x00FF)){
                        case 0x07: {
                            regV[x] = regDelay;
                            break;
                        }
                        case 0x0A: {
                            if (pressedKey == 0x10){
                                awaitingKeyPress = 1;
                                regPC -= 2;
                            } else {
                                regV[x] = pressedKey;
                                pressedKey = 0x10;
                            }
                            break;
                        }
                        case 0x15: {
                            regDelay = regV[x];
                            break;
                        }
                        case 0x18: {
                            regSound = regV[x];
                            break;
                        }
                        case 0x1E: {
                            regI = regI + regV[x];
                            break;
                        }
                        case 0x29: {
                            regI = digitSpriteAddr[regV[x]];
                            break;
                        }
                        case 0x33: {
                            memory[regI] = (regV[x]/100)%10;
                            memory[regI+1] = (regV[x]/10)%10;
                            memory[regI+2] = regV[x]%10;
                            break;
                        }
                        case 0x55: {
                            for (int i = 0; i <= x; i++){
                                memory[regI+i] = regV[i];
                            }
                            break;
                        }
                        case 0x65: {
                            for (int i = 0; i <= x; i++){
                                regV[i] = memory[regI+i];
                            }
                            break;
                        }
                        default:
                            printf("unknown inst: %04X\n", inst);
                            return 4;
                    }
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
                uint8_t displayX = col;
                uint8_t displayXshift = 63-col;
                uint8_t displayY = lin;
                uint8_t displayPixel = (((display[displayY])>>displayXshift)&1);
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
