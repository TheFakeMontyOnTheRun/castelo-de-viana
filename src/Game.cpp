#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "Common.h"
#include "NativeBitmap.h"
#include "Game.h"
#include "Renderer.h"
#include "IFileLoaderDelegate.h"
#include "CPackedFileReader.h"
#include "LoadImage.h"

int heroFrame = 0;
Actor player;
int counter = 0;
int room = 0;
bool hasKey = false;
int ticksUntilVulnerable = 14;
int ticksToShowHealth = 14;
int arrowCooldown = 0;
bool paused = false;
bool hasBossOnScreen = false;

int backgroundTiles[6][10];
int foregroundTiles[6][10];
odb::ItemVector foes;
odb::ItemVector doors;
odb::ItemVector items;
odb::ItemVector arrows;

const char* hurtSound = "t240m60i53l8dca";
const char* swordSound = "t240m60i44l8dgd";
const char* arrowSound = "t240m60i44l8abc";
const char* jumpSound = "t240m60i46l8cdefedc";
const char* pickSound = "t240m60i54l8abfc";

int totalBossHealth = 0;
const char* currentBossName;
EScreen screen = kIntro;

void initVec2i( Vec2i& vec, int x, int y ) {
	vec.mX = x;
	vec.mY = y;
}

void evalutePlayerAttack();

void openAllDoors();

void init() {
    player.mPosition.mX = 0;
    player.mPosition.mY = 0;
    player.mSpeed.mX = 0;
    player.mSpeed.mY = 0;
    player.mHealth = 10;
    player.mType = kPlayer;
    player.mStance = kJumping;
    counter = 0;
    room = 0;
    hasKey = false;
    prepareRoom(room);
}

void
updateHero(bool isOnGround, bool isJumping, bool isUpPressed, bool isDownPressed, bool isLeftPressed, bool isAttacking,
           bool isUsingSpecial,
           bool isRightPressed, bool isOnStairs, bool isPausePressed) {

    if (isJumping) {
        if (isOnGround) {
            player.mSpeed.mY = -12;
            playTune(jumpSound);
        }
        player.mStance = kJumping;
    }

    if (isUpPressed) {
        if (isOnStairs) {
            player.mSpeed.mY = -8;
            player.mStance = kClimbing;
        } else if (isOnGround) {
            if ( !isOnStairs && arrowCooldown <= 0) {
                Actor *a = (Actor*)calloc(sizeof(Actor), 1);
                odb::pushVector( &arrows, a );
                a->mType = kArrow;
                a->mPosition = player.mPosition;
                initVec2i( a->mSpeed, 0, -16 );
                a->mActive = true;
                a->mDirection = player.mDirection;
                player.mStance = kUp;
                arrowCooldown = 4;
                playTune(arrowSound);
            }
        }
    }

    if (isDownPressed) {
        if (isOnStairs && !isOnGround) {
            player.mSpeed.mY = +8;
            player.mStance = kClimbing;
        } else {
            int posX = ((player.mPosition.mX + 16) / 32);
            int posY = ((player.mPosition.mY + 16) / 32);

            if ( posY <=5 && foregroundTiles[ posY + 1 ][posX] == 3 ) {
                player.mSpeed.mY = +16; //give the player an extra push down to overcome the engine's protection stopping the player
                //from falling
                player.mStance = kClimbing;
            } else {
                player.mStance = kStanding;
            }
        }
    }

    if (isLeftPressed) {
        player.mSpeed.mX = -8;
        player.mDirection = kDirectionLeft;
        if (isOnGround) {
            player.mStance = kStanding;
        }
    }

    if (isRightPressed) {
        player.mSpeed.mX = +8;
        player.mDirection = kDirectionRight;
        if (isOnGround) {
            player.mStance = kStanding;
        }
    }

    if (isAttacking) {
        player.mStance = kAttacking;
        playTune(swordSound);
    }

    if (isUsingSpecial && arrowCooldown <= 0) {
        Actor *a = (Actor*)calloc( 1, sizeof(Actor));
        odb::pushVector( &arrows, a );
        a->mType = kArrow;
        a->mActive = true;
        a->mPosition = player.mPosition;
        initVec2i( a->mSpeed, player.mDirection == kDirectionRight ? 16 : -16, 0);
        a->mDirection = player.mDirection;
        player.mStance = kAltAttacking;
        arrowCooldown = 4;
        playTune(arrowSound);
    }

    if (isPausePressed) {
        paused = !paused;
        ticksToShowHealth = 14;
    }
}

