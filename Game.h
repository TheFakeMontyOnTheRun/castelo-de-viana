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

class Actor {
public:
    Vec2i mPosition;
    Vec2i mSpeed;
};

extern Actor player;
extern int counter;
extern int room;
extern EDirection playerDirection;
extern EStance playerStance;
extern int heroFrame;
extern std::array<std::array<int, 10>, 6> backgroundTiles;
extern std::array<std::array<int, 10>, 6> foregroundTiles;
extern std::vector<Actor> foes;

extern void clearBuffers();
void init();
void gameTick(bool &isOnGround, bool &isOnStairs);
void loadTiles( std::vector<std::string> tilesToLoad );
void updateHero(bool isOnGround, bool isJumping, bool isUpPressed, bool isDownPressed, bool isLeftPressed,
                bool isRightPressed, bool isOnStairs);
void prepareRoom(int room);
void enforceScreenLimits();
#endif
