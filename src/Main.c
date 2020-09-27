#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


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

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

enum EVideoType videoType;


int enableSecret = FALSE;

struct ItemVector tiles;

struct NativeBitmap *pausedSign;

struct NativeBitmap *arrowSprite[2];

struct NativeBitmap *doorStates[2];

struct NativeBitmap *foeSprites[2];

struct NativeBitmap *itemSprites[2];

struct NativeBitmap *gargoyleSprites[2];

struct NativeBitmap *capirotoSprites[2];

struct NativeBitmap *handSprites[2];

struct NativeBitmap *tinhosoSprites[2];

struct NativeBitmap *hero[6][2];

uint8_t imageBuffer[320 * 200];

struct NativeBitmap *currentScreen = NULL;

void initOPL2() {
    setupOPL2();
}

void prepareScreenFor(enum EScreen screenState) {

    muteSound();

    pauseCoolDown = 10;

    switch (screenState) {
        case kIntro:
            currentScreen = loadBitmap((enableSecret ? "secret.img" : "intro.png"), videoType);
            playTune("eefggfedccdeedd");
            break;
        case kGame:
            currentScreen = NULL;
            break;
        case kGameOver:
            currentScreen = loadBitmap("gameover.png", videoType);
            playTune("gggefffd");
            break;
        case kVictory:
            currentScreen = loadBitmap("victory.png", videoType);
            playTune("eefggfedccdeed12d4eefggfedccdedc12c4ddecde12f12ecde12f12edcdpeefggfedccdedc12c4");
            break;
    }
}

void clearBuffers() {
    memset(imageBuffer, 4, 320 * 200);
}

void loadTiles(struct ItemVector *tilesToLoad) {
    size_t pos;
    char **ptr = (char **) tilesToLoad->items;
    initVector(&tiles, tilesToLoad->used);


    for (pos = 0; pos < tilesToLoad->used; ++pos) {

        char *tile = *ptr;

        struct ItemVector *strip = (struct ItemVector *) calloc(sizeof(struct ItemVector), 1);


        if (!strcmp(tile + (strlen(tile) - 4), ".png")) {
            initVector(strip, 1);
            pushVector(strip, loadBitmap(tile, videoType));
            pushVector(&tiles, strip);
        } else {
            pushVector(&tiles, loadSpriteList(tile, videoType));
        }

        ++ptr;
    }
}

