#include <go32.h>
#include <sys/farptr.h>
#include <conio.h>
#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <bios.h>
#include <algorithm>
#include <array>
#include <random>
#include <iostream>
#include <time.h>
#include <unistd.h>
#include <memory>
#include <fstream>
#include <sstream>

#include "Game.h"

int heroFrame = 0;
Actor player;
int counter = 0;
int room = 0;
bool hasKey = false;
int ticksUntilVulnerable = 0;

std::array<std::array<int, 10>, 6> backgroundTiles;
std::array<std::array<int, 10>, 6> foregroundTiles;
std::vector<Actor> foes;
std::vector<Actor> doors;
std::vector<Item> items;
std::vector<Actor> arrows;

void init() {
    player.mPosition.mX = 0;
    player.mPosition.mY = 0;
    player.mSpeed.mX = 0;
    player.mSpeed.mY = 0;
    player.mHealth = 10;
    player.mType = EActorType::kPlayer;
    counter = 0;
    room = 0;
}

void updateHero(bool isOnGround, bool isJumping, bool isUpPressed, bool isDownPressed, bool isLeftPressed, bool isAttacking, bool isUsingSpecial,
                bool isRightPressed, bool isOnStairs) {

    if (isJumping) {
        if (isOnGround) {
            player.mSpeed.mY = -12;
        }
        player.mStance = kJumping;
    }

    if (isUpPressed) {
        if (isOnStairs) {
            player.mSpeed.mY = -8;
            player.mStance = kClimbing;
        } else if (isOnGround) {
            player.mStance = kUp;
        }
    }

    if (isDownPressed) {
        if (isOnStairs) {
            player.mSpeed.mY = +8;
            player.mStance = kClimbing;
        } else {
            player.mStance = kStanding;
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

    if ( isAttacking ) {
        player.mStance = kAttacking;
    }

    if ( isUsingSpecial  ){
        Actor a;
        a.mType = kArrow;
        a.mPosition = player.mPosition;
        a.mSpeed = { player.mDirection == kRight ? 16 : -16, 0 };
        a.mDirection = player.mDirection;
        arrows.push_back(a);
        player.mStance = kAltAttacking;
    }
}

bool isOnFloor( const Actor& actor ) {
    int ground = ((actor.mPosition.mY + 32) / 32);

    if (ground > 5) {
        return true;
    } else {
        if (foregroundTiles[ground][(actor.mPosition.mX + 16) / 32] == 1 || foregroundTiles[ground][(actor.mPosition.mX + 16) / 32] == 3) {
            return true;
        }
    }

    return false;
}

bool isBlockedByWall( const Actor& actor ) {
    int front = ((actor.mPosition.mX ) / 32);

    if ( actor.mDirection == EDirection::kRight ) {
        front++;
    }

    return ( foregroundTiles[ ( actor.mPosition.mY + 16  ) / 32 ][ front ] == 1 );
}

bool collide( const Actor& a, const Actor& b, int tolerance = 32 ) {
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

void gameTick(bool &isOnGround, bool &isOnStairs) {

    if ( ticksUntilVulnerable > 0 ) {
        --ticksUntilVulnerable;
    }

    isOnStairs= (foregroundTiles[(player.mPosition.mY + 16) / 32][(player.mPosition.mX + 16) / 32] == 3);

    if (player.mDirection == EDirection::kRight) {
        if (foregroundTiles[((player.mPosition.mY + 16) / 32)][ ( player.mPosition.mX + 32 ) / 32 ] == 1) {
            player.mSpeed.mX = 0;
        }
    }

    if (player.mDirection == EDirection::kLeft) {
        if (foregroundTiles[((player.mPosition.mY + 16) / 32)][ ( player.mPosition.mX ) / 32 ] == 1) {
            player.mSpeed.mX = 0;
        }
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

    if ( player.mSpeed.mX == 0 ) {
        heroFrame = 0;
    }

    int ceiling = ( player.mPosition.mY) / 32;

    isOnGround = isOnFloor(player);

    if (isOnGround ) {
        player.mStance = kStanding;
    }

    if (ceiling < 0) {
        ceiling = 0;
    }

    if ((player.mSpeed.mX != 0 && isOnGround) || (player.mSpeed.mY != 0 && isOnStairs)) {
        heroFrame = (heroFrame + 1) % 2;
    }

    player.mSpeed.mX = player.mSpeed.mX / 2;

    if (isOnGround && !isOnStairs ) {
        player.mSpeed.mY = std::min( 0, player.mSpeed.mY );
        player.mPosition.mY = std::min( player.mPosition.mY, (player.mPosition.mY / 32) * 32 );
    }

    if ( player.mSpeed.mY < 0 && foregroundTiles[ceiling][(player.mPosition.mX + 16) / 32] == 1) {
        player.mSpeed.mY = -player.mSpeed.mY;
    }

    if ( player.mSpeed.mY < 0 && foregroundTiles[player.mPosition.mY / 32][(player.mPosition.mX + 16) / 32] == 1) {
        player.mSpeed.mY = -player.mSpeed.mY;
    }

    if (!isOnStairs) {
        player.mSpeed.mY = player.mSpeed.mY + 2;
        //this prevents from jumping while keeping the climbing animation state. Unfortunately, prevents looking up.
        //playerStance = EStance::kStanding;
    } else {
        player.mSpeed.mY = 0;
    }

    int px = ( (player.mPosition.mX / 32 ) * 32 );
    int py = ( (player.mPosition.mY / 32 ) * 32 );

    std::vector<Actor> actorsToRemove;

    for ( auto& arrow : arrows ) {
        arrow.mPosition.mX += arrow.mSpeed.mX;
        arrow.mPosition.mY += arrow.mSpeed.mY;

        if ( isBlockedByWall( arrow ) ) {
            actorsToRemove.push_back( arrow );
            continue;
        }

        for ( auto& foe : foes ) {
            if ( collide( foe, arrow, 16 ) ) {
                foe.mSpeed.mX = 0;
                actorsToRemove.push_back( arrow );
            }
        }
    }

    arrows.erase( std::remove_if( std::begin(arrows),std::end(arrows),
                                 [&](Actor x){
                                     return std::find(std::begin(actorsToRemove),std::end(actorsToRemove),x)!=std::end(actorsToRemove);
                                 }
    ), std::end(arrows) );

    std::vector<Item> itemsToRemove;

    for (const auto& item : items ) {
        int ix = (item.mPosition.mX / 32) * 32;
        int iy = (item.mPosition.mY / 32) * 32;

        if ( ix == px && iy == py) {

            if ( item.mType == kKey && !hasKey ) {
                hasKey = true;
                itemsToRemove.push_back( item );

                for (auto& door : doors ) {
                    door.mType = EActorType::kOpenDoor;
                }
            } else if ( item.mType == kMeat ) {
                itemsToRemove.push_back( item );
            }
        }
    }

    items.erase( std::remove_if( std::begin(items),std::end(items),
                        [&](Item x){
                            return std::find(std::begin(itemsToRemove),std::end(itemsToRemove),x)!=std::end(itemsToRemove);
                        }
    ), std::end(items) );

    for ( auto& item : itemsToRemove ) {
        std::remove( std::begin(items), std::end( items ), item );
    }

    actorsToRemove.clear();

    if ( player.mStance == EStance::kAttacking ) {

        for ( auto& foe : foes ) {
            if ( std::abs(foe.mPosition.mY - player.mPosition.mY) < 64 ) {

                if (player.mDirection == EDirection::kRight) {
                    int diff = foe.mPosition.mX - player.mPosition.mX;
                    if (diff < (64) && diff > 0) {
                        foe.mHealth--;
                    }
                } else {
                    int diff = player.mPosition.mX - foe.mPosition.mX;
                    if (diff < (64) && diff > 0) {
                        foe.mHealth--;
                    }
                }
            }

            if ( foe.mHealth <= 0 ) {
                actorsToRemove.push_back( foe );
            }

            foes.erase( std::remove_if( std::begin(foes),std::end(foes),
                                          [&](Actor x){
                                              return std::find(std::begin(actorsToRemove),std::end(actorsToRemove),x)!=std::end(actorsToRemove);
                                          }
            ), std::end(foes) );
        }


        player.mStance = EStance::kStanding;
    }


    ++counter;


    for ( auto& foe : foes ) {

        foe.mPosition.mX += foe.mSpeed.mX;
        foe.mPosition.mY += foe.mSpeed.mY;

        if ( foe.mPosition.mX >= ( 320 - 32 ) ) {
            foe.mSpeed.mX = -8;
            foe.mDirection = EDirection::kLeft;
        }

        if ( foe.mPosition.mX < 0 ) {
            foe.mSpeed.mX = 8;
            foe.mDirection = EDirection::kRight;
        }

        if ( isBlockedByWall( foe )  ) {
            foe.mSpeed.mX *= -1;

            if ( foe.mDirection == EDirection::kLeft ) {
                foe.mDirection = EDirection::kRight;
            } else {
                foe.mDirection = EDirection::kLeft;
            }
        }

        if ( (ticksUntilVulnerable <= 0 ) && collide( foe, player, 16 ) ) {
            player.mHealth--;
            ticksUntilVulnerable = 14;
        }

        foe.mSpeed.mY += 2;
        bool isOnGround = isOnFloor(foe);

        if ( isOnGround ) {
            foe.mSpeed.mY = 0;
            foe.mPosition.mY = (foe.mPosition.mY / 32) * 32;
        }
    }
}

void prepareRoom(int room) {

    if (room < 0) {
        std::cout << "room " << room << " is invalid " << std::endl;
        exit(0);
    }

    std::stringstream roomName;

    roomName = std::stringstream("");
    roomName << room;
    roomName << ".bg";
    std::ifstream bgmap(roomName.str());

    roomName = std::stringstream("");
    roomName << room;
    roomName << ".fg";

    std::ifstream fgmap(roomName.str());
    foes.clear();
    items.clear();
    doors.clear();
    for (int y = 0; y < 6; ++y) {
        for (int x = 0; x < 10; ++x) {
            char ch = '0';

            bgmap >> ch;
            backgroundTiles[y][x] = ch - '0';

            fgmap >> ch;


            if ( ch == 'm' ) {
                foregroundTiles[y][x] = 0;
                Item item;
                item.mType = kMeat;
                item.mPosition = Vec2i{x * 32, y * 32};
                items.push_back(item);
            } else if ( ch == 'k' ) {
                if ( !hasKey ) {
                    foregroundTiles[y][x] = 0;
                    Item item;
                    item.mType = kKey;
                    item.mPosition = Vec2i{x * 32, y * 32};
                    items.push_back(item);
                }
            } else if ( ch == 'a' ) {
                foregroundTiles[y][x] = 0;
                Actor a;
                a.mType = EActorType::kSkeleton;
                a.mPosition = Vec2i{ x * 32, y * 32 };
                a.mSpeed.mX = 8;
                a.mHealth = 2;
                foes.push_back(a);
            } else if ( ch == 'd' ) {
                foregroundTiles[y][x] = 0;
                Actor a;
                a.mType = hasKey ? EActorType::kOpenDoor : EActorType::kClosedDoor;
                a.mPosition = Vec2i{ x * 32, y * 32 };
                a.mSpeed.mX = 8;
                doors.push_back(a);
            } else {
                foregroundTiles[y][x] = ch - '0';
            }



        }
    }

    roomName = std::stringstream("");
    roomName << room;
    roomName << ".lst";

    std::ifstream tileList(roomName.str());
    std::string buffer;

    std::vector<std::string> tilesToLoad;

    while (tileList.good()) {
        std::getline(tileList, buffer);
        tilesToLoad.push_back(buffer);
    }

    loadTiles( tilesToLoad );

    clearBuffers();
}

void enforceScreenLimits() {
    if (player.mPosition.mX < 0) {
        if ((room % 10) > 0) {
            player.mPosition.mX = 320 - 32 - 1;
            prepareRoom(--room);
        } else {
            player.mPosition.mX = 0;
        }
    }

    if (player.mPosition.mY < 0) {
        if ((room / 10) <= 9) {
            player.mPosition.mY = 200 - 32 - 1;
            room += 10;
            prepareRoom(room);
        } else {
            player.mPosition.mY = 0;
        }
    }

    if ((player.mPosition.mX + 32) >= 320) {
        if ((room % 10) < 9) {
            player.mPosition.mX = 1;
            prepareRoom(++room);
        } else {
            player.mPosition.mX = 320 - 32;
        }
    }

    if ((player.mPosition.mY + 32) >= 200) {
        if ((room / 10) >= 1) {
            player.mPosition.mY = 1;
            room -= 10;
            prepareRoom(room);
        } else {
            player.mPosition.mY = 200 - 32 - 1;
        }
    }
}

bool operator==( const Vec2i& a, const Vec2i& b ) {
    return a.mX == b.mX && a.mY == b.mY;
}

bool operator==( const Item& a, const Item& b ) {
    return a.mType == b.mType && a.mPosition == b.mPosition;
}

bool operator==( const Actor& a, const Actor& b ) {
    return a.mType == b.mType && a.mStance == b.mStance && a.mDirection == b.mDirection && a.mPosition == b.mPosition && a.mSpeed == b.mSpeed;
}