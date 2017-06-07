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

#include "Game.h"

int heroFrame = 0;
Actor player;
int counter = 0;
int room = 0;

std::array<std::array<int, 10>, 6> backgroundTiles;
std::array<std::array<int, 10>, 6> foregroundTiles;
std::vector<Actor> foes;

void init() {
    player.mPosition.mX = 0;
    player.mPosition.mY = 0;
    player.mSpeed.mX = 0;
    player.mSpeed.mY = 0;
    counter = 0;
    room = 0;
}

void updateHero(bool isOnGround, bool isJumping, bool isUpPressed, bool isDownPressed, bool isLeftPressed,
                bool isRightPressed, bool isOnStairs) {
    if (isJumping) {
        if (isOnGround) {
            player.mSpeed.mY = -12;
        }
        player.mStance = kStanding;
    }

    if (isUpPressed) {
        if (isOnStairs) {
            player.mSpeed.mY = -8;
            player.mStance = kClimbing;
        } else if (isOnGround) {
            player.mStance = kUp;
        }
    }

    if (isDownPressed) {
        if (isOnStairs) {
            player.mSpeed.mY = +8;
            player.mStance = kClimbing;
        } else {
            player.mStance = kStanding;
        }
    }

    if (isLeftPressed) {
        player.mSpeed.mX = -8;
        player.mDirection = kLeft;
        if (isOnGround) {
            player.mStance = kStanding;
        }
    }

    if (isRightPressed) {
        player.mSpeed.mX = +8;
        player.mDirection = kRight;
        if (isOnGround) {
            player.mStance = kStanding;
        }
    }
}

void gameTick(bool &isOnGround, bool &isOnStairs) {
    isOnStairs= (foregroundTiles[(player.mPosition.mY + 16) / 32][(player.mPosition.mX + 16) / 32] == 3);

    if (player.mDirection == EDirection::kRight) {
        if (foregroundTiles[((player.mPosition.mY + 16) / 32)][ ( player.mPosition.mX + 32 ) / 32 ] == 1) {
            player.mSpeed.mX = 0;
        }
    }

    if (player.mDirection == EDirection::kLeft) {
        if (foregroundTiles[((player.mPosition.mY + 16) / 32)][ ( player.mPosition.mX ) / 32 ] == 1) {
            player.mSpeed.mX = 0;
        }
    }

    player.mPosition.mX += player.mSpeed.mX;
    player.mPosition.mY += player.mSpeed.mY;

    if (player.mSpeed.mX == 1) {
        player.mSpeed.mX = 0;
    }

    if (player.mSpeed.mY == 1) {
        player.mSpeed.mY = 0;
    }

    enforceScreenLimits();

    if ( player.mSpeed.mX == 0 ) {
        heroFrame = 0;
    }

    int ground = ((player.mPosition.mY + 32) / 32);
    int ceiling = ( player.mPosition.mY) / 32;

    if (ground > 5) {
        ground = 5;
    }

    if (ceiling < 0) {
        ceiling = 0;
    }


    if (foregroundTiles[ground][(player.mPosition.mX + 16) / 32] == 1) {
        isOnGround = true;
    }


    if ((vx != 0 && isOnGround) || (vy != 0 && isOnStairs)) {
        heroFrame = (heroFrame + 1) % 2;
    }

    player.mSpeed.mX = player.mSpeed.mX / 2;

    if ((isOnGround || foregroundTiles[ground][(player.mPosition.mX + 16) / 32] == 3) && !isOnStairs ) {
        player.mSpeed.mY = std::min( 0, player.mSpeed.mY );
        player.mPosition.mY = std::min( player.mPosition.mY, (player.mPosition.mY / 32) * 32 );
    }

    if ( player.mSpeed.mY < 0 && foregroundTiles[ceiling][(player.mPosition.mX + 16) / 32] == 1) {
        player.mSpeed.mY = -player.mSpeed.mY;
    }

    if ( player.mSpeed.mY < 0 && foregroundTiles[player.mPosition.mY / 32][(player.mPosition.mX + 16) / 32] == 1) {
        player.mSpeed.mY = -player.mSpeed.mY;
    }




    if (!isOnStairs) {
        player.mSpeed.mY = player.mSpeed.mY + 2;
        //this prevents from jumping while keeping the climbing animation state. Unfortunately, prevents looking up.
        //playerStance = EStance::kStanding;
    } else {
        player.mSpeed.mY = 0;
    }

    int level = 0;
    ++counter;


}

void prepareRoom(int room) {

    if (room < 0) {
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


            if ( ch == 'a' ) {
                foregroundTiles[y][x] = 0;
                Actor a;
                a.mPosition = Vec2i{ x * 32, y * 32 };
                foes.push_back(a);
            } else {
                foregroundTiles[y][x] = ch - '0';
            }



        }
    }

    roomName = std::stringstream("");
    roomName << room;
    roomName << ".lst";

    std::ifstream tileList(roomName.str());
    std::string buffer;

    std::vector<std::string> tilesToLoad;

    while (tileList.good()) {
        std::getline(tileList, buffer);
        tilesToLoad.push_back(buffer);
    }

    loadTiles( tilesToLoad );

    clearBuffers();
}

void enforceScreenLimits() {
    if (player.mPosition.mX < 0) {
        if ((room % 10) > 0) {
            player.mPosition.mX = 320 - 32 - 1;
            prepareRoom(--room);
        } else {
            player.mPosition.mX = 0;
        }
    }

    if (player.mPosition.mY < 0) {
        if ((room / 10) <= 9) {
            player.mPosition.mY = 200 - 32 - 1;
            room += 10;
            prepareRoom(room);
        } else {
            player.mPosition.mY = 0;
        }
    }

    if ((player.mPosition.mX + 32) >= 320) {
        if ((room % 10) < 9) {
            player.mPosition.mX = 1;
            prepareRoom(++room);
        } else {
            player.mPosition.mX = 320 - 32;
        }
    }

    if ((player.mPosition.mY + 32) >= 200) {
        if ((room / 10) >= 1) {
            player.mPosition.mY = 1;
            room -= 10;
            prepareRoom(room);
        } else {
            player.mPosition.mY = 200 - 32 - 1;
        }
    }
}
