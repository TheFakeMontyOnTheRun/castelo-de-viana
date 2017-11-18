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
std::array<unsigned char, 320 * 200> buffer;
std::array<unsigned char, 320 * 100 / 4> evenBuffer;
std::array<unsigned char, 320 * 100 / 4> oddBuffer;

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

void plot(int x, int y, int color) {
    int b, m; /* bits and mask */
    unsigned char c;
    /* address section differs depending on odd/even scanline */
    bool odd = (1 == (y & 0x1));

    /* divide by 2 (each address section is 100 pixels) */
    y >>= 1;

    /* start bit (b) and mask (m) for 2-bit pixels */
    switch (x & 0x3) {
        case 0:
            b = 6;
            m = 0xC0;
            break;
        case 1:
            b = 4;
            m = 0x30;
            break;
        case 2:
            b = 2;
            m = 0x0C;
            break;
        case 3:
            b = 0;
            m = 0x03;
            break;
    }

    /* divide X by 4 (2 bits for each pixel) */
    x >>= 2;

    unsigned int offset = ((80 * y) + x);

    /* read current pixel */
    if (odd) {
        c = oddBuffer[offset];
    } else {
        c = evenBuffer[offset];
    }

    /* remove bits at new position */
    c = c & ~m;

    /* set bits at new position */
    c = c | (color << b);

    if (odd) {
        oddBuffer[offset] = c;
    } else {
        evenBuffer[offset] = c;
    }
}

void copyImageBufferToVideoMemory(const std::array<unsigned int, 320 * 200>& imageBuffer ) {

    int origin = 0;
    int value = 0;
    int last = 0;
    auto currentImageBufferPos = std::begin(imageBuffer);
    auto currentBufferPos = std::begin(buffer);

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

            if (0 < origin && origin < 4) {
                if (((x + y) % 2) == 0) {
                    value = 0;
                } else {
                    value = origin;
                }
            }

            if (4 <= origin && origin < 8) {
                value = origin - 4;
            }

            if (origin >= 8) {
                if (((x + y) % 2) == 0) {
                    value = 3;
                } else {
                    value = origin - 8;
                }
            }

            plot(x, y, value);
            *currentBufferPos = origin;

            currentBufferPos = std::next(currentBufferPos);
            currentImageBufferPos = std::next(currentImageBufferPos);
        }
    }

    dosmemput(evenBuffer.data(), 320 * 100 / 4, 0xB800 * 16);
    dosmemput(oddBuffer.data(), 320 * 100 / 4, (0xB800 * 16) + 0x2000);

    //gotoxy(1, 1);
    //std::cout << "room " << room << std::endl;
}

void initVideo() {
    union REGS regs;

    regs.h.ah = 0x00;
    regs.h.al = 0x4;
    int86(0x10, &regs, &regs);
}

std::string getResPath() {
    return "resCGA/";
}
