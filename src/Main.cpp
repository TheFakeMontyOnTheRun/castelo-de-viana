#include <assert.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "Common.h"
#include "NativeBitmap.h"
#include "Game.h"
#include "Renderer.h"

#include "IFileLoaderDelegate.h"
#include "CPackedFileReader.h"
#include "LoadImage.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

bool enableSecret = false;

odb::ItemVector tiles;

odb::NativeBitmap* pausedSign;

odb::NativeBitmap* arrowSprite[2];

odb::NativeBitmap* doorStates[2];

odb::NativeBitmap* foeSprites[2];

odb::NativeBitmap* itemSprites[2];

odb::NativeBitmap* gargoyleSprites[2];

odb::NativeBitmap* capirotoSprites[2];

odb::NativeBitmap* handSprites[2];

odb::NativeBitmap* tinhosoSprites[2];


odb::NativeBitmap* hero[6][2];

uint8_t imageBuffer[ 320 * 200 ];
odb::NativeBitmap* currentScreen = NULL;

void initOPL2() {
    setupOPL2();
}

void prepareScreenFor(EScreen screenState) {
    muteSound();
    switch (screenState) {
        case kIntro:
            currentScreen = odb::loadBitmap( (enableSecret ? "secret.dat" : "intro.png"), videoType  );
            playTune("eefggfedccdeedd");
            break;
        case kGame:
            currentScreen = NULL;
            break;
        case kGameOver:
            currentScreen = odb::loadBitmap( "gameover.png", videoType );
            playTune("gggefffd");
            break;
        case kVictory:
            currentScreen = odb::loadBitmap( "victory.png", videoType );
            playTune("eefggfedccdeed12d4eefggfedccdedc12c4ddecde12f12ecde12f12edcdpeefggfedccdedc12c4");
            break;
    }
}

void clearBuffers() {
    memset( imageBuffer, 4, 320 * 200 );
}

void loadTiles(odb::ItemVector tilesToLoad) {

    char** ptr = (char**)tilesToLoad.items;
    odb::initVector( &tiles, tilesToLoad.used );

    for ( size_t pos = 0; pos < tilesToLoad.used; ++pos ) {

        char* tile = *ptr;

        odb::ItemVector* strip = (odb::ItemVector*)calloc(sizeof(odb::ItemVector), 1);


        if ( !strcmp( tile + (strlen(tile) - 4), ".png" ) ) {
            odb::initVector( strip, 1 );
            odb::pushVector( strip, odb::loadBitmap( tile, videoType ) );
            odb::pushVector( &tiles, strip);
        } else {
            odb::pushVector( &tiles, odb::loadSpriteList( tile, videoType));
        }

        ++ptr;
    }
}