void render() {
    int y0;
    int y1;
    int x0;
    int x1;
    int ty;
    int pixel;
    int y;
    int x;
    int bossHealthColour;
    int heroHealthColour;
    int backgroundColour;
    uint8_t *pixelData;
    struct NativeBitmap *sprite;
    uint8_t transparency;
    size_t pos;

    if (videoType == kCGA) {
        memset(imageBuffer, 4, 320 * 200);
        transparency = 0;
    } else {
        memset(imageBuffer, 0, 320 * 200);
        transparency = 199;
    }


    if (currentScreen != NULL) {

        uint8_t *pixelData = currentScreen->mRawData;

        int c = 0;
        for (c = 0; c < 320 * 200; ++c) {
            imageBuffer[c] = pixelData[c];
        }

        copyImageBufferToVideoMemory(&imageBuffer[0]);
        return;
    }


    y0 = 0;
    y1 = 0;
    x0 = 0;
    x1 = 0;

    for (ty = 0; ty < 6; ++ty) {
        int tx = 0;
        for (tx = 0; tx < 10; ++tx) {
            struct NativeBitmap *tile;
            uint8_t *pixelData;

            y0 = (ty * 32);
            y1 = 32 + (ty * 32);
            x0 = (tx * 32);
            x1 = 32 + (tx * 32);
            pixel = 4;

            if (backgroundTiles[ty][tx] != 0) {
                struct ItemVector *tileset = (struct ItemVector *) tiles.items[backgroundTiles[ty][tx]];
                tile = (struct NativeBitmap *) tileset->items[counter % tileset->used];

                pixelData = tile->mRawData;

                pixel = 4;
                y = y0;
                for (y = y0; y < y1; ++y) {

                    uint8_t *sourceLine = pixelData + (32 * (y - y0));
                    uint8_t *destLine = &imageBuffer[0] + (320 * y) + x0;

                    int x = x0;
                    for (x = x0; x < x1; ++x) {

                        pixel = *sourceLine;

                        if (pixel != transparency) {
                            *destLine = pixel;
                        }

                        ++sourceLine;
                        ++destLine;
                    }
                }
            }

            if (foregroundTiles[ty][tx] != 0) {
                struct ItemVector *tileset = (struct ItemVector *) tiles.items[foregroundTiles[ty][tx]];
                tile = (struct NativeBitmap *) tileset->items[counter % tileset->used];
                pixelData = tile->mRawData;

                pixel = 4;
                y = y0;
                for (y = y0; y < y1; ++y) {

                    uint8_t *sourceLine = pixelData + (32 * (y - y0));
                    uint8_t *destLine = &imageBuffer[0] + (320 * y) + x0;

                    int x = x0;
                    for (x = x0; x < x1; ++x) {

                        pixel = *sourceLine;

                        if (pixel != transparency) {
                            *destLine = pixel;
                        }

                        ++sourceLine;
                        ++destLine;
                    }
                }
            }
        }
    }

    for (pos = 0; pos < doors.used; ++pos) {
        struct Actor *door = doors.items[pos];

        if (door == NULL) {
            continue;
        }

        pixelData = doorStates[door->mType - kClosedDoor]->mRawData;
        y0 = (door->mPosition.mY);
        y1 = 32 + y0;
        x0 = (door->mPosition.mX);
        x1 = 32 + x0;

        pixel = 0;

        y = y0;
        for (y = y0; y < y1; ++y) {
            int x;

            if (y < 0 || y >= 200) {
                continue;
            }

            x = x0;
            for (x = x0; x < x1; ++x) {
                pixel = (pixelData[(32 * (y - y0)) + ((x - x0))]);

                if (pixel == transparency) {
                    continue;
                }

                if (x < 0 || x >= 320) {
                    continue;
                }

                imageBuffer[(320 * y) + (x)] = pixel;
            }
        }
    }

    sprite = hero[player.mStance][heroFrame];

    if (((ticksUntilVulnerable <= 0) || ((counter % 2) == 0)) || paused) {
        int spriteWidth;

        y0 = (player.mPosition.mY);
        spriteWidth = sprite->mWidth;
        y1 = sprite->mHeight + y0;
        x0 = (player.mPosition.mX);

        if (player.mDirection == kDirectionLeft) {
            x0 -= (spriteWidth - 32);
        }

        x1 = spriteWidth + x0;
        pixelData = sprite->mRawData;

        pixel = 0;

        for (y = y0; y < y1; ++y) {

            if (y < 0 || y >= 200) {
                continue;
            }

            for (x = x0; x < x1; ++x) {
                if (player.mDirection == kDirectionRight) {
                    pixel = (pixelData[(spriteWidth * (y - y0)) + ((x - x0))]);
                } else {
                    pixel = (pixelData[(spriteWidth * (y - y0)) + ((spriteWidth - 1) - (x - x0))]);
                }

                if (pixel == transparency) {
                    continue;
                }

                if (x < 0 || x >= 320) {
                    continue;
                }

                imageBuffer[(320 * y) + (x)] = pixel;
            }
        }
    }

    for (pos = 0; pos < arrows.used; ++pos) {
        int pixel;
        int y;
        int x;
        struct Actor *arrow = arrows.items[pos];

        if (arrow == NULL || !arrow->mActive) {
            continue;
        }

        if (abs(arrow->mSpeed.mX) > abs(arrow->mSpeed.mY)) {
            pixelData = arrowSprite[0]->mRawData;
        } else {
            pixelData = arrowSprite[1]->mRawData;
        }


        y0 = (arrow->mPosition.mY);
        y1 = 32 + y0;
        x0 = (arrow->mPosition.mX);
        x1 = 32 + x0;

        pixel = 0;

        for (y = y0; y < y1; ++y) {

            if (y < 0 || y >= 200) {
                continue;
            }

            for (x = x0; x < x1; ++x) {
                if (arrow->mDirection == kDirectionRight) {
                    pixel = (pixelData[(32 * (y - y0)) + ((x - x0))]);
                } else {
                    pixel = (pixelData[(32 * (y - y0)) + (31 - (x - x0))]);
                }


                if (pixel == transparency) {
                    continue;
                }

                if (x < 0 || x >= 320) {
                    continue;
                }

                imageBuffer[(320 * y) + (x)] = pixel;
            }
        }
    }

    for (pos = 0; pos < foes.used; ++pos) {
        struct Actor *foe = foes.items[pos];

        if (foe == NULL || !foe->mActive) {
            continue;
        }

        if (foe->mType != kSkeleton &&
            foe->mType != kGargoyle &&
            foe->mType != kHand &&
            foe->mType != kTinhoso &&
            foe->mType != kCapiroto) {
            continue;
        }

        if (foe->mType == kSkeleton) {
            pixelData = foeSprites[counter % 2]->mRawData;
        } else if (foe->mType == kTinhoso) {
            pixelData = tinhosoSprites[counter % 2]->mRawData;
        } else if (foe->mType == kHand) {
            pixelData = handSprites[counter % 2]->mRawData;
        } else if (foe->mType == kCapiroto) {
            pixelData = capirotoSprites[counter % 2]->mRawData;
        } else if (foe->mType == kGargoyle) {
            if (foe->mHealth > 0) {
                pixelData = gargoyleSprites[0]->mRawData;
            } else {
                pixelData = gargoyleSprites[1]->mRawData;
            }
        }

        y0 = (foe->mPosition.mY);
        y1 = 32 + y0;
        x0 = (foe->mPosition.mX);
        x1 = 32 + x0;

        for (y = y0; y < y1; ++y) {

            if (y < 0 || y >= 200) {
                continue;
            }

            for (x = x0; x < x1; ++x) {
                if (foe->mDirection == kDirectionRight) {
                    pixel = (pixelData[(32 * (y - y0)) + ((x - x0))]);
                } else {
                    pixel = (pixelData[(32 * (y - y0)) + (31 - (x - x0))]);
                }


                if (pixel == transparency) {
                    continue;
                }

                if (x < 0 || x >= 320) {
                    continue;
                }

                imageBuffer[(320 * y) + (x)] = pixel;
            }
        }
    }

    for (pos = 0; pos < items.used; ++pos) {
        struct Item *item = items.items[pos];

        if (item == NULL || !item->mActive) {
            continue;
        }

        y0 = (item->mPosition.mY);
        y1 = 32 + y0;
        x0 = (item->mPosition.mX);
        x1 = 32 + x0;
        pixelData = itemSprites[item->mType]->mRawData;
        for (y = y0; y < y1; ++y) {

            if (y < 0 || y >= 200) {
                continue;
            }

            for (x = x0; x < x1; ++x) {
                pixel = (pixelData[(32 * (y - y0)) + ((x - x0))]);

                if (pixel == transparency) {
                    continue;
                }

                if (x < 0 || x >= 320) {
                    continue;
                }

                imageBuffer[(320 * y) + (x)] = pixel;
            }
        }
    }

    if ((hasKey && ((counter % 2) == 0 || paused))) {
        y0 = 2;
        y1 = 32 + y0;
        x0 = 2;
        x1 = 32 + x0;
        pixelData = itemSprites[kKey]->mRawData;

        for (y = y0; y < y1; ++y) {

            if (y < 0 || y >= 200) {
                continue;
            }

            for (x = x0; x < x1; ++x) {
                pixel = (pixelData[(32 * (y - y0)) + ((x - x0))]);

                if (pixel == transparency) {
                    continue;
                }

                if (x < 0 || x >= 320) {
                    continue;
                }

                imageBuffer[(320 * y) + (x)] = pixel;
            }
        }
    }

    if (paused) {
        uint8_t *pixelsPause = pausedSign->mRawData;
        int width = pausedSign->mWidth;
        int height = pausedSign->mHeight;
        int centerX = 320 / 2;
        int centerY = 200 / 2;

        int y = 0;
        for (y = 0; y < height; ++y) {
            int py = centerY - (height / 2) + y;
            int x = 0;
            for (x = 0; x < width; ++x) {
                int px = centerX - (width / 2) + x;

                uint8_t pixel = (pixelsPause[(width * y) + x]);
                imageBuffer[(320 * py) + (px)] = pixel;
            }
        }
    }

    bossHealthColour = 10;
    heroHealthColour = 9;
    backgroundColour = 8;

    if (videoType == kVGA) {
        bossHealthColour = 0xFFFF0000;
        heroHealthColour = 0xFFFFFFFF;
        backgroundColour = 0xFFAAAAAA;
    }

    if (ticksToShowHealth > 0) {
        int y = 192;
        for (y = 192; y < 200; ++y) {
            int x = 0;
            for (x = 0; x < (8 * player.mHealth); ++x) {
                imageBuffer[(320 * y) + (x)] = heroHealthColour;
            }

            for (x = (8 * player.mHealth); x < (80); ++x) {
                imageBuffer[(320 * y) + (x)] = backgroundColour;
            }

        }
    }

    if (hasBossOnScreen) {
        int bossHealth = 0;

        size_t pos = 0;
        for (pos = 0; pos < foes.used; ++pos) {
            struct Actor *foe = foes.items[pos];

            if (foe == NULL || !foe->mActive) {
                continue;
            }

            if (foe->mType == kTinhoso || foe->mType == kCapiroto) {
                bossHealth = foe->mHealth;
            }
        }

        for (y = 184; y < 192; ++y) {
            for (x = 0; x < (8 * bossHealth); ++x) {
                imageBuffer[(320 * y) + (x)] = bossHealthColour;
            }

            for (x = (8 * bossHealth); x < (totalBossHealth * 8); ++x) {
                imageBuffer[(320 * y) + (x)] = backgroundColour;
            }
        }
    }

    copyImageBufferToVideoMemory(&imageBuffer[0]);
}

