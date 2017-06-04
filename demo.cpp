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

std::vector<std::shared_ptr<odb::NativeBitmap>> tiles;


std::array<std::array<int, 10>,6> backgroundTiles;
std::array<std::array<int, 10>,6> foregroundTiles;

enum EDirection { kLeft, kRight };
enum EStance { kUp, kStanding, kClimbing };

std::shared_ptr<odb::NativeBitmap> hero[3][2] ={
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


int heroFrame = 0;
int px = 0;
int py = 0;
int vx = 0;
int vy = 0;
int counter = 0;
int room = 0;
EDirection playerDireciton = EDirection::kRight;
EStance playerStance = EStance::kStanding;

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
        c = oddBuffer[ offset ];
    } else {
        c = evenBuffer[ offset ];
    }

    /* remove bits at new position */
    c = c & ~m;

    /* set bits at new position */
    c = c | (color << b);

    if (odd) {
        oddBuffer[ offset ] = c;
    } else {
        evenBuffer[ offset ] = c;
    }
}

void copyImageBufferToVideoMemory() {

    int origin = 0;
    int value = 0;
    int last = 0;
    auto currentImageBufferPos = std::begin( imageBuffer );
    auto currentBufferPos = std::begin( buffer );

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

            if (last == origin ) {
                currentBufferPos = std::next( currentBufferPos );
                currentImageBufferPos = std::next( currentImageBufferPos );
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

            plot( x, y, value );
            *currentBufferPos = origin;

            currentBufferPos = std::next( currentBufferPos );
            currentImageBufferPos = std::next( currentImageBufferPos );
        }
    }

    dosmemput( evenBuffer.data(), 320 * 100 / 4, 0xB800 * 16 );
    dosmemput( oddBuffer.data(), 320 * 100 / 4, (0xB800 * 16) + 0x2000 );

    gotoxy(1,1);
    std::cout << "room " << room << std::endl;
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

            if (backgroundTiles[ty][tx] != 0 ) {
                tile = tiles[backgroundTiles[ty][tx]];

                if (tile == nullptr) {
                    std::cout << "null tile at " << tx << ", " << ty << std::endl;
                    exit(0);
                }

                pixelData = tile->getPixelData();

                pixel = 4;
                for (int y = y0; y < y1; ++y) {
                    if ( y  < 0 || y >= 200 ) {
                        continue;
                    }

                    for (int x = x0; x < x1; ++x) {

                        if ( x  < 0 || x >= 320 ) {
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

            if (foregroundTiles[ty][tx] != 0 ) {
                tile = tiles[foregroundTiles[ty][tx]];

                if (tile == nullptr) {
                    std::cout << "null tile at " << tx << ", " << ty << std::endl;
                    exit(0);
                }

                pixelData = tile->getPixelData();

                pixel = 4;
                for (int y = y0; y < y1; ++y) {

                    if ( y  < 0 || y >= 200 ) {
                        continue;
                    }

                    for (int x = x0; x < x1; ++x) {

                        if ( x  < 0 || x >= 320 ) {
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

    y0 = (py );
    y1 = 32 + y0;
    x0 = (px);
    x1 = 32 + x0;
    int *pixelData = hero[playerStance][heroFrame]->getPixelData();

    int pixel = 0;
    for (int y = y0; y < y1; ++y) {

        if ( y  < 0 || y >= 200 ) {
            continue;
        }

        for (int x = x0; x < x1; ++x) {
            if ( playerDireciton == EDirection::kRight ) {
                pixel = (pixelData[(32 * (y - y0)) + ((x - x0))]);
            } else {
                pixel = (pixelData[(32 * (y - y0)) + ( 31 - (x - x0))]);
            }

            if ( pixel == 0 ) {
                continue;
            }

            if ( x  < 0 || x >= 320 ) {
                continue;
            }

            imageBuffer[(320 * y) + (x)] = pixel;
        }
    }

    copyImageBufferToVideoMemory();
    usleep(20000);
}

void prepareRoom( int room ) {

    if ( room < 0 ) {
        std::cout << "room " << room << " is invalid " << std::endl;
        exit(0);
    }

    std::stringstream roomName;

    roomName = std::stringstream("");
    roomName << room;
    roomName << ".bg";
    std::ifstream bgmap(roomName.str());

    roomName = std::stringstream("");
    roomName << room;
    roomName << ".fg";

    std::ifstream fgmap(roomName.str());

    for (int y = 0; y < 6; ++y) {
        for (int x = 0; x < 10; ++x) {
            char ch = '0';

            bgmap >> ch;
            backgroundTiles[y][x] = ch - '0';

            fgmap >> ch;
            foregroundTiles[y][x] = ch - '0';

        }
    }

    roomName = std::stringstream("");
    roomName << room;
    roomName << ".lst";

    std::ifstream tileList(roomName.str());
    std::string buffer;
    tiles.clear();

    while ( tileList.good() ) {
        std::getline( tileList, buffer );
        tiles.push_back(odb::loadBitmap(buffer));
    }

    std::fill(std::begin(imageBuffer), std::end(imageBuffer), 4);
    std::fill(std::begin(buffer), std::end(buffer), 4);
}

void enforceScreenLimits() {
    if (px < 0) {
     if ( (room % 10) > 0 ) {
            px = 320 - 32 - 1;
            prepareRoom(--room);
        } else {
            px = 0;
        }
    }

    if (py < 0) {
        if ( (room / 10) <= 9 ) {
            py = 200 - 32 - 1;
            room += 10;
            prepareRoom(room);
        } else {
            py = 0;
        }
    }

    if ((px + 32) >= 320) {
        if ( (room % 10) < 9 ) {
            px = 1;
            prepareRoom(++room);
        } else {
            px = 320 - 32;
        }
    }

    if ((py + 32 ) >= 200 ) {
        if ( (room / 10) >= 1 ) {
            py = 1;
            room -= 10;
            prepareRoom(room);
        } else {
            py = 200 - 32 - 1;
        }
    }
}

int main(int argc, char **argv) {

    prepareRoom(0);

    bool done = false;

    char lastKey = 0;

    initMode4h();

    while (!done) {

        px += vx;
        py += vy;

        if ( vx == 1 ) {
            vx = 0;
        }

        if ( vy == 1 ) {
            vy = 0;
        }

        enforceScreenLimits();
        bool isOnGround = false;
        bool isOnStairs = ( foregroundTiles[ (py + 16 ) / 32 ][ ( px + 16 ) / 32 ] == 3 );
        bool isJumping = false;
        bool isUpPressed = false;
        bool isDownPressed = false;
        bool isLeftPressed = false;
        bool isRightPressed = false;
        bool isAttacking = false;

        int ground = ( (py + 32) / 32 );
        int front = ( (px ) / 32 );

        if ( vx > 0 ) {
            front++;
        }

        if ( vx < 0 ) {
            front--;
        }



        if ( ground > 5 ) {
            ground = 5;
        }

        if ( foregroundTiles[ ground ][ (px + 16) / 32 ] == 1 ) {
            isOnGround = true;
        }

        if ( foregroundTiles[ (py/32) ][ front ] == 1 ) {
            vx = 0;
        }

        if ( (vx != 0 && isOnGround) || (vy != 0 && isOnStairs) ) {
            heroFrame = ( heroFrame + 1) % 2;
        }

        vx = vx / 2;

        if ( isOnGround ) {
            vy = 0;
            py = ( py / 32 ) * 32;
        }


        if ( !isOnStairs) {
            vy = vy + 2;
            playerStance = EStance::kStanding;
        } else {
            vy = 0;
        }


        int level = 0;
        ++counter;
        render();

        lastKey = bioskey(0x11);
        auto extendedKeys = bioskey(0x12);

        if ( extendedKeys & ( 0b0000000000000100) ||
             extendedKeys & ( 0b0000000100000000)
                ) {
            isJumping = true;
        }

        if ( extendedKeys & ( 0b0000000000001000) ||
             extendedKeys & ( 0b0000001000000000)
                ) {
            isAttacking = true;
        }


        bdos (0xC, 0, 0) ;

        gotoxy( 1,2);
            std::cout << "key: " << lastKey << std::endl;

            switch (lastKey) {
                case 27:
                    done = true;
                    break;
                case 'q':
                    isJumping = true;
                    break;
                case 'w':
                    isUpPressed = true;
                    break;
                case 's':
                    isDownPressed = true;
                    break;
                case 'a':
                    isLeftPressed = true;
                    break;
                case 'd':
                    isRightPressed = true;
                    break;
            }






        if ( isJumping ) {
            if (isOnGround) {
                vy = -12;
            }
            playerStance = EStance::kStanding;
        }

        if ( isUpPressed ) {
            if (isOnStairs) {
                vy = -8;
                playerStance = EStance::kClimbing;
            } else if (isOnGround) {
                playerStance = EStance::kUp;
            }
        }

        if ( isDownPressed ) {
            if (isOnStairs) {
                vy = +8;
                playerStance = EStance::kClimbing;
            } else {
                playerStance = EStance::kStanding;
            }
        }

        if ( isLeftPressed ) {
            vx = -8;
            playerDireciton = EDirection::kLeft;
            if (isOnGround) {
                playerStance = EStance::kStanding;
            }
        }

        if (isRightPressed ) {
            vx = +8;
            playerDireciton = EDirection::kRight;
            if (isOnGround) {
                playerStance = EStance::kStanding;
            }
        }




        }

    return 0;
}
