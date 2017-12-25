#include <string>
#include <cstdlib>
#include <cmath>
#include <array>
#include <memory>
#include <vector>
#include <cstring>
#include <unordered_map>
#include "NativeBitmap.h"
#include "Game.h"
#include "Renderer.h"

using std::vector;

#include "IFileLoaderDelegate.h"
#include "CPackedFileReader.h"
#include "LoadImage.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

bool enableSecret = false;

std::shared_ptr<odb::CPackedFileReader> reader;

std::vector<std::vector<std::shared_ptr<odb::NativeBitmap>>> tiles;

std::shared_ptr<odb::NativeBitmap> pausedSign;

std::shared_ptr<odb::NativeBitmap> arrowSprite[2];

std::shared_ptr<odb::NativeBitmap> doorStates[2];

std::shared_ptr<odb::NativeBitmap> foeSprites[2];

std::shared_ptr<odb::NativeBitmap> itemSprites[2];

std::shared_ptr<odb::NativeBitmap> gargoyleSprites[2];

std::shared_ptr<odb::NativeBitmap> capirotoSprites[2];

std::shared_ptr<odb::NativeBitmap> handSprites[2];

std::shared_ptr<odb::NativeBitmap> tinhosoSprites[2];


std::shared_ptr<odb::NativeBitmap> hero[6][2];

std::array<unsigned int, 320 * 200> imageBuffer;
std::shared_ptr<odb::NativeBitmap> currentScreen = nullptr;

void initOPL2(int instrument) {
    setupOPL2(instrument);
}

void prepareScreenFor(EScreen screenState) {
    muteSound();
    switch (screenState) {
        case kIntro:
            currentScreen = odb::loadBitmap( (enableSecret ? "secret.dat" : "intro.png"), reader, videoType  );
            playTune("eefggfedccdeed12d4eefggfedccdedc12c4ddecde12f12ecde12f12edcdpeefggfedccdedc12c4");
            break;
        case kGame:
            currentScreen = nullptr;
            playMusic(1, "001|cba|cba|cba");
            break;
        case kGameOver:
            currentScreen = odb::loadBitmap( "gameover.png", reader, videoType );
            playTune("gggefffd");
            break;
        case kVictory:
            currentScreen = odb::loadBitmap( "victory.png", reader, videoType );
            playTune("eefggfedccdeeddeefgg");
            break;
    }
}

void clearBuffers() {
    std::fill(std::begin(imageBuffer), std::end(imageBuffer), 4);
}

void loadTiles(std::vector<std::string> tilesToLoad) {
    tiles.clear();

    for (const auto &tile : tilesToLoad) {

        if (tile.substr(tile.length() - 4) == ".png") {
            tiles.push_back({odb::loadBitmap( tile, reader, videoType )});
        } else {
            tiles.push_back(odb::loadSpriteList( tile, reader, videoType));
        }
    }
}