void render() {

    uint8_t transparency;

    if ( videoType == kCGA ) {
        memset( imageBuffer, 4, 320 * 200 );
        transparency = 0;
    } else {
        memset( imageBuffer, 0, 320 * 200 );
        transparency = getPaletteEntry(0x00FF00);
    }


    if (currentScreen != NULL) {

        uint8_t* pixelData = currentScreen->mRawData;

        for (int c = 0; c < 320 * 200; ++c) {
            imageBuffer[c] = pixelData[c];
        }

        copyImageBufferToVideoMemory(imageBuffer);
        return;
    }


    int y0 = 0;
    int y1 = 0;
    int x0 = 0;
    int x1 = 0;


    for (int ty = 0; ty < 6; ++ty) {
        for (int tx = 0; tx < 10; ++tx) {
            odb::NativeBitmap* tile;
            uint8_t *pixelData;
            y0 = (ty * 32);
            y1 = 32 + (ty * 32);
            x0 = (tx * 32);
            x1 = 32 + (tx * 32);
            int pixel = 4;

            if (backgroundTiles[ty][tx] != 0) {
                odb::ItemVector *tileset = (odb::ItemVector*)tiles.items[backgroundTiles[ty][tx]];
                tile = (odb::NativeBitmap*)tileset->items[counter % tileset->used];

                pixelData = tile->mRawData;

                pixel = 4;
                for (int y = y0; y < y1; ++y) {

                    uint8_t* sourceLine = pixelData + (32 * (y - y0));
                    uint8_t* destLine = &imageBuffer[0] + (320 * y) + x0;

                    for (int x = x0; x < x1; ++x) {

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
                odb::ItemVector *tileset = (odb::ItemVector*)tiles.items[foregroundTiles[ty][tx]];
                tile = (odb::NativeBitmap*)tileset->items[counter % tileset->used];
                pixelData = tile->mRawData;

                pixel = 4;
                for (int y = y0; y < y1; ++y) {

                    uint8_t* sourceLine = pixelData + (32 * (y - y0));
                    uint8_t* destLine = &imageBuffer[0] + (320 * y) + x0;

                    for (int x = x0; x < x1; ++x) {

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

    uint8_t *pixelData;

    Actor** doorPtr = (Actor**)doors.items;

    for (size_t pos = 0; pos < doors.used; ++ pos ) {
        Actor* door = *doorPtr;

        pixelData = doorStates[door->mType - kClosedDoor]->mRawData;
        y0 = (door->mPosition.mY);
        y1 = 32 + y0;
        x0 = (door->mPosition.mX);
        x1 = 32 + x0;

        int pixel = 0;
        for (int y = y0; y < y1; ++y) {

            if (y < 0 || y >= 200) {
                continue;
            }

            for (int x = x0; x < x1; ++x) {
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
        ++doorPtr;
    }

    odb::NativeBitmap* sprite = hero[player.mStance][heroFrame];

    if (((ticksUntilVulnerable <= 0) || ((counter % 2) == 0)) || paused) {
        y0 = (player.mPosition.mY);
        int spriteWidth = sprite->mWidth;
        y1 = sprite->mHeight + y0;
        x0 = (player.mPosition.mX);

        if (player.mDirection == kDirectionLeft) {
            x0 -= (spriteWidth - 32);
        }

        x1 = spriteWidth + x0;
        pixelData = sprite->mRawData;

        int pixel = 0;
        for (int y = y0; y < y1; ++y) {

            if (y < 0 || y >= 200) {
                continue;
            }

            for (int x = x0; x < x1; ++x) {
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

    Actor** arrowPtr = (Actor**)arrows.items;

    for (size_t pos = 0; pos < arrows.used; ++pos ) {

        Actor* arrow = *arrowPtr;

        if ( !arrow->mActive ) {
            continue;
        }

        if (std::abs(arrow->mSpeed.mX) > std::abs(arrow->mSpeed.mY) ) {
            pixelData = arrowSprite[0]->mRawData;
        } else {
            pixelData = arrowSprite[1]->mRawData;
        }


        y0 = (arrow->mPosition.mY);
        y1 = 32 + y0;
        x0 = (arrow->mPosition.mX);
        x1 = 32 + x0;

        int pixel = 0;
        for (int y = y0; y < y1; ++y) {

            if (y < 0 || y >= 200) {
                continue;
            }

            for (int x = x0; x < x1; ++x) {
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
        ++arrowPtr;
    }

    Actor** foePtr = (Actor**)foes.items;
    for (size_t pos = 0; pos < foes.used; ++pos ) {

        Actor* foe = *foePtr;

		if (!foe->mActive) {
			continue;
		}

		if (foe->mType != kSkeleton &&
		    foe->mType != kGargoyle &&
		    foe->mType != kHand &&
            foe->mType != kTinhoso &&
            foe->mType != kCapiroto ) {
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

        int pixel = 0;
        for (int y = y0; y < y1; ++y) {

            if (y < 0 || y >= 200) {
                continue;
            }

            for (int x = x0; x < x1; ++x) {
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
        foePtr++;
    }

    Item** itemPtr = (Item**)items.items;
    for (size_t pos = 0; pos < items.used; ++pos ) {
        Item* item = *itemPtr;

        if (!item->mActive ) {
            continue;
        }

        y0 = (item->mPosition.mY);
        y1 = 32 + y0;
        x0 = (item->mPosition.mX);
        x1 = 32 + x0;
        pixelData = itemSprites[item->mType]->mRawData;
        int pixel = 0;
        for (int y = y0; y < y1; ++y) {

            if (y < 0 || y >= 200) {
                continue;
            }

            for (int x = x0; x < x1; ++x) {
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
        ++itemPtr;
    }

    if ((hasKey && ((counter % 2) == 0 || paused))) {
        y0 = 2;
        y1 = 32 + y0;
        x0 = 2;
        x1 = 32 + x0;
        pixelData = itemSprites[kKey]->mRawData;
        int pixel = 0;
        for (int y = y0; y < y1; ++y) {

            if (y < 0 || y >= 200) {
                continue;
            }

            for (int x = x0; x < x1; ++x) {
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
        uint8_t* pixelsPause = pausedSign->mRawData;
        int width = pausedSign->mWidth;
        int height = pausedSign->mHeight;
        int centerX = 320 / 2;
        int centerY = 200 / 2;

        for ( int y = 0; y < height; ++y ) {
            int py = centerY - (height / 2) + y;
            for ( int x = 0; x < width; ++x ) {
                int px = centerX - (width / 2) + x;

                uint8_t pixel = (pixelsPause[(width * y) + x]);
                imageBuffer[(320 * py) + (px)] = pixel;
            }
        }
    }

    int bossHealthColour = 10;
    int heroHealthColour = 9;
    int backgroundColour = 8;

    if (videoType == kVGA ) {
        bossHealthColour = 0xFFFF0000;
        heroHealthColour = 0xFFFFFFFF;
        backgroundColour = 0xFFAAAAAA;
    }

    if (ticksToShowHealth > 0) {
        for ( int y = 192; y < 200; ++y ) {
            for ( int x = 0; x < (8 * player.mHealth); ++x ) {
                imageBuffer[(320 * y) + (x)] = heroHealthColour;
            }

            for ( int x = (8 * player.mHealth); x < (80); ++x ) {
                imageBuffer[(320 * y) + (x)] = backgroundColour;
            }

        }
    }

    if (hasBossOnScreen) {
        int bossHealth = 0;

        Actor** foePtr = (Actor**)foes.items;
        for (size_t pos = 0; pos < foes.used; ++pos ) {
            Actor* foe = *foePtr;

			if (!foe->mActive) {
				continue;
			}

			if (foe->mType == kTinhoso || foe->mType == kCapiroto) {
                bossHealth = foe->mHealth;
            }

			++foePtr;
        }

        for ( int y = 184; y < 192; ++y ) {
            for ( int x = 0; x < (8 * bossHealth ); ++x ) {
                imageBuffer[(320 * y) + (x)] = bossHealthColour;
            }

            for ( int x = (8 * bossHealth); x < (totalBossHealth * 8); ++x ) {
                imageBuffer[(320 * y) + (x)] = backgroundColour;
            }
        }
    }

    copyImageBufferToVideoMemory(imageBuffer);
}

bool done = false;

void sysTick() {
    beginFrame();

    bool isOnGround = false;
    bool isJumping = false;
    bool isUpPressed = false;
    bool isDownPressed = false;
    bool isLeftPressed = false;
    bool isRightPressed = false;
    bool isAttacking = false;
    bool isAltAttackPressed = false;
    bool isPausePressed = false;
    bool isOnStairs = false;
    render();


    if (!paused) {
        gameTick(isOnGround, isOnStairs);
    }


    ControlState controlState = getControlState();

    if ( controlState.sword ) {
        isAttacking = true;
    }

    if ( controlState.jump ) {
        isJumping = true;
    }

    if ( controlState.secret ) {
        enableSecret = true;
        prepareScreenFor(kIntro);
    }

    if ( controlState.escape ) {
        done = true;
    }

    if ( controlState.jump ) {
        isJumping = true;
    }

    if ( controlState.moveUp && !isOnStairs && isOnGround ) {
        isAltAttackPressed = true;
    }

    if ( controlState.moveDown ) {
        isDownPressed = true;
    }

    if ( controlState.moveUp ) {
        isUpPressed = true;
    }

    if ( controlState.moveLeft ) {
        isLeftPressed = true;
    }

    if ( controlState.moveRight ) {
        isRightPressed = true;
    }

    if ( controlState.fireArrow ) {
        isAltAttackPressed = true;
    }

    if ( controlState.enter ) {
        switch (screen) {
            case kIntro:
                screen = kGame;
                prepareScreenFor(screen);
                init();
                break;
            case kGame:
                isPausePressed = true;
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
    pausedSign = odb::loadBitmap("paused.png", videoType);

    arrowSprite[0] = odb::loadBitmap("arrow.png", videoType);
    arrowSprite[1] = odb::loadBitmap("arrowup.png", videoType);

    doorStates[0] = odb::loadBitmap("door0.png", videoType);
    doorStates[1] = odb::loadBitmap("door1.png", videoType);

    foeSprites[0] = odb::loadBitmap("foe0.png", videoType);
    foeSprites[1] = odb::loadBitmap("foe1.png", videoType);

    itemSprites[0] = odb::loadBitmap("meat.png", videoType);
    itemSprites[1] = odb::loadBitmap("key.png", videoType);

    gargoyleSprites[0] = odb::loadBitmap("garg0.png", videoType);
    gargoyleSprites[1] = odb::loadBitmap("garg1.png", videoType);

    capirotoSprites[0] = odb::loadBitmap("capi0.png", videoType);
    capirotoSprites[1] = odb::loadBitmap("capi1.png", videoType);

    handSprites[0] = odb::loadBitmap("hand0.png", videoType);
    handSprites[1] = odb::loadBitmap("hand0.png", videoType);

    tinhosoSprites[0] = odb::loadBitmap("tinhoso0.png", videoType);
    tinhosoSprites[1] = odb::loadBitmap("tinhoso1.png", videoType);


    hero[0][0] = odb::loadBitmap( "up0.png", videoType);
    hero[0][1] = odb::loadBitmap( "up1.png", videoType);
    hero[1][0] = odb::loadBitmap( "hero0.png", videoType);
    hero[1][1] = odb::loadBitmap( "hero1.png", videoType);
    hero[2][0] = odb::loadBitmap( "down0.png", videoType);
    hero[2][1] = odb::loadBitmap( "down1.png", videoType);
    hero[3][0] = odb::loadBitmap( "attack0.png", videoType);
    hero[3][1] = odb::loadBitmap( "attack0.png", videoType);
    hero[4][0] = odb::loadBitmap( "jump0.png", videoType);
    hero[4][1] = odb::loadBitmap( "jump0.png", videoType);
    hero[5][0] = odb::loadBitmap( "arrow0.png", videoType);
    hero[5][1] = odb::loadBitmap( "arrow1.png", videoType);
}

int main(int argc, char **argv) {

    for ( int c = 1; c < argc; ++c ) {
        char* parm = argv[ c ];

        if ( !strcmp(parm, "opl2lpt")) {
            initOPL2();
        }

        if ( !strcmp(parm, "secret")) {
            enableSecret = true;
        }

        if ( !strcmp(parm, "vga")) {
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
