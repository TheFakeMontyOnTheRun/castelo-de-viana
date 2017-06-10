#ifndef GAME_H
#define GAME_H


enum EDirection {
    kLeft, kRight
};
enum EStance {
    kUp, kStanding, kClimbing, kAttacking, kJumping, kAltAttacking
};

enum EItemType {
    kMeat, kKey
};

enum EActorType {
    kPlayer, kSkeleton, kClosedDoor, kOpenDoor, kArrow, kGargoyle, kCapiroto
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
    EActorType mType;
    EDirection mDirection = EDirection::kRight;
    EStance mStance = EStance::kStanding;
    int mHealth = 0;
};

class Item {
public:
    Vec2i mPosition;
    EItemType mType;
};

bool operator==( const Vec2i& a, const Vec2i& b );
bool operator==( const Item& a, const Item& b );
bool operator==( const Actor& a, const Actor& b );

extern Actor player;
extern bool hasKey;
extern bool hasBossOnScreen;
extern int counter;
extern int room;
extern bool paused;
extern int heroFrame;
extern int ticksUntilVulnerable;
extern int ticksToShowHealth;
extern std::array<std::array<int, 10>, 6> backgroundTiles;
extern std::array<std::array<int, 10>, 6> foregroundTiles;
extern std::vector<Actor> foes;
extern std::vector<Item> items;
extern std::vector<Actor> doors;
extern std::vector<Actor> arrows;

extern void clearBuffers();
void init();
void gameTick(bool &isOnGround, bool &isOnStairs);
void loadTiles( std::vector<std::string> tilesToLoad );
void updateHero(bool isOnGround, bool isJumping, bool isUpPressed, bool isDownPressed, bool isLeftPressed, bool isAttacking, bool isUsingSpecial,
                bool isRightPressed, bool isOnStairs, bool isPausePressed );
void prepareRoom(int room);
void enforceScreenLimits();
#endif
