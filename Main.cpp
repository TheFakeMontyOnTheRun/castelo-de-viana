#include <algorithm>
#include <array>
#include <random>
#include <memory>
#include <fstream>
#include <sstream>
#include <cstring>
#include "NativeBitmap.h"
#include "LoadImage.h"

#include "Game.h"
#include "Renderer.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

bool enableSecret = false;

std::vector<std::vector<std::shared_ptr<odb::NativeBitmap>>> tiles;

std::shared_ptr<odb::NativeBitmap> pausedSign = {
        odb::loadBitmap(getResPath() + "paused.png")
};

std::shared_ptr<odb::NativeBitmap> arrowSprite[2] = {
        odb::loadBitmap(getResPath() + "arrow.png"),
        odb::loadBitmap(getResPath() + "arrowup.png")
};

std::shared_ptr<odb::NativeBitmap> doorStates[2] = {
        odb::loadBitmap(getResPath() + "door0.png"),
        odb::loadBitmap(getResPath() + "door1.png"),
};

std::shared_ptr<odb::NativeBitmap> foeSprites[2] = {
        odb::loadBitmap(getResPath() + "foe0.png"),
        odb::loadBitmap(getResPath() + "foe1.png"),
};

std::shared_ptr<odb::NativeBitmap> itemSprites[2] = {
        odb::loadBitmap(getResPath() + "meat.png"),
        odb::loadBitmap(getResPath() + "key.png"),
};

std::shared_ptr<odb::NativeBitmap> gargoyleSprites[2] = {
        odb::loadBitmap(getResPath() + "garg0.png"),
        odb::loadBitmap(getResPath() + "garg1.png"),
};

std::shared_ptr<odb::NativeBitmap> capirotoSprites[2] = {
        odb::loadBitmap(getResPath() + "capi0.png"),
        odb::loadBitmap(getResPath() + "capi1.png"),
};

std::shared_ptr<odb::NativeBitmap> handSprites[2] = {
        odb::loadBitmap(getResPath() + "hand0.png"),
        odb::loadBitmap(getResPath() + "hand0.png"),
};

std::shared_ptr<odb::NativeBitmap> tinhosoSprites[2] = {
        odb::loadBitmap(getResPath() + "tinhoso0.png"),
        odb::loadBitmap(getResPath() + "tinhoso1.png"),
};


std::shared_ptr<odb::NativeBitmap> hero[6][2] = {
        {
                odb::loadBitmap( getResPath() + "up0.png"),
                odb::loadBitmap( getResPath() + "up1.png"),
        },
        {
                odb::loadBitmap( getResPath() + "hero0.png"),
                odb::loadBitmap( getResPath() + "hero1.png"),
        },
        {
                odb::loadBitmap( getResPath() + "down0.png"),
                odb::loadBitmap( getResPath() + "down1.png"),
        },
        {
                odb::loadBitmap( getResPath() + "attack0.png"),
                odb::loadBitmap( getResPath() + "attack0.png"),
        },
        {
                odb::loadBitmap( getResPath() + "jump0.png"),
                odb::loadBitmap( getResPath() + "jump0.png"),
        },
        {
                odb::loadBitmap( getResPath() + "up0.png"),
                odb::loadBitmap( getResPath() + "up1.png"),
        },
};

std::array<unsigned int, 320 * 200> imageBuffer;
std::shared_ptr<odb::NativeBitmap> currentScreen = nullptr;


void initOPL2(int instrument) {
    setupOPL2(instrument);
}

