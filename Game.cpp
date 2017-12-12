#include <time.h>
#include <unistd.h>
#include <array>
#include <memory>
#include <vector>
#include <algorithm>

#include "Game.h"
#include "Renderer.h"
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

std::array<std::array<int, 10>, 6> backgroundTiles;
std::array<std::array<int, 10>, 6> foregroundTiles;
std::vector<Actor> foes;
std::vector<Actor> doors;
std::vector<Item> items;
std::vector<Actor> arrows;

auto hurtSound = "053|dca|dca|";
auto swordSound = "044|dgd|dgd|dgd";
auto arrowSound = "044|abc|abc|abc";
auto jumpSound = "046|cdefedc|cdefedc|cdefedc";
auto pickSound = "054|abfc|abfc|abfc";

int totalBossHealth = 0;
std::string currentBossName;
EScreen screen = kIntro;

void evalutePlayerAttack();

void init() {
    player.mPosition.mX = 0;
    player.mPosition.mY = 0;
    player.mSpeed.mX = 0;
    player.mSpeed.mY = 0;
    player.mHealth = 10;
    player.mType = EActorType::kPlayer;
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
            playMusic(jumpSound);
        }
        player.mStance = kJumping;
    }

    if (isUpPressed) {
        if (isOnStairs) {
            player.mSpeed.mY = -8;
            player.mStance = kClimbing;
        } else if (isOnGround) {
            if ( !isOnStairs && arrowCooldown <= 0) {
                Actor a;
                a.mType = kArrow;
                a.mPosition = player.mPosition;
                a.mSpeed = {0, -16};
                a.mDirection = player.mDirection;
                arrows.push_back(a);
                player.mStance = kUp;
                arrowCooldown = 4;
                playMusic(arrowSound);
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
        player.mDirection = kLeft;
        if (isOnGround) {
            player.mStance = kStanding;
        }
    }

    if (isRightPressed) {
        player.mSpeed.mX = +8;
        player.mDirection = kRight;
        if (isOnGround) {
            player.mStance = kStanding;
        }
    }

    if (isAttacking) {
        player.mStance = kAttacking;
        playMusic(swordSound);
    }

    if (isUsingSpecial && arrowCooldown <= 0) {
        Actor a;
        a.mType = kArrow;
        a.mPosition = player.mPosition;
        a.mSpeed = {player.mDirection == kRight ? 16 : -16, 0};
        a.mDirection = player.mDirection;
        arrows.push_back(a);
        player.mStance = kAltAttacking;
        arrowCooldown = 4;
        playMusic(arrowSound);
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

    if (actor.mDirection == EDirection::kRight) {
        front++;
    }

    return (foregroundTiles[(actor.mPosition.mY + 16) / 32][front] == 1);
}

bool collide(const Actor &a, const Item &b, int tolerance = 32) {
    if (std::abs(a.mPosition.mY - b.mPosition.mY) < tolerance) {

        if (a.mDirection == EDirection::kRight) {
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

        if (b.mDirection == EDirection::kRight) {
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

void removeFrom(std::vector<Actor> &mainCollection, std::vector<Actor> &removeList) {
    mainCollection.erase(std::remove_if(std::begin(mainCollection), std::end(mainCollection),
                                        [&](Actor x) {
                                            return std::find(std::begin(removeList), std::end(removeList), x) !=
                                                   std::end(removeList);
                                        }
    ), std::end(mainCollection));
}


void removeFrom(std::vector<Item> &mainCollection, std::vector<Item> &removeList) {
    mainCollection.erase(std::remove_if(std::begin(mainCollection), std::end(mainCollection),
                                        [&](Item x) {
                                            return std::find(std::begin(removeList), std::end(removeList), x) !=
                                                   std::end(removeList);
                                        }
    ), std::end(mainCollection));
}


bool isOnHarmfulBlock(const Actor &actor) {
    int block = foregroundTiles[((actor.mPosition.mY + 16) / 32)][(actor.mPosition.mX + 16) / 32];
    return (block == 5);
}

void hurtPlayer(int ammount) {
    player.mHealth -= ammount;
    ticksUntilVulnerable = 14;
    ticksToShowHealth = 14;
    playMusic(hurtSound);
}

bool isOnDoor(const Actor &actor) {
    for (auto &door : doors) {
        if (door.mType == kOpenDoor && collide(door, actor)) {
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

    if (player.mStance == EStance::kAttacking) {
        evalutePlayerAttack();

        player.mStance = EStance::kStanding;
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

    if (player.mSpeed.mX == 0 && player.mSpeed.mY == 0 ) {
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

    if ((player.mSpeed.mX != 0 && isOnGround)) {
        heroFrame = (heroFrame + 1) % 2;
    }

    if (isOnGround) {
        player.mSpeed.mX = player.mSpeed.mX / 2;
        player.mPosition.mY = (player.mPosition.mY / 32) * 32;
    }

    if (isOnGround && !isOnStairs) {
        player.mSpeed.mY = std::min(0, player.mSpeed.mY);
        player.mPosition.mY = std::min(player.mPosition.mY, (player.mPosition.mY / 32) * 32);
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

    std::vector<Actor> actorsToRemove;

    for (auto &arrow : arrows) {
        arrow.mPosition.mX += arrow.mSpeed.mX;
        arrow.mPosition.mY += arrow.mSpeed.mY;

        if (isBlockedByWall(arrow)) {
            actorsToRemove.push_back(arrow);
            continue;
        }

        for (auto &foe : foes) {
            if ( foe.mType != kHand && collide(foe, arrow, 32)) {
                foe.mHealth--;
                actorsToRemove.push_back(arrow);

                if (foe.mType == kGargoyle) {
                    for (auto &door : doors) {
                        door.mType = EActorType::kOpenDoor;
                    }
                }

            }
        }
    }

    removeFrom(arrows, actorsToRemove);

    std::vector<Item> itemsToRemove;

    for (const auto &item : items) {

        if (collide(player, item)) {
            if (item.mType == kKey && !hasKey) {
                hasKey = true;
                itemsToRemove.push_back(item);
                playMusic(pickSound);
                for (auto &door : doors) {
                    door.mType = EActorType::kOpenDoor;
                }
            } else if (item.mType == kMeat) {
                if (player.mHealth < 10) {
                    playMusic(pickSound);
                    itemsToRemove.push_back(item);
                    player.mHealth = 10;
                    playMusic(pickSound);
                }
                ticksToShowHealth = 14;
            }
        }
    }

    removeFrom(items, itemsToRemove);

    actorsToRemove.clear();

    for (auto &foe : foes) {

        if (foe.mType == kSpawner) {
            if ( ( counter % 40 ) == 0 && ( foes.size() <= 5 ) ) {
                Actor a;
                a.mType = EActorType::kSkeleton;
                a.mPosition = Vec2i( foe.mPosition );
                a.mSpeed.mX = 8;
                a.mHealth = 2;
                foes.push_back(a);
            }
            continue;
        }


        if (( counter % 5 ) == 0 && foe.mType == kHand) {
            int dx = player.mPosition.mX - foe.mPosition.mX;
            int dy = player.mPosition.mY - foe.mPosition.mY;

            if ( dx > 0 ) {
                foe.mSpeed.mX = 1;
            } else {
                foe.mSpeed.mX = -1;
            }

            if ( dy > 0 ) {
                foe.mSpeed.mY = 1;
            } else {
                foe.mSpeed.mY = -1;
            }

            switch ( foe.mDirection ) {
                case kLeft:
                    if ( foe.mPosition.mX < 160 ) {
                        foe.mSpeed.mX = -foe.mSpeed.mX;
                    }
                    break;
                case kRight:
                    if (  160 <= foe.mPosition.mX) {
                        foe.mSpeed.mX = -foe.mSpeed.mX;
                    }
                    break;
            }
        }

        if (foe.mType != kSkeleton && foe.mType != kTinhoso && foe.mType != kCapiroto && foe.mType != kHand ) {
            continue;
        }

        if (foe.mHealth <= 0) {
            actorsToRemove.push_back(foe);

            if (foe.mType == kCapiroto) {
                screen = kVictory;
                prepareScreenFor(screen);
                return;
            }

            if (foe.mType == kTinhoso) {
                hasBossOnScreen = false;

                doors.clear();
            }

            continue;
        }

        if (foe.mType != kSkeleton && foe.mType != kHand ) {
            continue;
        }

        foe.mPosition.mX += foe.mSpeed.mX;
        foe.mPosition.mY += foe.mSpeed.mY;

        if (foe.mPosition.mX >= (320 - 32)) {
            foe.mSpeed.mX = -8;
            foe.mDirection = EDirection::kLeft;
        }

        if (foe.mPosition.mX < 0) {
            foe.mSpeed.mX = 8;
            foe.mDirection = EDirection::kRight;
        }

        if (isBlockedByWall(foe)) {
            foe.mSpeed.mX *= -1;

            if (foe.mDirection == EDirection::kLeft) {
                foe.mDirection = EDirection::kRight;
            } else {
                foe.mDirection = EDirection::kLeft;
            }
        }

        if ((ticksUntilVulnerable <= 0) && collide(foe, player, 16)) {
            hurtPlayer(1);
        }

        foe.mSpeed.mY += 2;
        bool isOnGround = isOnFloor(foe);

        if (isOnGround) {
            foe.mSpeed.mY = 0;
            foe.mPosition.mY = (foe.mPosition.mY / 32) * 32;
        }
    }
    removeFrom(foes, actorsToRemove);
}

void evalutePlayerAttack() {
    for (auto &foe : foes) {
            if ( foe.mType != kTinhoso && foe.mType != kHand &&  foe.mType != kCapiroto && foe.mType != kGargoyle && collide(foe, player)) {
                foe.mHealth -= 2;
                return; //only one enemy per attack!
            }
        }
}

void prepareRoom(int room) {
    muteSound();
    char buffer[64];

    snprintf(buffer, 64, "%s%d.bg", odb::getResPath().c_str(), room );
    FILE *fd;
    fd = fopen(buffer, "r");
    auto bgmap = odb::readToBuffer(fd);
    fclose( fd );

    snprintf(buffer, 64, "%s%d.fg", odb::getResPath().c_str(), room );
    fd = fopen(buffer, "r");
    auto fgmap = odb::readToBuffer(fd);
    fclose( fd );

    foes.clear();
    items.clear();
    doors.clear();
    hasBossOnScreen = false;
    int position = 0;
    for (int y = 0; y < 6; ++y) {
        for (int x = 0; x < 10; ++x) {
            char ch = '0';

            ch = bgmap[ position ];
            backgroundTiles[y][x] = ch - '0';

            ch = fgmap[ position ];
            ++position;

            if (ch == 'm') {
                foregroundTiles[y][x] = 0;
                Item item;
                item.mType = kMeat;
                item.mPosition = Vec2i{x * 32, y * 32};
                items.push_back(item);
            } else if (ch == 'k') {
                if (!hasKey) {
                    foregroundTiles[y][x] = 0;
                    Item item;
                    item.mType = kKey;
                    item.mPosition = Vec2i{x * 32, y * 32};
                    items.push_back(item);
                }
            } else if (ch == 'a') {
                foregroundTiles[y][x] = 0;
                Actor a;
                a.mType = EActorType::kSkeleton;
                a.mPosition = Vec2i{x * 32, y * 32};
                a.mSpeed.mX = 8;
                a.mHealth = 2;
                foes.push_back(a);
            } else if (ch == 'c') {
                foregroundTiles[y][x] = 0;
                currentBossName = "CAPIROTO";
                Actor a;
                a.mType = EActorType::kCapiroto;
                a.mPosition = Vec2i{x * 32, y * 32};
                a.mHealth = 25;
                totalBossHealth = 25;
                hasBossOnScreen = true;
                foes.push_back(a);

                {
                    foregroundTiles[y + 2][ x + 2] = 0;
                    Actor a;
                    a.mType = EActorType::kHand;
                    a.mPosition = Vec2i{(x + 2 ) * 32, (y + 2) * 32};
                    a.mDirection = kLeft;
                    a.mHealth = 100000;
                    foes.push_back(a);
                }
                {
                    foregroundTiles[y + 2][x - 2] = 0;
                    Actor a;
                    a.mType = EActorType::kHand;
                    a.mDirection = kRight;
                    a.mPosition = Vec2i{(x - 2) * 32, (y + 2) * 32};
                    a.mHealth = 100000;
                    foes.push_back(a);
                }


            } else if (ch == 't') {
                foregroundTiles[y][x] = 0;
                currentBossName = "TINHOSO";
                totalBossHealth = 5;
                Actor a;
                a.mType = EActorType::kTinhoso;
                a.mPosition = Vec2i{x * 32, y * 32};
                a.mHealth = 5;
                hasBossOnScreen = true;
                foes.push_back(a);
            } else if (ch == 's') {
                foregroundTiles[y][x] = 0;
                Actor a;
                a.mType = EActorType::kSpawner;
                a.mPosition = Vec2i{x * 32, y * 32};
                a.mHealth = 20;
                foes.push_back(a);
            } else if (ch == 'g') {
                foregroundTiles[y][x] = 0;
                Actor a;
                a.mType = EActorType::kGargoyle;
                a.mPosition = Vec2i{x * 32, y * 32};
                a.mSpeed.mX = 8;
                a.mHealth = 1;
                foes.push_back(a);
            } else if (ch == 'd') {
                foregroundTiles[y][x] = 0;
                Actor a;
                a.mType = hasKey ? EActorType::kOpenDoor : EActorType::kClosedDoor;
                a.mPosition = Vec2i{x * 32, y * 32};
                a.mSpeed.mX = 8;
                doors.push_back(a);
            } else if (ch == 'D') {
                foregroundTiles[y][x] = 0;
                Actor a;
                a.mType = EActorType::kClosedDoor;
                a.mPosition = Vec2i{x * 32, y * 32};
                a.mSpeed.mX = 8;
                doors.push_back(a);
            } else {
                foregroundTiles[y][x] = ch - '0';
            }
        }
        ++position; //\n
    }

    snprintf(buffer, 64, "%s%d.lst", odb::getResPath().c_str(), room );

    std::vector<std::string> tilesToLoad;

    fd = fopen(buffer, "r");
    auto listBuffer = odb::readToBuffer(fd);
    listBuffer.push_back('\n');
    fclose(fd);

    int lastPoint = 0;
    int since = 0;
    auto bufferBegin = std::begin( listBuffer );
    for (const auto& c : listBuffer ) {
        ++since;
        if ( c == '\n' ) {
            auto filename = std::string( bufferBegin + lastPoint, bufferBegin + lastPoint + since - 1 );
            lastPoint += since;
            if ( !filename.empty()) {
                tilesToLoad.push_back(filename);
            }
            since = 0;
        }
    }

    loadTiles(tilesToLoad);

    clearBuffers();
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

bool operator==(const Vec2i &a, const Vec2i &b) {
    return a.mX == b.mX && a.mY == b.mY;
}

bool operator==(const Item &a, const Item &b) {
    return a.mType == b.mType && a.mPosition == b.mPosition;
}

bool operator==(const Actor &a, const Actor &b) {
    return a.mType == b.mType && a.mStance == b.mStance && a.mDirection == b.mDirection && a.mPosition == b.mPosition &&
           a.mSpeed == b.mSpeed;
}