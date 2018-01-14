//
// Created by monty on 01-07-2017.
//

#ifndef CASTLEVANIA_RENDERER_H
#define CASTLEVANIA_RENDERER_H


class ControlState {
public:
    bool moveLeft = false;
    bool moveRight = false;
    bool moveUp = false;
    bool moveDown = false;
    bool fireArrow = false;
    bool sword = false;
    bool jump = false;
    bool secret = false;
    bool escape = false;
    bool enter = false;
};

enum EVideoType : uint8_t {
    kCGA,
    kVGA,
};

ControlState getControlState();
void copyImageBufferToVideoMemory(const std::array<uint8_t , 320 * 200>& imageBuffer );
void onQuit();
void beginFrame();
void doneWithFrame();
void soundFrequency(int frequency);
void muteSound();
void onQuit();
void initVideoFor(EVideoType videoType);
void setupOPL2(int instrument);
void stopSounds();
void soundTick();
uint8_t getPaletteEntry( uint32_t origin );
std::string getAssetsPath();

extern EVideoType videoType;

#endif //CASTLEVANIA_RENDERER_H_H
