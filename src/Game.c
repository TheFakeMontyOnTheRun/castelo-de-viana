#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#ifdef AMIGA
#include "AmigaInt.h"

#else

#ifdef CPC

#include "CPCInt.h"

#else

#include <stdint.h>
#include <unistd.h>

#endif

#endif

#include "Common.h"
#include "NativeBitmap.h"
#include "Game.h"
#include "Renderer.h"
#include "CPackedFileReader.h"
#include "LoadImage.h"

int heroFrame = 0;
struct Actor player;
int counter = 0;
int room = 0;
int hasKey = FALSE;
int ticksUntilVulnerable = 14;
int ticksToShowHealth = 14;
int arrowCooldown = 0;
int paused = FALSE;
int hasBossOnScreen = FALSE;
int pauseCoolDown = 0;

int backgroundTiles[6][10];
int foregroundTiles[6][10];
struct ItemVector foes;
struct ItemVector doors;
struct ItemVector items;
struct ItemVector arrows;

const char *hurtSound = "t240m60i53l8dca";
const char *swordSound = "t240m60i44l8dgd";
const char *arrowSound = "t240m60i44l8abc";
const char *jumpSound = "t240m60i46l8cdefedc";
const char *pickSound = "t240m60i54l8abfc";

int totalBossHealth = 0;
const char *currentBossName;
enum EScreen screen = kIntro;

void initVec2i(struct Vec2i *vec, int x, int y) {
    vec->mX = x;
    vec->mY = y;
}

void evaluatePlayerAttack();

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
    hasKey = FALSE;
    prepareRoom(room);
}

