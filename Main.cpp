#include <algorithm>
#include <array>
#include <random>
#include <memory>
#include <fstream>
#include <sstream>
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


void prepareScreenFor(EScreen screenState) {
    muteSound();
    switch (screenState) {
        case kIntro:
            currentScreen = odb::loadBitmap( getResPath() + (enableSecret ? "secret.dat" : "intro.png") );
            playMusic(
                    "E5R1E3R0D3R0E3R0E1R0D1R0-G4R1F3R0F1R0F1R0A3R0F1R0E1R0D1R0D1R0E5R0C3R0C1R0C1R0E3R0C1R0-B1R0C1R0-B1R0-A1R0-A1-B5R0E1R0E1R0E1R0E1R0E1R0E1R0D1R0E1R0E1R0E1R0D1R0-A1R0-A1R0B3R1-A1R0-B1R0C1R0D1R0E1R0F1R0E1R0F3R1A3R1B1R0A1R0F3R0E3R0E1R0E4R0");
            break;
        case kGame:
            currentScreen = nullptr;
            break;
        case kGameOver:
            currentScreen = odb::loadBitmap( getResPath() + "gameover.png");
            playMusic("MBT180o2P2P8L8GGGL2E-P24P8L8FFFL2D");
            break;
        case kVictory:
            currentScreen = odb::loadBitmap( getResPath() + "victory.png");
            playMusic(
                    "e8e8f8g8g8f8e8d8c8c8d8e8e8d12d4e8e8f8g8g8f8e8d8c8c8d8e8d8c12c4d8d8e8c8d8e12f12e8c8d8e12f12e8d8c8d8p8e8e8f8g8g8f8e8d8c8c8d8e8d8c12c4");
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

    copyImageBufferToVideoMemory(imageBuffer);

    if (paused) {
    }

    if (ticksToShowHealth > 0) {
    }

    if (hasBossOnScreen) {
    }
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
        enableSecret = true;
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
