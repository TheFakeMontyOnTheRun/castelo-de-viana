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
int px = 0;
int py = 0;
int vx = 0;
int vy = 0;
int counter = 0;
int room = 0;
EDirection playerDirection = EDirection::kRight;
EStance playerStance = EStance::kStanding;

std::array<std::array<int, 10>, 6> backgroundTiles;
std::array<std::array<int, 10>, 6> foregroundTiles;

void init() {
    px = 0;
    py = 0;
    vx = 0;
    vy = 0;
    counter = 0;
    room = 0;
    playerDirection = EDirection::kRight;
    playerStance = EStance::kStanding;
}

void updateHero(bool isOnGround, bool isJumping, bool isUpPressed, bool isDownPressed, bool isLeftPressed,
                bool isRightPressed, bool isOnStairs) {
    if (isJumping) {
        if (isOnGround) {
            vy = -12;
        }
        playerStance = kStanding;
    }

    if (isUpPressed) {
        if (isOnStairs) {
            vy = -8;
            playerStance = kClimbing;
        } else if (isOnGround) {
            playerStance = kUp;
        }
    }

    if (isDownPressed) {
        if (isOnStairs) {
            vy = +8;
            playerStance = kClimbing;
        } else {
            playerStance = kStanding;
        }
    }

    if (isLeftPressed) {
        vx = -8;
        playerDirection = kLeft;
        if (isOnGround) {
            playerStance = kStanding;
        }
    }

    if (isRightPressed) {
        vx = +8;
        playerDirection = kRight;
        if (isOnGround) {
            playerStance = kStanding;
        }
    }
}

void gameTick(bool &isOnGround, bool &isOnStairs) {
    isOnStairs= (foregroundTiles[(py + 16) / 32][(px + 16) / 32] == 3);

    if (playerDirection == EDirection::kRight) {
        if (foregroundTiles[((py + 16) / 32)][ ( px + 32 ) / 32 ] == 1) {
            vx = 0;
        }
    }

    if (playerDirection == EDirection::kLeft) {
        if (foregroundTiles[((py + 16) / 32)][ ( px ) / 32 ] == 1) {
            vx = 0;
        }
    }

    px += vx;
    py += vy;

    if (vx == 1) {
        vx = 0;
    }

    if (vy == 1) {
        vy = 0;
    }

    enforceScreenLimits();

    if ( vx == 0 ) {
        heroFrame = 0;
    }

    int ground = ((py + 32) / 32);
    int ceiling = ( py) / 32;

    if (ground > 5) {
        ground = 5;
    }

    if (ceiling < 0) {
        ceiling = 0;
    }


    if (foregroundTiles[ground][(px + 16) / 32] == 1) {
        isOnGround = true;
    }


    if ((vx != 0 && isOnGround) || (vy != 0 && isOnStairs)) {
        heroFrame = (heroFrame + 1) % 2;
    }

    vx = vx / 2;

    if (isOnGround) {
        vy = 0;
        py = (py / 32) * 32;
    }

    if ( vy < 0 && foregroundTiles[ceiling][(px + 16) / 32] == 1) {
        vy = -vy;
    }

    if ( vy < 0 && foregroundTiles[py / 32][(px + 16) / 32] == 1) {
        vy = -vy;
    }




    if (!isOnStairs) {
        vy = vy + 2;
        //this prevents from jumping while keeping the climbing animation state. Unfortunately, prevents looking up.
        //playerStance = EStance::kStanding;
    } else {
        vy = 0;
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
            foregroundTiles[y][x] = ch - '0';

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
    if (px < 0) {
        if ((room % 10) > 0) {
            px = 320 - 32 - 1;
            prepareRoom(--room);
        } else {
            px = 0;
        }
    }

    if (py < 0) {
        if ((room / 10) <= 9) {
            py = 200 - 32 - 1;
            room += 10;
            prepareRoom(room);
        } else {
            py = 0;
        }
    }

    if ((px + 32) >= 320) {
        if ((room % 10) < 9) {
            px = 1;
            prepareRoom(++room);
        } else {
            px = 320 - 32;
        }
    }

    if ((py + 32) >= 200) {
        if ((room / 10) >= 1) {
            py = 1;
            room -= 10;
            prepareRoom(room);
        } else {
            py = 200 - 32 - 1;
        }
    }
}
