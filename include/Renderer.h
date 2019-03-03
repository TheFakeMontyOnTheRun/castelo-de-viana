//
// Created by monty on 01-07-2017.
//

#ifndef CASTLEVANIA_RENDERER_H
#define CASTLEVANIA_RENDERER_H


struct ControlState {
	bool moveLeft;
	bool moveRight;
	bool moveUp;
	bool moveDown;
	bool fireArrow;
	bool sword;
	bool jump;
	bool secret;
	bool escape;
	bool enter;
};

enum EVideoType {
	kCGA,
	kVGA,
};

ControlState getControlState();

void copyImageBufferToVideoMemory(uint8_t *imageBuffer);

void onQuit();

void beginFrame();

void doneWithFrame();

void soundFrequency(int frequency);

void muteSound();

void onQuit();

void initVideoFor(EVideoType videoType);

void setupOPL2();

void stopSounds();

void soundTick();

uint8_t getPaletteEntry(uint32_t origin);

const char *getAssetsPath();

extern EVideoType videoType;

#endif //CASTLEVANIA_RENDERER_H_H
