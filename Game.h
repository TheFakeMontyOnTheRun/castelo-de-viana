#ifndef GAME_H
#define GAME_H


enum EDirection {
    kLeft, kRight
};
enum EStance {
    kUp, kStanding, kClimbing
};

extern int px;
extern int py;
extern int vx;
extern int vy;
extern int counter;
extern int room;
extern EDirection playerDirection;
extern EStance playerStance;

extern std::array<std::array<int, 10>, 6> backgroundTiles;
extern std::array<std::array<int, 10>, 6> foregroundTiles;

void init();
#endif
