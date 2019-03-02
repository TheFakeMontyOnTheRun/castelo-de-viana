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
    kPlayer, kSkeleton, kClosedDoor, kOpenDoor, kArrow, kGargoyle, kTinhoso, kSpawner, kCapiroto, kHand
};

enum EScreen {
    kIntro, kGame, kGameOver, kVictory
};

class Vec2i {
public:
    int mX = 0;
    int mY = 0;
};

struct Actor {
    Vec2i mPosition;
    Vec2i mSpeed;
    EActorType mType;
    EDirection mDirection = EDirection::kRight;
    EStance mStance = EStance::kStanding;
    int mHealth = 0;
    bool mActive = true;
};

struct Item {
    Vec2i mPosition;
    EItemType mType;
    bool mActive;
};

bool operator==(const Vec2i &a, const Vec2i &b);

bool operator==(const Item &a, const Item &b);

bool operator==(const Actor &a, const Actor &b);

extern Actor player;
extern bool hasKey;
extern bool hasBossOnScreen;
extern int counter;
extern int room;
extern bool paused;
extern int heroFrame;
extern int ticksUntilVulnerable;
extern int ticksToShowHealth;
extern int backgroundTiles[6][10];
extern int foregroundTiles[6][10];
extern std::vector<Actor> foes;
extern std::vector<Item> items;
extern std::vector<Actor> doors;
extern std::vector<Actor> arrows;
extern odb::NativeBitmap* currentScreen;
extern EScreen screen;
extern const char* currentBossName;
extern int totalBossHealth;
extern void clearBuffers();

void init();

void gameTick(bool &isOnGround, bool &isOnStairs);

void loadTiles(std::vector<char*> tilesToLoad);

void
updateHero(bool isOnGround, bool isJumping, bool isUpPressed, bool isDownPressed, bool isLeftPressed, bool isAttacking,
           bool isUsingSpecial,
           bool isRightPressed, bool isOnStairs, bool isPausePressed);

void prepareRoom(int room);

void enforceScreenLimits();

void prepareScreenFor(EScreen screenState);

void playMusic(int instrument, const char* music);

void playTune(const char* music);

#endif
