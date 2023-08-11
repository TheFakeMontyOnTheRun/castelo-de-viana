#ifndef GAME_H
#define GAME_H

enum EDirection {
	kDirectionLeft, kDirectionRight
};
enum EStance {
	kUp, kStanding, kClimbing, kAttacking, kJumping, kAltAttacking
};

enum EItemType {
	kMeat, kKey
};

enum EActorType {
	kPlayer,
	kSkeleton,
	kClosedDoor,
	kOpenDoor,
	kArrow,
	kGargoyle,
	kTinhoso,
	kSpawner,
	kCapiroto,
	kHand
};

enum EScreen {
	kIntro, kGame, kGameOver, kVictory
};

struct Vec2i {
	int mX;
	int mY;
};

struct Actor {
	struct Vec2i mPosition;
	struct Vec2i mSpeed;
	enum EActorType mType;
	enum EDirection mDirection;
	enum EStance mStance;
	int mHealth;
	int mActive;
};

struct Item {
	struct Vec2i mPosition;
	enum EItemType mType;
	int mActive;
};

extern struct Actor player;
extern int hasKey;
extern int hasBossOnScreen;
extern int counter;
extern int room;
extern int paused;
extern int heroFrame;
extern int ticksUntilVulnerable;
extern int ticksToShowHealth;
extern int backgroundTiles[6][10];
extern int foregroundTiles[6][10];
extern struct ItemVector foes;
extern struct ItemVector items;
extern struct ItemVector doors;
extern struct ItemVector arrows;
extern struct NativeBitmap *currentScreen;
extern enum EScreen screen;
extern const char *currentBossName;
extern int totalBossHealth;
extern int pauseCoolDown;

extern void clearBuffers();

void init();

void gameTick(int *isOnGround, int *isOnStairs);

void loadTiles(struct ItemVector *tilesToLoad);

void
updateHero(int isOnGround, int isJumping, int isUpPressed,
		   int isDownPressed, int isLeftPressed, int isAttacking,
		   int isUsingSpecial,
		   int isRightPressed, int isOnStairs, int isPausePressed);

void prepareRoom(int room);

void enforceScreenLimits();

void prepareScreenFor(enum EScreen screenState);

void playMusic(int instrument, const char *music);

void playTune(const std::string& music);

#endif