void render() {
    std::fill(std::begin(imageBuffer), std::end(imageBuffer), 4);

    if (currentScreen != nullptr) {

        auto pixelData = currentScreen->getPixelData();

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
            std::shared_ptr<odb::NativeBitmap> tile;
            int *pixelData;
            y0 = (ty * 32);
            y1 = 32 + (ty * 32);
            x0 = (tx * 32);
            x1 = 32 + (tx * 32);
            int pixel = 4;

            if (backgroundTiles[ty][tx] != 0) {
                auto tileset = tiles[backgroundTiles[ty][tx]];
                tile = tileset[counter % tileset.size()];

                pixelData = tile->getPixelData();

                pixel = 4;
                for (int y = y0; y < y1; ++y) {
                    if (y < 0 || y >= 200) {
                        continue;
                    }

                    for (int x = x0; x < x1; ++x) {

                        if (x < 0 || x >= 320) {
                            continue;
                        }

                        pixel = (pixelData[(32 * (y - y0)) + (x - x0)]);

                        if (pixel == 0) {
                            continue;
                        }

                        imageBuffer[(320 * y) + x] = pixel;
                    }
                }
            }

            if (foregroundTiles[ty][tx] != 0) {
                auto tileset = tiles[foregroundTiles[ty][tx]];
                tile = tileset[counter % tileset.size()];

                pixelData = tile->getPixelData();

                pixel = 4;
                for (int y = y0; y < y1; ++y) {

                    if (y < 0 || y >= 200) {
                        continue;
                    }

                    for (int x = x0; x < x1; ++x) {

                        if (x < 0 || x >= 320) {
                            continue;
                        }

                        pixel = (pixelData[(32 * (y - y0)) + (x - x0)]);

                        if (pixel == 0) {
                            continue;
                        }

                        imageBuffer[(320 * y) + x] = pixel;
                    }
                }
            }
        }
    }

    int *pixelData;

    for (const auto &door : doors) {
        pixelData = doorStates[door.mType - EActorType::kClosedDoor]->getPixelData();
        y0 = (door.mPosition.mY);
        y1 = 32 + y0;
        x0 = (door.mPosition.mX);
        x1 = 32 + x0;

        int pixel = 0;
        for (int y = y0; y < y1; ++y) {

            if (y < 0 || y >= 200) {
                continue;
            }

            for (int x = x0; x < x1; ++x) {
                pixel = (pixelData[(32 * (y - y0)) + ((x - x0))]);

                if (pixel == 0) {
                    continue;
                }

                if (x < 0 || x >= 320) {
                    continue;
                }

                imageBuffer[(320 * y) + (x)] = pixel;
            }
        }
    }

    auto sprite = hero[player.mStance][heroFrame];

    if (((ticksUntilVulnerable <= 0) || ((counter % 2) == 0)) || paused) {
        y0 = (player.mPosition.mY);
        int spriteWidth = sprite->getWidth();
        y1 = sprite->getHeight() + y0;
        x0 = (player.mPosition.mX);

        if (player.mDirection == EDirection::kLeft) {
            x0 -= (spriteWidth - 32);
        }

        x1 = spriteWidth + x0;
        pixelData = sprite->getPixelData();

        int pixel = 0;
        for (int y = y0; y < y1; ++y) {

            if (y < 0 || y >= 200) {
                continue;
            }

            for (int x = x0; x < x1; ++x) {
                if (player.mDirection == EDirection::kRight) {
                    pixel = (pixelData[(spriteWidth * (y - y0)) + ((x - x0))]);
                } else {
                    pixel = (pixelData[(spriteWidth * (y - y0)) + ((spriteWidth - 1) - (x - x0))]);
                }

                if (pixel == 0) {
                    continue;
                }

                if (x < 0 || x >= 320) {
                    continue;
                }

                imageBuffer[(320 * y) + (x)] = pixel;
            }
        }
    }

    for (const auto &arrow : arrows) {

        if (std::abs(arrow.mSpeed.mX) > std::abs(arrow.mSpeed.mY) ) {
            pixelData = arrowSprite[0]->getPixelData();
        } else {
            pixelData = arrowSprite[1]->getPixelData();
        }


        y0 = (arrow.mPosition.mY);
        y1 = 32 + y0;
        x0 = (arrow.mPosition.mX);
        x1 = 32 + x0;

        int pixel = 0;
        for (int y = y0; y < y1; ++y) {

            if (y < 0 || y >= 200) {
                continue;
            }

            for (int x = x0; x < x1; ++x) {
                if (arrow.mDirection == EDirection::kRight) {
                    pixel = (pixelData[(32 * (y - y0)) + ((x - x0))]);
                } else {
                    pixel = (pixelData[(32 * (y - y0)) + (31 - (x - x0))]);
                }


                if (pixel == 0) {
                    continue;
                }

                if (x < 0 || x >= 320) {
                    continue;
                }

                imageBuffer[(320 * y) + (x)] = pixel;
            }
        }

    }


    for (const auto &foe : foes) {

        if (foe.mType != EActorType::kSkeleton && foe.mType != EActorType::kGargoyle && foe.mType != EActorType::kHand &&
            foe.mType != EActorType::kTinhoso && foe.mType != EActorType::kCapiroto ) {
            continue;
        }

        if (foe.mType == kSkeleton) {
            pixelData = foeSprites[counter % 2]->getPixelData();
        } else if (foe.mType == kTinhoso) {
            pixelData = tinhosoSprites[counter % 2]->getPixelData();
        } else if (foe.mType == kHand) {
            pixelData = handSprites[counter % 2]->getPixelData();
        } else if (foe.mType == kCapiroto) {
            pixelData = capirotoSprites[counter % 2]->getPixelData();
        } else if (foe.mType == kGargoyle) {
            if (foe.mHealth > 0) {
                pixelData = gargoyleSprites[0]->getPixelData();
            } else {
                pixelData = gargoyleSprites[1]->getPixelData();
            }

        }

        y0 = (foe.mPosition.mY);
        y1 = 32 + y0;
        x0 = (foe.mPosition.mX);
        x1 = 32 + x0;

        int pixel = 0;
        for (int y = y0; y < y1; ++y) {

            if (y < 0 || y >= 200) {
                continue;
            }

            for (int x = x0; x < x1; ++x) {
                if (foe.mDirection == EDirection::kRight) {
                    pixel = (pixelData[(32 * (y - y0)) + ((x - x0))]);
                } else {
                    pixel = (pixelData[(32 * (y - y0)) + (31 - (x - x0))]);
                }


                if (pixel == 0) {
                    continue;
                }

                if (x < 0 || x >= 320) {
                    continue;
                }

                imageBuffer[(320 * y) + (x)] = pixel;
            }
        }
    }

    for (const auto &item : items) {
        y0 = (item.mPosition.mY);
        y1 = 32 + y0;
        x0 = (item.mPosition.mX);
        x1 = 32 + x0;
        pixelData = itemSprites[item.mType]->getPixelData();
        int pixel = 0;
        for (int y = y0; y < y1; ++y) {

            if (y < 0 || y >= 200) {
                continue;
            }

            for (int x = x0; x < x1; ++x) {
                pixel = (pixelData[(32 * (y - y0)) + ((x - x0))]);

                if (pixel == 0) {
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
        pixelData = itemSprites[kKey]->getPixelData();
        int pixel = 0;
        for (int y = y0; y < y1; ++y) {

            if (y < 0 || y >= 200) {
                continue;
            }

            for (int x = x0; x < x1; ++x) {
                pixel = (pixelData[(32 * (y - y0)) + ((x - x0))]);

                if (pixel == 0) {
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
        auto pixelsPause = pausedSign->getPixelData();
        auto width = pausedSign->getWidth();
        auto height = pausedSign->getHeight();
        auto centerX = 320 / 2;
        auto centerY = 200 / 2;

        for ( int y = 0; y < height; ++y ) {
            auto py = centerY - (height / 2) + y;
            for ( int x = 0; x < width; ++x ) {
                auto px = centerX - (width / 2) + x;

                auto pixel = (pixelsPause[(width * y) + x]);
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

        for (auto const &foe : foes) {
            if (foe.mType == kTinhoso || foe.mType == kCapiroto) {
                bossHealth = foe.mHealth;
            }
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
    } else {
        muteSound();
    }


    auto controlState = getControlState();

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
    reader = std::make_shared<odb::CPackedFileReader>(getAssetsPath());

    pausedSign = odb::loadBitmap("paused.png", reader, videoType);

    arrowSprite[0] = odb::loadBitmap("arrow.png",  reader, videoType);
    arrowSprite[1] = odb::loadBitmap("arrowup.png",  reader, videoType);

    doorStates[0] = odb::loadBitmap("door0.png",  reader, videoType);
    doorStates[1] = odb::loadBitmap("door1.png",  reader, videoType);

    foeSprites[0] = odb::loadBitmap("foe0.png",  reader, videoType);
    foeSprites[1] = odb::loadBitmap("foe1.png",  reader, videoType);

    itemSprites[0] = odb::loadBitmap("meat.png",  reader, videoType);
    itemSprites[1] = odb::loadBitmap("key.png",  reader, videoType);

    gargoyleSprites[0] = odb::loadBitmap("garg0.png",  reader, videoType);
    gargoyleSprites[1] = odb::loadBitmap("garg1.png",  reader, videoType);

    capirotoSprites[0] = odb::loadBitmap("capi0.png",  reader, videoType);
    capirotoSprites[1] = odb::loadBitmap("capi1.png",  reader, videoType);

    handSprites[0] = odb::loadBitmap("hand0.png",  reader, videoType);
    handSprites[1] = odb::loadBitmap("hand0.png",  reader, videoType);

    tinhosoSprites[0] = odb::loadBitmap("tinhoso0.png",  reader, videoType);
    tinhosoSprites[1] = odb::loadBitmap("tinhoso1.png",  reader, videoType);


    hero[0][0] = odb::loadBitmap( "up0.png",  reader, videoType);
    hero[0][1] = odb::loadBitmap( "up1.png",  reader, videoType);
    hero[1][0] = odb::loadBitmap( "hero0.png",  reader, videoType);
    hero[1][1] = odb::loadBitmap( "hero1.png",  reader, videoType);
    hero[2][0] = odb::loadBitmap( "down0.png",  reader, videoType);
    hero[2][1] = odb::loadBitmap( "down1.png",  reader, videoType);
    hero[3][0] = odb::loadBitmap( "attack0.png", reader, videoType);
    hero[3][1] = odb::loadBitmap( "attack0.png", reader, videoType);
    hero[4][0] = odb::loadBitmap( "jump0.png", reader, videoType);
    hero[4][1] = odb::loadBitmap( "jump0.png",  reader, videoType);
    hero[5][0] = odb::loadBitmap( "up0.png",  reader, videoType);
    hero[5][1] = odb::loadBitmap( "up1.png",  reader, videoType);
}

int main(int argc, char **argv) {

    if ( argc >= 2 ) {

        if ( !std::strcmp(argv[1], "opl2lpt")) {
            int instrument = 80;

            if (argc >= 3 ) {
                instrument = atoi(argv[2]);
            }

            initOPL2(instrument);
        }

        if ( !std::strcmp(argv[1], "secret")) {
            enableSecret = true;
        }

        if ( !std::strcmp(argv[1], "vga")) {
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
    emscripten_set_main_loop(sysTick, 20, 1);
#endif


    onQuit();

    return 0;
}