int done = FALSE;

void sysTick() {
    int isOnGround;
    int isJumping;
    int isUpPressed;
    int isDownPressed;
    int isLeftPressed;
    int isRightPressed;
    int isAttacking;
    int isAltAttackPressed;
    int isPausePressed;
    int isOnStairs;
    struct ControlState controlState;

    beginFrame();

    isOnGround = FALSE;
    isJumping = FALSE;
    isUpPressed = FALSE;
    isDownPressed = FALSE;
    isLeftPressed = FALSE;
    isRightPressed = FALSE;
    isAttacking = FALSE;
    isAltAttackPressed = FALSE;
    isPausePressed = FALSE;
    isOnStairs = FALSE;

    render();

    if (pauseCoolDown > 0 ) {
        --pauseCoolDown;
    }

    if (!paused) {
        gameTick(&isOnGround, &isOnStairs);
    }


    controlState = getControlState();

    if (controlState.sword) {
        isAttacking = TRUE;
    }

    if (controlState.jump) {
        isJumping = TRUE;
    }

    if (controlState.secret) {
        enableSecret = TRUE;
        prepareScreenFor(kIntro);
    }

    if (controlState.escape) {
        done = TRUE;
    }

    if (controlState.jump) {
        isJumping = TRUE;
    }

    if (controlState.moveUp && !isOnStairs && isOnGround) {
        isAltAttackPressed = TRUE;
    }

    if (controlState.moveDown) {
        isDownPressed = TRUE;
    }

    if (controlState.moveUp) {
        isUpPressed = TRUE;
    }

    if (controlState.moveLeft) {
        isLeftPressed = TRUE;
    }

    if (controlState.moveRight) {
        isRightPressed = TRUE;
    }

    if (controlState.fireArrow) {
        isAltAttackPressed = TRUE;
    }

    if (controlState.enter) {
        switch (screen) {
            case kIntro:
                screen = kGame;
                prepareScreenFor(screen);
                init();
                break;
            case kGame:
                isPausePressed = TRUE;
                break;
            case kGameOver:
                screen = kGame;
                prepareScreenFor(screen);
                init();
                break;
            case kVictory:
                screen = kIntro;
                prepareScreenFor(screen);
                break;
        }
    }

    if (screen == kGame) {
        updateHero(isOnGround, isJumping, isUpPressed, isDownPressed, isLeftPressed, isAttacking,
                   isAltAttackPressed, isRightPressed, isOnStairs, isPausePressed);
    }

    doneWithFrame();
}


