#ifndef GAME_H
#define GAME_H


enum EDirection {
    kLeft, kRight
};
enum EStance {
    kUp, kStanding, kClimbing, kAttacking, kJumping
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
    EDirection mDirection = EDirection::kRight;
    EStance mStance = EStance::kStanding;;
};

extern Actor player;
extern int counter;
extern int room;
extern int heroFrame;
extern std::array<std::array<int, 10>, 6> backgroundTiles;
extern std::array<std::array<int, 10>, 6> foregroundTiles;
extern std::vector<Actor> foes;

extern void clearBuffers();
void init();
void gameTick(bool &isOnGround, bool &isOnStairs);
void loadTiles( std::vector<std::string> tilesToLoad );
void updateHero(bool isOnGround, bool isJumping, bool isUpPressed, bool isDownPressed, bool isLeftPressed, bool isAttacking, bool isUsingSpecial,
                bool isRightPressed, bool isOnStairs);
void prepareRoom(int room);
void enforceScreenLimits();
#endif