void prepareScreenFor(EScreen screenState) {
    muteSound();
    switch (screenState) {
        case kIntro:
            currentScreen = odb::loadBitmap( getResPath() + (enableSecret ? "secret.dat" : "intro.png") );
//            playMusic(
//                    "e5r1e3r0d3r0e3r0e1r0d1r0-g4r1f3r0f1r0f1r0a3r0f1r0e1r0d1r0d1r0e5r0c3r0c1r0c1r0e3r0c1r0-b1r0c1r0-b1r0-a1r0-a1-b5r0e1r0e1r0e1r0e1r0e1r0e1r0d1r0e1r0e1r0e1r0d1r0-a1r0-a1r0b3r1-a1r0-b1r0c1r0d1r0e1r0f1r0e1r0f3r1a3r1b1r0a1r0f3r0e3r0e1r0e4r0|e5r1e3r0d3r0e3r0e1r0d1r0-g4r1f3r0f1r0f1r0a3r0f1r0e1r0d1r0d1r0e5r0c3r0c1r0c1r0e3r0c1r0-b1r0c1r0-b1r0-a1r0-a1-b5r0e1r0e1r0e1r0e1r0e1r0e1r0d1r0e1r0e1r0e1r0d1r0-a1r0-a1r0b3r1-a1r0-b1r0c1r0d1r0e1r0f1r0e1r0f3r1a3r1b1r0a1r0f3r0e3r0e1r0e4r0|e5r1e3r0d3r0e3r0e1r0d1r0-g4r1f3r0f1r0f1r0a3r0f1r0e1r0d1r0d1r0e5r0c3r0c1r0c1r0e3r0c1r0-b1r0c1r0-b1r0-a1r0-a1-b5r0e1r0e1r0e1r0e1r0e1r0e1r0d1r0e1r0e1r0e1r0d1r0-a1r0-a1r0b3r1-a1r0-b1r0c1r0d1r0e1r0f1r0e1r0f3r1a3r1b1r0a1r0f3r0e3r0e1r0e4r0");
            break;
        case kGame:
            currentScreen = nullptr;
            break;
        case kGameOver:
            currentScreen = odb::loadBitmap( getResPath() + "gameover.png");
//            playMusic("mbt180o2p2p8l8gggl2e-p24p8l8fffl2d");
            break;
        case kVictory:
            currentScreen = odb::loadBitmap( getResPath() + "victory.png");
//            playMusic(
//                    "e8e8f8g8g8f8e8d8c8c8d8e8e8d12d4e8e8f8g8g8f8e8d8c8c8d8e8d8c12c4d8d8e8c8d8e12f12e8c8d8e12f12e8d8c8d8p8e8e8f8g8g8f8e8d8c8c8d8e8d8c12c4");
            break;

    }
}

void clearBuffers() {
    std::fill(std::begin(imageBuffer), std::end(imageBuffer), 4);
}

std::vector<std::shared_ptr<odb::NativeBitmap>> loadSpriteList(std::string listName) {
    std::ifstream tileList(listName);
    std::string buffer;

    std::vector<std::shared_ptr<odb::NativeBitmap>> tilesToLoad;

    while (tileList.good()) {
        std::getline(tileList, buffer);
        tilesToLoad.push_back(odb::loadBitmap( getResPath() + buffer));
    }
    return tilesToLoad;
}

void loadTiles(std::vector<std::string> tilesToLoad) {
    tiles.clear();

    for (const auto &tile : tilesToLoad) {

        if (tile.substr(tile.length() - 4) == ".png") {
            tiles.push_back({odb::loadBitmap( getResPath() + tile)});
        } else {
            tiles.push_back(loadSpriteList( getResPath() + tile));
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

#ifdef CGA
    int bossHealthColour = 10;
    int heroHealthColour = 9;
    int backgroundColour = 8;
#else
    int bossHealthColour = 0xFFFF0000;
    int heroHealthColour = 0xFFFFFFFF;
    int backgroundColour = 0xFFAAAAAA;
#endif

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
    bool isOnStairs;
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
                screen = kIntro;
                prepareScreenFor(screen);
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

int main(int argc, char **argv) {

    if ( argc >= 2 ) {
      //  enableSecret = true;
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
    }

    init();
    prepareScreenFor(kIntro);


    initVideo();

#ifndef __EMSCRIPTEN__
    while (!done) {
        sysTick();
    }
#else
    emscripten_set_main_loop(sysTick, 20, 1);
#endif


    onQuit();

    return 0;
}