void
updateHero(int isOnGround, int isJumping, int isUpPressed, int isDownPressed,
           int isLeftPressed, int isAttacking,
           int isUsingSpecial,
           int isRightPressed, int isOnStairs, int isPausePressed) {

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
            if (!isOnStairs && arrowCooldown <= 0 && arrows.used == 0) {
                struct Actor *a = (struct Actor *) calloc(sizeof(struct Actor), 1);
                pushVector(&arrows, a);
                a->mType = kArrow;
                a->mPosition.mX = player.mPosition.mX;
                a->mPosition.mY = player.mPosition.mY;
                initVec2i(&a->mSpeed, 0, -16);
                a->mActive = TRUE;
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

            if (posY <= 5 && foregroundTiles[posY + 1][posX] == 3) {
/*give the player an extra push down to overcome the engine's
protection stopping the player from falling*/

                player.mSpeed.mY = +16;
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

    if (isUsingSpecial && arrowCooldown <= 0 && arrows.used == 0) {
        struct Actor *a = (struct Actor *) calloc(1, sizeof(struct Actor));
        assert(pushVector(&arrows, a));
        a->mType = kArrow;
        a->mActive = TRUE;
        a->mPosition.mX = player.mPosition.mX;
        a->mPosition.mY = player.mPosition.mY;

        initVec2i(&a->mSpeed, player.mDirection == kDirectionRight ? 16 : -16, 0);
        a->mDirection = player.mDirection;
        player.mStance = kAltAttacking;
        arrowCooldown = 4;
        playTune(arrowSound);
    }

    if (isPausePressed) {
        if (pauseCoolDown <= 0 ) {
            ticksToShowHealth = 14;
            pauseCoolDown = 10;
            paused = !paused;
        }
    }
}

int isOnFloor(const struct Actor *actor) {
    int ground = ((actor->mPosition.mY + 32) / 32);

    if (ground > 5) {
        return TRUE;
    } else {
        if (foregroundTiles[ground][(actor->mPosition.mX + 16) / 32] == 1 ||
            (foregroundTiles[ground][(actor->mPosition.mX + 16) / 32] == 3 &&
             foregroundTiles[((actor->mPosition.mY + 16) / 32)][(actor->mPosition.mX + 16) / 32] != 3)
                ) {
            return TRUE;
        }
    }

    return FALSE;
}

int isBlockedByWall(const struct Actor *actor) {
    int front = ((actor->mPosition.mX) / 32);

    if (actor->mDirection == kDirectionRight) {
        front++;
    }

    return (foregroundTiles[(actor->mPosition.mY + 16) / 32][front] == 1);
}

int collideActorActor(const struct Actor *a, const struct Actor *b, int tolerance) {
    if (abs(a->mPosition.mY - b->mPosition.mY) < tolerance) {

        if (a->mDirection == kDirectionRight) {
            int diff = a->mPosition.mX - b->mPosition.mX;
            if (diff < (tolerance) && diff > 0) {
                return TRUE;
            }
        } else {
            int diff = b->mPosition.mX - a->mPosition.mX;
            if (diff < (tolerance) && diff > 0) {
                return TRUE;
            }
        }
    }

    return FALSE;
}

int collideActorItem(const struct Actor *a, const struct Item *b, int tolerance) {
    if (abs(a->mPosition.mY - b->mPosition.mY) < tolerance) {

        if (a->mDirection == kDirectionRight) {
            int diff = a->mPosition.mX - b->mPosition.mX;
            if (diff < (tolerance) && diff > 0) {
                return TRUE;
            }
        } else {
            int diff = b->mPosition.mX - a->mPosition.mX;
            if (diff < (tolerance) && diff > 0) {
                return TRUE;
            }
        }
    }

    return FALSE;
}

int isOnHarmfulBlock(const struct Actor *actor) {
    int block = foregroundTiles[((actor->mPosition.mY + 16) / 32)][(actor->mPosition.mX + 16) / 32];
    return (block == 5);
}

void hurtPlayer(int ammount) {
    player.mHealth -= ammount;
    ticksUntilVulnerable = 14;
    ticksToShowHealth = 14;
    playTune(hurtSound);
}

int isOnDoor(const struct Actor *actor) {
    size_t pos = 0;
    for (pos = 0; pos < doors.used; ++pos) {
        const struct Actor *door = doors.items[pos];
        if (door != NULL && door->mType == kOpenDoor && collideActorActor(door, actor, DEFAULT_TOLERANCE)) {
            return TRUE;
        }
    }

    return FALSE;
}

void advanceFloor() {
    room += 10;
    hasKey = FALSE;
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

void gameTick(int *isOnGround, int *isOnStairs) {
    int ceiling;
    size_t pos;
    size_t pos2 = 0;

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
        evaluatePlayerAttack();

        player.mStance = kStanding;
    }

    *isOnStairs = (foregroundTiles[(player.mPosition.mY + 16) / 32][(player.mPosition.mX + 16) / 32] == 3);

    if (isBlockedByWall(&player)) {
        player.mSpeed.mX = 0;
    }

    if (isOnHarmfulBlock(&player) && ticksUntilVulnerable <= 0) {
        hurtPlayer(1);
    }

    if (isOnDoor(&player)) {
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

    if (player.mSpeed.mX == 0 && player.mSpeed.mY == 0 && player.mStance != kAltAttacking) {
        heroFrame = 0;
    }

    ceiling = (player.mPosition.mY) / 32;

    *isOnGround = isOnFloor(&player);

    if (*isOnStairs) {
        player.mStance = kClimbing;
        player.mSpeed.mX = 0;
    } else {
        player.mStance = kJumping;
    }

    if (*isOnGround || !*isOnStairs) {
        player.mStance = kStanding;

        if (player.mSpeed.mX == 0) {
            heroFrame = 0;
        }

    }

    if (ceiling < 0) {
        ceiling = 0;
    }

    if (player.mSpeed.mY != 0 && player.mStance == kClimbing) {
        heroFrame = (heroFrame + 1) % 2;
    }

    if ((player.mSpeed.mX != 0 && *isOnGround) || player.mStance == kAltAttacking) {
        heroFrame = (heroFrame + 1) % 2;
    }

    if (*isOnGround) {
        player.mSpeed.mX = player.mSpeed.mX / 2;
        player.mPosition.mY = (player.mPosition.mY / 32) * 32;
    }

    if (*isOnGround && !*isOnStairs) {
        player.mSpeed.mY = min(0, player.mSpeed.mY);
        player.mPosition.mY = min(player.mPosition.mY, (player.mPosition.mY / 32) * 32);
    }

    if (player.mSpeed.mY < 0 && foregroundTiles[ceiling][(player.mPosition.mX + 16) / 32] == 1) {
        player.mSpeed.mY = -player.mSpeed.mY;
    }

    if (player.mSpeed.mY < 0 && foregroundTiles[player.mPosition.mY / 32][(player.mPosition.mX + 16) / 32] == 1) {
        player.mSpeed.mY = -player.mSpeed.mY;
    }

    if (!*isOnStairs) {
        player.mSpeed.mY = player.mSpeed.mY + 2;
/*this prevents from jumping while keeping the climbing animation state.
Unfortunately, prevents looking up.*/

    } else {
        player.mSpeed.mY = 0;
    }

    for (pos = 0; pos < arrows.used; ++pos) {
        struct Actor *arrow = arrows.items[pos];

        if (arrow == NULL) {
            continue;
        }

        if (!arrow->mActive) {
            continue;
        }

        arrow->mPosition.mX += arrow->mSpeed.mX;
        arrow->mPosition.mY += arrow->mSpeed.mY;

        if (isBlockedByWall(arrow)) {
            removeFromVector(&arrows, arrow);
            free(arrow);
            continue;
        }

        for (pos2 = 0; pos2 < foes.used; ++pos2) {
            struct Actor *foe = foes.items[pos2];

            if (foe == NULL || !foe->mActive) {
                continue;
            }

            if (foe->mType != kHand && collideActorActor(foe, arrow, DEFAULT_TOLERANCE)) {
                foe->mHealth--;
                removeFromVector(&arrows, arrow);
                free(arrow);


                if (foe->mType == kGargoyle) {
                    openAllDoors();
                }
            }
        }
    }

    for (pos = 0; pos < items.used; ++pos) {

        struct Item *item = items.items[pos];

        if (item == NULL || !item->mActive) {
            continue;
        }

        if (collideActorItem(&player, item, DEFAULT_TOLERANCE)) {
            if (item->mType == kKey && !hasKey) {
                hasKey = TRUE;
                item->mActive = FALSE;
                playTune(pickSound);
                openAllDoors();

            } else if (item->mType == kMeat) {
                if (player.mHealth < 10) {
                    playTune(pickSound);
                    item->mActive = FALSE;
                    player.mHealth = 10;
                    playTune(pickSound);
                }
                ticksToShowHealth = 14;
            }
        }
    }

    for (pos = 0; pos < foes.used; ++pos) {
        struct Actor *foe = foes.items[pos];

        if (foe == NULL || !foe->mActive) {
            continue;
        }

        if (foe->mType == kSpawner) {
            if ((counter % 40) == 0 && (foes.used <= 5)) {
                struct Actor *a = (struct Actor *) calloc(1, sizeof(struct Actor));
                pushVector(&foes, a);
                a->mType = kSkeleton;
                a->mSpeed.mX = 8;
                a->mPosition = foe->mPosition;
                a->mDirection = kDirectionRight;
                a->mActive = TRUE;
                a->mHealth = 2;
            }
            continue;
        }


        if ((counter % 5) == 0 && foe->mType == kHand) {
            int dx = player.mPosition.mX - foe->mPosition.mX;
            int dy = player.mPosition.mY - foe->mPosition.mY;

            if (dx > 0) {
                foe->mSpeed.mX = 1;
            } else {
                foe->mSpeed.mX = -1;
            }

            if (dy > 0) {
                foe->mSpeed.mY = 1;
            } else {
                foe->mSpeed.mY = -1;
            }

            switch (foe->mDirection) {
                case kDirectionLeft:
                    if (foe->mPosition.mX < 160) {
                        foe->mSpeed.mX = -foe->mSpeed.mX;
                    }
                    break;
                case kDirectionRight:
                    if (160 <= foe->mPosition.mX) {
                        foe->mSpeed.mX = -foe->mSpeed.mX;
                    }
                    break;
            }
        }

        if (foe->mType != kSkeleton &&
            foe->mType != kTinhoso &&
            foe->mType != kCapiroto &&
            foe->mType != kHand) {

            continue;
        }

        if (foe->mHealth <= 0) {
            foe->mActive = FALSE;

            if (foe->mType == kCapiroto) {
                screen = kVictory;
                prepareScreenFor(screen);
                return;
            } else if (foe->mType == kTinhoso) {
                hasBossOnScreen = FALSE;

                clearVector(&doors);
            }

            continue;
        }

        if (foe->mType != kSkeleton && foe->mType != kHand) {
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

        if (isBlockedByWall(foe)) {
            foe->mSpeed.mX *= -1;

            if (foe->mDirection == kDirectionLeft) {
                foe->mDirection = kDirectionRight;
            } else {
                foe->mDirection = kDirectionLeft;
            }
        }

        if ((ticksUntilVulnerable <= 0) && collideActorActor(foe, &player, 16)) {
            hurtPlayer(1);
        }

        foe->mSpeed.mY += 2;

        if (isOnFloor(foe)) {
            foe->mSpeed.mY = 0;
            foe->mPosition.mY = (foe->mPosition.mY / 32) * 32;
        }
    }

    for (pos = 0; pos < foes.used; ++pos) {
        struct Actor *foe = foes.items[pos];

        if (foe != NULL && !foe->mActive) {
            removeFromVector(&foes, foe);
        }
    }
}

void openAllDoors() {
    size_t pos = 0;
    for (pos = 0; pos < doors.used; ++pos) {
        struct Actor *door = doors.items[pos];
        if (door != NULL) {
            door->mType = kOpenDoor;
        }
    }
}

void evaluatePlayerAttack() {

    size_t pos = 0;
    for (pos = 0; pos < foes.used; pos++) {
        struct Actor *foe = foes.items[pos];

        if (foe == NULL || !foe->mActive) {
            continue;
        }

		if ( foe->mType != kTinhoso &&
		    foe->mType != kHand &&
		    foe->mType != kCapiroto &&
		    foe->mType != kGargoyle &&
		    collideActorActor(foe, &player, DEFAULT_TOLERANCE)) {

            foe->mHealth -= 2;
            return; /*only one enemy per attack!*/
        }
    }
}

void prepareRoom(int room) {
    char buffer[64];
    struct StaticBuffer bgmap;
    struct StaticBuffer fgmap;
    int position;
    struct StaticBuffer listBuffer;
    struct ItemVector tilestoLoad;
    size_t amount;
    int lastPoint;
    int since;
    uint8_t *bufferBegin;
    size_t pos;
    int y;

    muteSound();

    sprintf(buffer, "%d.bg", room);
    bgmap = loadFileFromPath("gamedata.pfs", buffer);

    sprintf(buffer, "%d.fg", room);
    fgmap = loadFileFromPath("gamedata.pfs", buffer);

    memset(backgroundTiles, 0, sizeof(int) * 10 * 6);
    memset(foregroundTiles, 0, sizeof(int) * 10 * 6);

    initVector(&foes, 8);
    initVector(&items, 4);
    initVector(&doors, 2);
    initVector(&arrows, 8);

    hasBossOnScreen = FALSE;
    position = 0;

    for (y = 0; y < 6; ++y) {
        int x = 0;
        for (x = 0; x < 10; ++x) {
            char ch = '0';

            ch = bgmap.data[position];
            backgroundTiles[y][x] = ch - '0';

            ch = fgmap.data[position];
            ++position;

            if (ch == 'm') {
                struct Item *item;
                foregroundTiles[y][x] = 0;
                item = (struct Item *) calloc(sizeof(struct Item), 1);
                pushVector(&items, item);
                item->mType = kMeat;
                item->mActive = TRUE;
                initVec2i(&item->mPosition, x * 32, y * 32);
            } else if (ch == 'k') {
                if (!hasKey) {
                    struct Item *item;
                    foregroundTiles[y][x] = 0;
                    item = (struct Item *) calloc(1, sizeof(struct Item));
                    pushVector(&items, item);
                    item->mType = kKey;
                    item->mActive = TRUE;
                    initVec2i(&item->mPosition, x * 32, y * 32);
                }
            } else if (ch == 'a') {
                struct Actor *a;
                foregroundTiles[y][x] = 0;
                a = (struct Actor *) calloc(sizeof(struct Actor), 1);
                pushVector(&foes, a);
                a->mType = kSkeleton;
                a->mDirection = kDirectionRight;
                initVec2i(&a->mPosition, x * 32, y * 32);
                a->mActive = TRUE;
                a->mSpeed.mX = 8;
                a->mHealth = 2;
            } else if (ch == 'c') {
                struct Actor *a;
                foregroundTiles[y][x] = 0;
                currentBossName = "CAPIROTO";
                a = (struct Actor *) calloc(1, sizeof(struct Actor));
                pushVector(&foes, a);
                a->mType = kCapiroto;
                initVec2i(&a->mPosition, x * 32, y * 32);
                a->mHealth = 25;
                a->mActive = TRUE;
                totalBossHealth = 25;
                hasBossOnScreen = TRUE;

                {
                    struct Actor *a;
                    foregroundTiles[y + 2][x + 2] = 0;
                    a = (struct Actor *) calloc(1, sizeof(struct Actor));
                    pushVector(&foes, a);
                    a->mType = kHand;
                    initVec2i(&a->mPosition, (x + 2) * 32, (y + 2) * 32);
                    a->mActive = TRUE;
                    a->mDirection = kDirectionLeft;
                    a->mHealth = 100000;
                }
                {
                    struct Actor *a;
                    foregroundTiles[y + 2][x - 2] = 0;
                    a = (struct Actor *) calloc(1, sizeof(struct Actor));
                    pushVector(&foes, a);
                    a->mType = kHand;
                    a->mActive = TRUE;
                    a->mDirection = kDirectionRight;
                    initVec2i(&a->mPosition, (x - 2) * 32, (y + 2) * 32);
                    a->mHealth = 100000;
                }


            } else if (ch == 't') {
                struct Actor *a;
                foregroundTiles[y][x] = 0;
                currentBossName = "TINHOSO";
                totalBossHealth = 5;
                a = (struct Actor *) calloc(1, sizeof(struct Actor));
                pushVector(&foes, a);
                a->mType = kTinhoso;
                a->mActive = TRUE;
                initVec2i(&a->mPosition, x * 32, y * 32);
                a->mHealth = 5;
                hasBossOnScreen = TRUE;
            } else if (ch == 's') {
                struct Actor *a;
                foregroundTiles[y][x] = 0;
                a = (struct Actor *) calloc(1, sizeof(struct Actor));
                pushVector(&foes, a);
                a->mActive = TRUE;
                a->mType = kSpawner;
                initVec2i(&a->mPosition, x * 32, y * 32);
                a->mHealth = 20;
            } else if (ch == 'g') {
                struct Actor *a;
                foregroundTiles[y][x] = 0;
                a = (struct Actor *) calloc(1, sizeof(struct Actor));
                pushVector(&foes, a);
                a->mType = kGargoyle;
                initVec2i(&a->mPosition, x * 32, y * 32);
                a->mSpeed.mX = 8;
                a->mActive = TRUE;
                a->mHealth = 1;
            } else if (ch == 'd') {
                struct Actor *a;
                foregroundTiles[y][x] = 0;
                a = (struct Actor *) calloc(1, sizeof(struct Actor));
                pushVector(&doors, a);
                a->mType = hasKey ? kOpenDoor : kClosedDoor;
                initVec2i(&a->mPosition, x * 32, y * 32);
                a->mActive = TRUE;
            } else if (ch == 'D') {
                struct Actor *a;
                foregroundTiles[y][x] = 0;
                a = (struct Actor *) calloc(1, sizeof(struct Actor));
                pushVector(&doors, a);
                a->mType = kClosedDoor;
                initVec2i(&a->mPosition, x * 32, y * 32);
                a->mActive = TRUE;
            } else {
                foregroundTiles[y][x] = ch - '0';
            }
        }
        ++position; /* \n */
    }

    sprintf(buffer, "%d.lst", room);

    listBuffer = loadFileFromPath("gamedata.pfs", buffer);

    amount = countTokens((char *) listBuffer.data, listBuffer.size) + 1;
    initVector(&tilestoLoad, amount);

    lastPoint = 0;
    since = 0;
    bufferBegin = listBuffer.data;

    for (pos = 0; pos < listBuffer.size; ++pos) {
        char c = listBuffer.data[pos];
        ++since;

        if (pos == listBuffer.size - 1 || c == '\n') {
            char *filename;

            if (pos == listBuffer.size - 1) {
                since++;
            }

            filename = (char *) (calloc(since - 1 + 1, 1));
            memcpy(filename, bufferBegin + lastPoint, since - 1);
            lastPoint += since;
            if (strlen(filename) > 0) {
                pushVector(&tilestoLoad, filename);
            }
            since = 0;
        }
    }

    loadTiles(&tilestoLoad);

    clearBuffers();
    free(listBuffer.data);
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
