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

ControlState getControlState();
void copyImageBufferToVideoMemory(const std::array<unsigned int, 320 * 200>& imageBuffer );
void onQuit();
void beginFrame();
void doneWithFrame();
void soundFrequency(int frequency);
void muteSound();
void onQuit();
void initVideo();
std::string getResPath();
void setupOPL2();
void stopSounds();
void soundTick();
#endif //CASTLEVANIA_RENDERER_H_H
