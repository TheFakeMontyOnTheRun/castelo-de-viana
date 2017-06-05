#include <go32.h>
#include <sys/farptr.h>
#include <conio.h>
#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <bios.h>
#include <algorithm>
#include <array>
#include <random>
#include <iostream>
#include <time.h>
#include <unistd.h>
#include <memory>
#include <fstream>
#include <sstream>
#include "NativeBitmap.h"
#include "LoadImage.h"

#include "Game.h"


std::vector<std::shared_ptr<odb::NativeBitmap>> tiles;


std::shared_ptr<odb::NativeBitmap> hero[3][2] = {
        {
                odb::loadBitmap("up0.png"),
                odb::loadBitmap("up1.png"),
        },
        {
                odb::loadBitmap("hero0.png"),
                odb::loadBitmap("hero1.png"),
        },
        {
                odb::loadBitmap("down0.png"),
                odb::loadBitmap("down1.png"),
        },
};

std::array<unsigned int, 320 * 200> imageBuffer;
std::array<unsigned char, 320 * 200> buffer;
std::array<unsigned char, 320 * 100 / 4> evenBuffer;
std::array<unsigned char, 320 * 100 / 4> oddBuffer;


void initMode4h() {
    union REGS regs;

    regs.h.ah = 0x00;
    regs.h.al = 0x4;
    int86(0x10, &regs, &regs);
}

void clearBuffers() {
    std::fill(std::begin(imageBuffer), std::end(imageBuffer), 4);
    std::fill(std::begin(buffer), std::end(buffer), 4);
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

void copyImageBufferToVideoMemory() {

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

void render() {
    std::fill(std::begin(imageBuffer), std::end(imageBuffer), 4);

    int y0 = 0;
    int y1 = 0;
    int x0 = 0;
    int x1 = 0;


    for (int ty = 0; ty < 6; ++ty) {
        for (int tx = 0; tx < 10; ++tx) {
            std::shared_ptr<odb::NativeBitmap> tile;
            int *pixelData;
            y0 = (ty * 32);
            y1 = 32 + (ty * 32);
            x0 = (tx * 32);
            x1 = 32 + (tx * 32);
            int pixel = 4;

            if (backgroundTiles[ty][tx] != 0) {
                tile = tiles[backgroundTiles[ty][tx]];

                if (tile == nullptr) {
                    std::cout << "null tile at " << tx << ", " << ty << std::endl;
                    exit(0);
                }

                pixelData = tile->getPixelData();

                pixel = 4;
                for (int y = y0; y < y1; ++y) {
                    if (y < 0 || y >= 200) {
                        continue;
                    }

                    for (int x = x0; x < x1; ++x) {

                        if (x < 0 || x >= 320) {
                            continue;
                        }

                        pixel = (pixelData[(32 * (y - y0)) + (x - x0)]);

                        if (pixel == 0) {
                            continue;
                        }

                        imageBuffer[(320 * y) + x] = pixel;
                    }
                }
            }

            if (foregroundTiles[ty][tx] != 0) {
                tile = tiles[foregroundTiles[ty][tx]];

                if (tile == nullptr) {
                    std::cout << "null tile at " << tx << ", " << ty << std::endl;
                    exit(0);
                }

                pixelData = tile->getPixelData();

                pixel = 4;
                for (int y = y0; y < y1; ++y) {

                    if (y < 0 || y >= 200) {
                        continue;
                    }

                    for (int x = x0; x < x1; ++x) {

                        if (x < 0 || x >= 320) {
                            continue;
                        }

                        pixel = (pixelData[(32 * (y - y0)) + (x - x0)]);

                        if (pixel == 0) {
                            continue;
                        }

                        imageBuffer[(320 * y) + x] = pixel;
                    }
                }
            }
        }
    }

    y0 = (py);
    y1 = 32 + y0;
    x0 = (px);
    x1 = 32 + x0;
    int *pixelData = hero[playerStance][heroFrame]->getPixelData();

    int pixel = 0;
    for (int y = y0; y < y1; ++y) {

        if (y < 0 || y >= 200) {
            continue;
        }

        for (int x = x0; x < x1; ++x) {
            if (playerDirection == EDirection::kRight) {
                pixel = (pixelData[(32 * (y - y0)) + ((x - x0))]);
            } else {
                pixel = (pixelData[(32 * (y - y0)) + (31 - (x - x0))]);
            }

            if (pixel == 0) {
                continue;
            }

            if (x < 0 || x >= 320) {
                continue;
            }

            imageBuffer[(320 * y) + (x)] = pixel;
        }
    }

    copyImageBufferToVideoMemory();
    usleep(20000);
}


int main(int argc, char **argv) {
    init();
    prepareRoom(0);

    bool done = false;

    int lastKey = 0;

    initMode4h();

    while (!done) {

        bool isOnGround = false;
        bool isJumping = false;
        bool isUpPressed = false;
        bool isDownPressed = false;
        bool isLeftPressed = false;
        bool isRightPressed = false;
        bool isAttacking = false;
        bool isAltAttackPressed = false;

        bool isOnStairs;
        gameTick(isOnGround, isOnStairs);
        render();

        lastKey = bioskey(0x11);
        auto extendedKeys = bioskey(0x12);

        if (extendedKeys & (0b0000000000000100) ||
            extendedKeys & (0b0000000100000000)
                ) {
            isAttacking = true;
        }

        if (extendedKeys & (0b0000000000000001) ||
            extendedKeys & (0b0000000000000010)
                ) {
            isJumping = true;
        }

        bdos(0xC, 0, 0);
        //gotoxy(1, 2);

        //std::cout << "key: " << lastKey << std::endl;

        switch (lastKey) {
            case 27:
            case 283:
                done = true;
                break;
            case 'q':
                isJumping = true;
                break;
            case 'w':
            case 4471:
            case 18656:
                isUpPressed = true;
                break;
            case 's':
            case 8051:
            case 20704:
                isDownPressed = true;
                break;
            case 'a':
            case 7777:
            case 19424:
                isLeftPressed = true;
                break;
            case 'd':
            case 8292:
            case 19936:
                isRightPressed = true;
                break;
            case ' ':
            case 14624:
                isAltAttackPressed = true;
                gotoxy( 1,3 );
                std::cout << "SPECIAL!" << std::endl;
        }


        updateHero(isOnGround, isJumping, isUpPressed, isDownPressed, isLeftPressed, isRightPressed, isOnStairs);

    }

    return 0;
}