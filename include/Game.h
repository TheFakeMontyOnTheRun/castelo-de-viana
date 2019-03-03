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
	Vec2i mPosition;
	Vec2i mSpeed;
	EActorType mType;
	EDirection mDirection;
	EStance mStance;
	int mHealth;
	bool mActive;
};

struct Item {
	Vec2i mPosition;
	EItemType mType;
	bool mActive;
};

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
extern ItemVector foes;
extern ItemVector items;
extern ItemVector doors;
extern ItemVector arrows;
extern NativeBitmap *currentScreen;
extern EScreen screen;
extern const char *currentBossName;
extern int totalBossHealth;

extern void clearBuffers();

void init();

void gameTick(bool &isOnGround, bool &isOnStairs);

void loadTiles(ItemVector tilesToLoad);

void
updateHero(bool isOnGround, bool isJumping, bool isUpPressed,
		   bool isDownPressed, bool isLeftPressed, bool isAttacking,
		   bool isUsingSpecial,
		   bool isRightPressed, bool isOnStairs, bool isPausePressed);

void prepareRoom(int room);

void enforceScreenLimits();

void prepareScreenFor(EScreen screenState);

void playMusic(int instrument, const char *music);

void playTune(const char *music);

#endif
