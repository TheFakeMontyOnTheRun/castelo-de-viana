#ifndef GAME_H
#define GAME_H


enum EDirection {
    kLeft, kRight
};
enum EStance {
    kUp, kStanding, kClimbing
};

class Vec2i {
public:
    int mX = 0;
    int mY = 0;
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
extern std::vector<Vec2i> foes;

extern void clearBuffers();
void init();
void gameTick(bool &isOnGround, bool &isOnStairs);
void loadTiles( std::vector<std::string> tilesToLoad );
void updateHero(bool isOnGround, bool isJumping, bool isUpPressed, bool isDownPressed, bool isLeftPressed,
                bool isRightPressed, bool isOnStairs);
void prepareRoom(int room);
void enforceScreenLimits();
#endif