void loadGraphics() {
    pausedSign = loadBitmap("paused.png", videoType);

    arrowSprite[0] = loadBitmap("arrow.png", videoType);
    arrowSprite[1] = loadBitmap("arrowup.png", videoType);

    doorStates[0] = loadBitmap("door0.png", videoType);
    doorStates[1] = loadBitmap("door1.png", videoType);

    foeSprites[0] = loadBitmap("foe0.png", videoType);
    foeSprites[1] = loadBitmap("foe1.png", videoType);

    itemSprites[0] = loadBitmap("meat.png", videoType);
    itemSprites[1] = loadBitmap("key.png", videoType);

    gargoyleSprites[0] = loadBitmap("garg0.png", videoType);
    gargoyleSprites[1] = loadBitmap("garg1.png", videoType);

    capirotoSprites[0] = loadBitmap("capi0.png", videoType);
    capirotoSprites[1] = loadBitmap("capi1.png", videoType);

    handSprites[0] = loadBitmap("hand0.png", videoType);
    handSprites[1] = loadBitmap("hand0.png", videoType);

    tinhosoSprites[0] = loadBitmap("tinhoso0.png", videoType);
    tinhosoSprites[1] = loadBitmap("tinhoso1.png", videoType);


    hero[0][0] = loadBitmap("up0.png", videoType);
    hero[0][1] = loadBitmap("up1.png", videoType);
    hero[1][0] = loadBitmap("hero0.png", videoType);
    hero[1][1] = loadBitmap("hero1.png", videoType);
    hero[2][0] = loadBitmap("down0.png", videoType);
    hero[2][1] = loadBitmap("down1.png", videoType);
    hero[3][0] = loadBitmap("attack0.png", videoType);
    hero[3][1] = loadBitmap("attack0.png", videoType);
    hero[4][0] = loadBitmap("jump0.png", videoType);
    hero[4][1] = loadBitmap("jump0.png", videoType);
    hero[5][0] = loadBitmap("arrow0.png", videoType);
    hero[5][1] = loadBitmap("arrow1.png", videoType);
}

#ifndef ANDROID

int main(int argc, char **argv) {

    int c = 1;
    for (c = 1; c < argc; ++c) {
        char *parm = argv[c];

        if (!strcmp(parm, "opl2lpt")) {
            initOPL2();
        }

        if (!strcmp(parm, "secret")) {
            enableSecret = TRUE;
        }

        if (!strcmp(parm, "vga")) {
            videoType = kVGA;
        }
    }
    initVideoFor(videoType);
    loadGraphics();
    init();
    prepareScreenFor(kIntro);


#ifndef __EMSCRIPTEN__
    while (!done) {
        soundTick();
        sysTick();
    }
#else
    emscripten_set_main_loop(sysTick, 15, 1);
#endif


    onQuit();

    return 0;
}

#endif
