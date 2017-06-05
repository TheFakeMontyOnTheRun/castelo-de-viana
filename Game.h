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
extern int heroFrame;
extern std::array<std::array<int, 10>, 6> backgroundTiles;
extern std::array<std::array<int, 10>, 6> foregroundTiles;
extern std::vector<std::shared_ptr<odb::NativeBitmap>> tiles;
extern void clearBuffers();
void init();
void gameTick(bool &isOnGround, bool &isOnStairs);

void updateHero(bool isOnGround, bool isJumping, bool isUpPressed, bool isDownPressed, bool isLeftPressed,
                bool isRightPressed, bool isOnStairs);
void prepareRoom(int room);
void enforceScreenLimits();
#endif