bool isOnFloor(const Actor &actor) {
    int ground = ((actor.mPosition.mY + 32) / 32);

    if (ground > 5) {
        return true;
    } else {
        if (foregroundTiles[ground][(actor.mPosition.mX + 16) / 32] == 1 ||
            (foregroundTiles[ground][(actor.mPosition.mX + 16) / 32] == 3 &&
             foregroundTiles[((actor.mPosition.mY + 16) / 32)][(actor.mPosition.mX + 16) / 32] != 3)
                ) {
            return true;
        }
    }

    return false;
}

bool isBlockedByWall(const Actor &actor) {
    int front = ((actor.mPosition.mX) / 32);

    if (actor.mDirection == kDirectionRight) {
        front++;
    }

    return (foregroundTiles[(actor.mPosition.mY + 16) / 32][front] == 1);
}

bool collide(const Actor &a, const Item &b, int tolerance = 32) {
    if (std::abs(a.mPosition.mY - b.mPosition.mY) < tolerance) {

        if (a.mDirection == kDirectionRight) {
            int diff = a.mPosition.mX - b.mPosition.mX;
            if (diff < (tolerance) && diff > 0) {
                return true;
            }
        } else {
            int diff = b.mPosition.mX - a.mPosition.mX;
            if (diff < (tolerance) && diff > 0) {
                return true;
            }
        }
    }

    return false;
}

bool collide(const Actor &a, const Actor &b, int tolerance = 32) {
    if (std::abs(a.mPosition.mY - b.mPosition.mY) < tolerance) {

        if (b.mDirection == kDirectionRight) {
            int diff = a.mPosition.mX - b.mPosition.mX;
            if (diff < (tolerance) && diff > 0) {
                return true;
            }
        } else {
            int diff = b.mPosition.mX - a.mPosition.mX;
            if (diff < (tolerance) && diff > 0) {
                return true;
            }
        }
    }

    return false;
}

bool isOnHarmfulBlock(const Actor &actor) {
    int block = foregroundTiles[((actor.mPosition.mY + 16) / 32)][(actor.mPosition.mX + 16) / 32];
    return (block == 5);
}

void hurtPlayer(int ammount) {
    player.mHealth -= ammount;
    ticksUntilVulnerable = 14;
    ticksToShowHealth = 14;
    playTune(hurtSound);
}

bool isOnDoor(const Actor &actor) {
    Actor** actorPtr = (Actor**)doors.items;
    for (size_t pos = 0; pos < doors.used; ++pos ) {
        Actor* door = *actorPtr;
        if (door->mType == kOpenDoor && collide(*door, actor)) {
            return true;
        }
    }

    return false;
}

void advanceFloor() {
    room += 10;
    hasKey = false;
    prepareRoom(room);
}

void updateTimers() {

    if (ticksUntilVulnerable > 0) {
        --ticksUntilVulnerable;
    }

    if (ticksToShowHealth > 0) {
        --ticksToShowHealth;
    }

    if (arrowCooldown > 0) {
        --arrowCooldown;
    }
}

