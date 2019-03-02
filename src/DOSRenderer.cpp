//
// Created by monty on 01-07-2017.
//
#include <array>
#include <go32.h>
#include <sys/farptr.h>
#include <conio.h>
#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <bios.h>
#include <time.h>
#include <unistd.h>

#include "Renderer.h"

clock_t timeRendering = 0;
int desiredTimeSlice = 75;

clock_t t0;
clock_t t1;
clock_t ms;

std::array<uint8_t, 320 * 100 / 4> evenBuffer;
std::array<uint8_t, 320 * 100 / 4> oddBuffer;

EVideoType videoType = kCGA;

ControlState getControlState() {
    ControlState toReturn;

    while (kbhit()) {
        auto getched = getch();
        switch (getched) {
            case 27:
                toReturn.escape = true;
                break;
            case 'z':
                toReturn.jump = true;
                break;
            case 'x':
                toReturn.fireArrow = true;
                break;
            case 'c':
            case ' ':
                toReturn.sword= true;
                break;
            case 13:
                toReturn.enter = true;
                break;
            case 224:
            case 0:
                auto arrow = getch();
                switch(arrow) {
                    case 75:
                        toReturn.moveLeft = true;
                        break;
                    case 72:
                        toReturn.moveUp = true;
                        break;
                    case 77:
                        toReturn.moveRight = true;
                        break;
                    case 80:
                        toReturn.moveDown = true;
                        break;
                }
                break;

        }
    }
    bdos(0xC, 0, 0);
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

void onQuit() {
    stopSounds();
    textmode(C80);
    clrscr();
    puts("Thanks for playing!\n\r\n");
}


uint8_t getPaletteEntry( uint32_t origin ) {
    uint8_t shade = 0;

    shade += (((((origin & 0x0000FF)      ) << 2) >> 8)) << 6;
    shade += (((((origin & 0x00FF00)  >> 8) << 3) >> 8)) << 3;
    shade += (((((origin & 0xFF0000) >> 16) << 3) >> 8)) << 0;

    return shade;
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

int frame = 0;

void copyImageBufferToVideoMemory(const std::array<uint8_t, 320 * 200>& imageBuffer ) {
    if ( videoType == kVGA ) {
        std::array<uint8_t , 320 * 200> mFinalBuffer;
        auto currentImageBufferPos = std::begin(imageBuffer);
        auto currentBufferPos = std::begin(mFinalBuffer);

        std::fill( std::begin(mFinalBuffer), std::end(mFinalBuffer), 0);

        for (int y = 0; y < 200; ++y) {
            for (int x = 0; x < 320; ++x) {
                *currentBufferPos++ = *currentImageBufferPos++;
            }
        }
        dosmemput(&mFinalBuffer[0], 64000, 0xa0000);
    } else {
        int origin = 0;
        int value = 0;
        int last = 0;
        auto currentImageBufferPos = std::begin(imageBuffer);

        for (int y = 0; y < 200; ++y) {
            for (int x = 0; x < 320; ++x) {

                origin = *currentImageBufferPos;

                if (last == origin) {
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

                currentImageBufferPos = std::next(currentImageBufferPos);
            }
        }

        dosmemput(evenBuffer.data(), 320 * 100 / 4, 0xB800 * 16);
        dosmemput(oddBuffer.data(), 320 * 100 / 4, (0xB800 * 16) + 0x2000);
    }
}

void initVideoFor( EVideoType videoType ) {

    if ( videoType == kVGA ) {
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
    } else {
        union REGS regs;

        regs.h.ah = 0x00;
        regs.h.al = 0x4;
        int86(0x10, &regs, &regs);
    }
}

const char* getAssetsPath() {
    if ( videoType == kVGA ) {
        return "vga.pfs";
    } else {
        return "cga.pfs";
    }
}
