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

int px = 0;
int py = 0;
int vx = 0;
int vy = 0;
int counter = 0;
int room = 0;
EDirection playerDireciton = EDirection::kRight;
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
    playerDireciton = EDirection::kRight;
    playerStance = EStance::kStanding;
}