void gameTick(bool &isOnGround, bool &isOnStairs) {
    ++counter;

    if (screen != kGame) {
        return;
    }

    updateTimers();

    if (player.mHealth == 0) {
        screen = kGameOver;
        prepareScreenFor(screen);
        return;
    }

    if (player.mStance == kAttacking) {
        evalutePlayerAttack();

        player.mStance = kStanding;
    }

    isOnStairs = (foregroundTiles[(player.mPosition.mY + 16) / 32][(player.mPosition.mX + 16) / 32] == 3);

    if (isBlockedByWall(player)) {
        player.mSpeed.mX = 0;
    }

    if (isOnHarmfulBlock(player) && ticksUntilVulnerable <= 0) {
        hurtPlayer(1);
    }

    if (isOnDoor(player)) {
        advanceFloor();
    }

    player.mPosition.mX += player.mSpeed.mX;
    player.mPosition.mY += player.mSpeed.mY;

    if (player.mSpeed.mX == 1) {
        player.mSpeed.mX = 0;
    }

    if (player.mSpeed.mY == 1) {
        player.mSpeed.mY = 0;
    }

    enforceScreenLimits();

    if (player.mSpeed.mX == 0 && player.mSpeed.mY == 0 && player.mStance != kAltAttacking ) {
        heroFrame = 0;
    }

    int ceiling = (player.mPosition.mY) / 32;

    isOnGround = isOnFloor(player);

    if (isOnStairs) {
        player.mStance = kClimbing;
        player.mSpeed.mX = 0;
    } else {
        player.mStance = kJumping;
    }

    if (isOnGround || !isOnStairs) {
        player.mStance = kStanding;

        if (player.mSpeed.mX == 0 ) {
            heroFrame = 0;
        }

    }

    if (ceiling < 0) {
        ceiling = 0;
    }

    if (player.mSpeed.mY != 0 && player.mStance == kClimbing) {
        heroFrame = (heroFrame + 1) % 2;
    }

    if ((player.mSpeed.mX != 0 && isOnGround) || player.mStance == kAltAttacking ) {
        heroFrame = (heroFrame + 1) % 2;
    }

    if (isOnGround) {
        player.mSpeed.mX = player.mSpeed.mX / 2;
        player.mPosition.mY = (player.mPosition.mY / 32) * 32;
    }

    if (isOnGround && !isOnStairs) {
        player.mSpeed.mY = odb::min(0, player.mSpeed.mY);
        player.mPosition.mY = odb::min(player.mPosition.mY, (player.mPosition.mY / 32) * 32);
    }

    if (player.mSpeed.mY < 0 && foregroundTiles[ceiling][(player.mPosition.mX + 16) / 32] == 1) {
        player.mSpeed.mY = -player.mSpeed.mY;
    }

    if (player.mSpeed.mY < 0 && foregroundTiles[player.mPosition.mY / 32][(player.mPosition.mX + 16) / 32] == 1) {
        player.mSpeed.mY = -player.mSpeed.mY;
    }

    if (!isOnStairs) {
        player.mSpeed.mY = player.mSpeed.mY + 2;
        //this prevents from jumping while keeping the climbing animation state. Unfortunately, prevents looking up.
        //playerStance = EStance::kStanding;
    } else {
        player.mSpeed.mY = 0;
    }

    Actor** arrowPtr = (Actor**)arrows.items;

    for (size_t pos = 0; pos < arrows.used; ++pos ) {
        Actor* arrow = *arrowPtr;

    	if ( !arrow->mActive ) {
    		continue;
    	}

        arrow->mPosition.mX += arrow->mSpeed.mX;
        arrow->mPosition.mY += arrow->mSpeed.mY;

        if (isBlockedByWall(*arrow)) {
            arrow->mActive = false;
            continue;
        }

        Actor** foePtr = (Actor**)foes.items;
        for ( size_t pos2 = 0; pos2 < foes.used; ++pos2) {
            Actor* foe = *foePtr;

        	if (!foe->mActive) {
        		continue;
        	}

            if ( foe->mType != kHand && collide(*foe, *arrow, 32)) {
                foe->mHealth--;
                arrow->mActive = false;

                if (foe->mType == kGargoyle) {
                    openAllDoors();
                }
            }
            ++foePtr;
        }

        ++arrowPtr;
    }

    Item** itemPtr = (Item**)items.items;
    for (size_t pos = 0; pos < items.used; ++pos ) {

        Item* item = *itemPtr;

    	if (!item->mActive ) {
    		continue;
    	}

        if (collide(player, *item)) {
            if (item->mType == kKey && !hasKey) {
                hasKey = true;
                item->mActive = false;
                playTune(pickSound);
                openAllDoors();

            } else if (item->mType == kMeat) {
                if (player.mHealth < 10) {
                    playTune(pickSound);
					item->mActive = false;
					player.mHealth = 10;
                    playTune(pickSound);
                }
                ticksToShowHealth = 14;
            }
        }

        ++itemPtr;
    }

    Actor **foePtr = (Actor**)foes.items;
    for (size_t pos = 0; pos < foes.used; ++pos ) {
        Actor* foe = *foePtr;

		if (!foe->mActive) {
			continue;
		}

		if (foe->mType == kSpawner) {
            if ( ( counter % 40 ) == 0 && ( foes.used <= 5 ) ) {
                Actor *a = (Actor*)calloc( 1, sizeof(Actor));
                odb::pushVector( &foes, a );
                a->mType = kSkeleton;
                a->mPosition = Vec2i( foe->mPosition );
                a->mSpeed.mX = 8;
                a->mDirection = kDirectionRight;
                a->mActive = true;
                a->mHealth = 2;
            }
            continue;
        }


        if (( counter % 5 ) == 0 && foe->mType == kHand) {
            int dx = player.mPosition.mX - foe->mPosition.mX;
            int dy = player.mPosition.mY - foe->mPosition.mY;

            if ( dx > 0 ) {
                foe->mSpeed.mX = 1;
            } else {
                foe->mSpeed.mX = -1;
            }

            if ( dy > 0 ) {
                foe->mSpeed.mY = 1;
            } else {
                foe->mSpeed.mY = -1;
            }

            switch ( foe->mDirection ) {
                case kDirectionLeft:
                    if ( foe->mPosition.mX < 160 ) {
                        foe->mSpeed.mX = -foe->mSpeed.mX;
                    }
                    break;
                case kDirectionRight:
                    if (  160 <= foe->mPosition.mX) {
                        foe->mSpeed.mX = -foe->mSpeed.mX;
                    }
                    break;
            }
        }

        if (foe->mType != kSkeleton &&
            foe->mType != kTinhoso &&
            foe->mType != kCapiroto &&
            foe->mType != kHand ) {

            continue;
        }

        if (foe->mHealth <= 0) {
        	foe->mActive = false;

            if (foe->mType == kCapiroto) {
                screen = kVictory;
                prepareScreenFor(screen);
                return;
            }

            if (foe->mType == kTinhoso) {
                hasBossOnScreen = false;

                odb::clearVector(&doors);
            }

            ++foePtr;
            continue;
        }

        if (foe->mType != kSkeleton && foe->mType != kHand ) {
            ++foePtr;
            continue;
        }

        foe->mPosition.mX += foe->mSpeed.mX;
        foe->mPosition.mY += foe->mSpeed.mY;

        if (foe->mPosition.mX >= (320 - 32)) {
            foe->mSpeed.mX = -8;
            foe->mDirection = kDirectionLeft;
        }

        if (foe->mPosition.mX < 0) {
            foe->mSpeed.mX = 8;
            foe->mDirection = kDirectionRight;
        }

        if (isBlockedByWall(*foe)) {
            foe->mSpeed.mX *= -1;

            if (foe->mDirection == kDirectionLeft) {
                foe->mDirection = kDirectionRight;
            } else {
                foe->mDirection = kDirectionLeft;
            }
        }

        if ((ticksUntilVulnerable <= 0) && collide(*foe, player, 16)) {
            hurtPlayer(1);
        }

        foe->mSpeed.mY += 2;
        bool isOnGround = isOnFloor(*foe);

        if (isOnGround) {
            foe->mSpeed.mY = 0;
            foe->mPosition.mY = (foe->mPosition.mY / 32) * 32;
        }

        ++foePtr;
    }
}

