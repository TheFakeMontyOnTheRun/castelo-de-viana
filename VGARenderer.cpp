//
// Created by monty on 01-07-2017.
//
#include <array>
#include "Renderer.h"

#include <go32.h>
#include <sys/farptr.h>
#include <conio.h>
#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <bios.h>
#include <time.h>
#include <unistd.h>


double timeRendering = 0;
int desiredTimeSlice = 75;
std::array<unsigned char, 320 * 200> mFinalBuffer;
std::array<unsigned int, 320 * 200> mBuffer;
double t0;
double t1;
double ms;

ControlState getControlState() {
    ControlState toReturn;

    auto extendedKeys = bioskey(0x12);

    if (extendedKeys & (0b0000000000000100) ||
        extendedKeys & (0b0000000100000000)
            ) {
        toReturn.sword = true;
    }

    if (extendedKeys & (0b0000000000000001) ||
        extendedKeys & (0b0000000000000010)
            ) {
        toReturn.jump = true;
    }

    auto lastKey = bioskey(0x11);
    bdos(0xC, 0, 0);

    switch (lastKey) {
        case 9836:
            toReturn.secret = true;
            break;
        case 27:
        case 283:
            toReturn.escape = true;
            break;
        case 'q':
            toReturn.jump = true;
            break;
        case 'w':
        case 4471:
        case 18656:
            toReturn.moveUp = true;
            break;
        case 's':
        case 8051:
        case 20704:
            toReturn.moveDown = true;
            break;
        case 'a':
        case 7777:
        case 19424:
            toReturn.moveLeft = true;
            break;
        case 'd':
        case 8292:
        case 19936:
            toReturn.moveRight = true;
            break;
        case ' ':
        case 3849:
        case 14624:
            toReturn.fireArrow = true;
            break;
        case 7181:
            toReturn.enter = true;
            break;
    }


    return toReturn;
}

void beginFrame() {
    t0 = uclock();
}

void doneWithFrame() {
    t1 = uclock();
    ms = (1000 * (t1 - t0)) / UCLOCKS_PER_SEC;
    timeRendering += ms;

    if (ms < desiredTimeSlice) {
        usleep((desiredTimeSlice - ms) * 1000);
    } else {
        ++desiredTimeSlice;
    }
}

void soundFrequency(int frequency) {
    sound( frequency );
}

void muteSound() {
    nosound();
}

void onQuit() {
    nosound();
    textmode(C80);
    clrscr();
//    std::cout << "Thanks for playing!" << std::endl;
}


unsigned char getPaletteEntry( int origin ) {
    unsigned char shade = 0;
    shade += (((((origin & 0x0000FF)      ) << 2  ) >> 8 ) ) << 6;
    shade += (((((origin & 0x00FF00) >> 8 ) << 3  ) >> 8 ) ) << 3;
    shade += (((((origin & 0xFF0000) >> 16) << 3  ) >> 8 ) ) << 0;
    return shade;
}

void plot(int x, int y, int color) {
    mBuffer[ (320 * y) + x ] = color;
}

int frame = 0;

void copyImageBufferToVideoMemory(const std::array<unsigned int, 320 * 200>& imageBuffer ) {

    int origin = 0;
    int value = 0;
    int last = 0;
    auto currentImageBufferPos = std::begin(imageBuffer);
    auto currentBufferPos = std::begin(mFinalBuffer);

    std::fill( std::begin(mFinalBuffer), std::end(mFinalBuffer), 0);

    for (int y = 0; y < 200; ++y) {

        if (y < 0 || y >= 200) {
            continue;
        }

        for (int x = 0; x < 320; ++x) {

            if (x < 0 || x >= 320) {
                continue;
            }

            origin = *currentImageBufferPos;
            last = *currentBufferPos;

            if (last == origin) {
                currentBufferPos = std::next(currentBufferPos);
                currentImageBufferPos = std::next(currentImageBufferPos);
                continue;
            }

            value = origin;

            *currentBufferPos = getPaletteEntry(value);

            currentBufferPos = std::next(currentBufferPos);
            currentImageBufferPos = std::next(currentImageBufferPos);
        }
    }

    dosmemput(&mFinalBuffer[0], 64000, 0xa0000);
}

void initVideo() {
    __dpmi_regs reg;

    reg.x.ax = 0x13;
    __dpmi_int(0x10, &reg);

    outp(0x03c8, 0);

    for ( int r = 0; r < 4; ++r ) {
        for ( int g = 0; g < 8; ++g ) {
            for ( int b = 0; b < 8; ++b ) {
                outp(0x03c9, (r * (21) ) );
                outp(0x03c9, (g * (8) ) );
                outp(0x03c9, (b * (8) ) );
            }
        }
    }
}

std::string getResPath() {
    return "resVGA/";
}