void openAllDoors() {
    Actor** doorPtr = (Actor**)doors.items;

    for (size_t pos3 = 0; pos3 < doors.used; ++pos3) {
                        Actor* door = *doorPtr;
                        door->mType = kOpenDoor;
                        ++doorPtr;
                    }
}

void evalutePlayerAttack() {

    Actor** foePtr = (Actor**)foes.items;
    for (size_t pos = 0; pos < foes.used; pos++ ) {
        Actor* foe = *foePtr;

		if (!foe->mActive) {
			continue;
		}

		if ( foe->mType != kTinhoso &&
		    foe->mType != kHand &&
		    foe->mType != kCapiroto &&
		    foe->mType != kGargoyle &&
		    collide(*foe, player)) {

            foe->mHealth -= 2;
            return; //only one enemy per attack!
        }
		++foePtr;
    }
}

void prepareRoom(int room) {
    muteSound();
    char buffer[64];

    snprintf(buffer, 64, "%d.bg", room );
    odb::StaticBuffer bgmap = loadFileFromPath("gamedata.pfs", buffer);

    snprintf(buffer, 64, "%d.fg", room );
    odb::StaticBuffer fgmap = loadFileFromPath("gamedata.pfs", buffer);

    memset(backgroundTiles, 0, sizeof(int) * 10 * 6 );
    memset(foregroundTiles, 0, sizeof(int) * 10 * 6 );

    odb::initVector(&foes, 8);
    odb::initVector(&items, 4);
    odb::initVector(&doors, 2);
    odb::initVector(&arrows, 8);

    hasBossOnScreen = false;
    int position = 0;
    for (int y = 0; y < 6; ++y) {
        for (int x = 0; x < 10; ++x) {
            char ch = '0';

            ch = bgmap.data[ position ];
            backgroundTiles[y][x] = ch - '0';

            ch = fgmap.data[ position ];
            ++position;

            if (ch == 'm') {
                foregroundTiles[y][x] = 0;
                Item *item = (Item*)calloc(sizeof(Item), 1);
                odb::pushVector( &items, item );
                item->mType = kMeat;
				item->mActive = true;
                initVec2i(item->mPosition, x * 32, y * 32 );
            } else if (ch == 'k') {
                if (!hasKey) {
                    foregroundTiles[y][x] = 0;
                    Item *item = (Item*)calloc( 1, sizeof(Item));
                    odb::pushVector( &items, item );
                    item->mType = kKey;
                    item->mActive = true;
                    initVec2i(item->mPosition, x * 32, y * 32);
                }
            } else if (ch == 'a') {
                foregroundTiles[y][x] = 0;
                Actor *a = (Actor*)calloc(sizeof(Actor), 1);
                odb::pushVector( &foes, a );
                a->mType = kSkeleton;
                a->mDirection = kDirectionRight;
                initVec2i( a->mPosition, x * 32, y * 32);
                a->mActive = true;
                a->mSpeed.mX = 8;
                a->mHealth = 2;
            } else if (ch == 'c') {
                foregroundTiles[y][x] = 0;
                currentBossName = "CAPIROTO";
                Actor *a = (Actor*)calloc( 1, sizeof(Actor));
                odb::pushVector( &foes, a );
                a->mType = kCapiroto;
                initVec2i( a->mPosition, x * 32, y * 32);
                a->mHealth = 25;
                a->mActive = true;
                totalBossHealth = 25;
                hasBossOnScreen = true;

                {
                    foregroundTiles[y + 2][ x + 2] = 0;
                    Actor *a = (Actor*)calloc( 1, sizeof(Actor));
                    odb::pushVector( &foes, a );
                    a->mType = kHand;
                    initVec2i( a->mPosition, (x + 2 ) * 32, (y + 2) * 32 );
                    a->mActive = true;
                    a->mDirection = kDirectionLeft;
                    a->mHealth = 100000;
                }
                {
                    foregroundTiles[y + 2][x - 2] = 0;
                    Actor *a = (Actor*)calloc( 1, sizeof(Actor));
                    odb::pushVector( &foes, a );
                    a->mType = kHand;
                    a->mActive = true;
                    a->mDirection = kDirectionRight;
                    initVec2i( a->mPosition, (x - 2) * 32, (y + 2) * 32 );
                    a->mHealth = 100000;
                }


            } else if (ch == 't') {
                foregroundTiles[y][x] = 0;
                currentBossName = "TINHOSO";
                totalBossHealth = 5;
                Actor *a = (Actor*)calloc( 1, sizeof(Actor));
                odb::pushVector( &foes, a );
                a->mType = kTinhoso;
                a->mActive = true;
                initVec2i(a->mPosition, x * 32, y * 32 );
                a->mHealth = 5;
                hasBossOnScreen = true;
            } else if (ch == 's') {
                foregroundTiles[y][x] = 0;
                Actor *a = (Actor*)calloc( 1, sizeof(Actor));
                odb::pushVector( &foes, a );
                a->mActive = true;
                a->mType = kSpawner;
                initVec2i( a->mPosition, x * 32, y * 32);
                a->mHealth = 20;
            } else if (ch == 'g') {
                foregroundTiles[y][x] = 0;
                Actor *a = (Actor*)calloc( 1, sizeof(Actor));
                odb::pushVector( &foes, a );
                a->mType = kGargoyle;
                initVec2i( a->mPosition, x * 32, y * 32 );
                a->mSpeed.mX = 8;
                a->mActive = true;
                a->mHealth = 1;
            } else if (ch == 'd') {
                foregroundTiles[y][x] = 0;
                Actor *a = (Actor*)calloc( 1, sizeof(Actor));
                odb::pushVector( &doors, a );
                a->mType = hasKey ? kOpenDoor : kClosedDoor;
                initVec2i( a->mPosition, x * 32, y * 32 );
                a->mActive = true;
            } else if (ch == 'D') {
                foregroundTiles[y][x] = 0;
                Actor *a = (Actor*)calloc( 1, sizeof(Actor));
                odb::pushVector( &doors, a );
                a->mType = kClosedDoor;
                initVec2i( a->mPosition, x * 32, y * 32 );
                a->mActive = true;
            } else {
                foregroundTiles[y][x] = ch - '0';
            }
        }
        ++position; //\n
    }

    snprintf(buffer, 64, "%d.lst", room );

    odb::StaticBuffer listBuffer = loadFileFromPath( "gamedata.pfs", buffer);

    size_t amount = odb::countTokens((char*)listBuffer.data, listBuffer.size) + 1;
    odb::ItemVector tilestoLoad;
    odb::initVector( &tilestoLoad, amount );

    int lastPoint = 0;
    int since = 0;
    uint8_t* bufferBegin = listBuffer.data;

    for ( size_t pos = 0; pos < listBuffer.size; ++pos ) {
        char c = listBuffer.data[ pos ];
        ++since;

        if ( pos == listBuffer.size - 1 || c == '\n') {

            if ( pos == listBuffer.size - 1 ) {
                since++;
            }

            char* filename = (char *)(calloc(since - 1 + 1, 1 ));
            memcpy( filename,  bufferBegin + lastPoint, since -1  );
            lastPoint += since;
            if ( strlen(filename) > 0 ) {
                odb::pushVector( &tilestoLoad, filename );
            }
            since = 0;
        }
    }

    loadTiles(tilestoLoad);

    clearBuffers();
	free( listBuffer.data );
    free(fgmap.data);
	free(bgmap.data);
}

void enforceScreenLimits() {
    if (player.mPosition.mX < 0) {
        if ((room % 10) > 0 && !hasBossOnScreen) {
            player.mPosition.mX = 320 - 32 - 1;
            prepareRoom(--room);
        } else {
            player.mPosition.mX = 0;
        }
    }

    if (player.mPosition.mY < 0) {
        if ((room / 10) <= 9 && !hasBossOnScreen) {
            player.mPosition.mY = (32 * 6) - 32 - 1;
            room += 10;
            prepareRoom(room);
        } else {
            player.mPosition.mY = 0;
        }
    }

    if ((player.mPosition.mX + 32) >= 320) {
        if ((room % 10) < 9 && !hasBossOnScreen) {
            player.mPosition.mX = 1;
            prepareRoom(++room);
        } else {
            player.mPosition.mX = 320 - 32;
        }
    }

    if ((player.mPosition.mY + 32) >= (32 * 6)) {
        if ((room / 10) >= 1 && !hasBossOnScreen) {
            player.mPosition.mY = 1;
            room -= 10;
            prepareRoom(room);
        } else {
            player.mPosition.mY = (32 * 6) - 32 - 1;
        }
    }
}