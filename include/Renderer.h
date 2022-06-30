#ifndef RENDERER_H
#define RENDERER_H

struct ControlState {
	int moveLeft;
	int moveRight;
	int moveUp;
	int moveDown;
	int fireArrow;
	int sword;
	int jump;
	int secret;
	int escape;
	int enter;
};

enum EVideoType {
	kCGA,
	kVGA
};

struct ControlState getControlState();

void copyImageBufferToVideoMemory(const uint8_t *imageBuffer);

void onQuit();

void beginFrame();

void doneWithFrame();

void soundFrequency(int frequency);

void muteSound();

void onQuit();

void initVideoFor(enum EVideoType videoType);

void setupOPL2();

void stopSounds();

void soundTick();

uint8_t getPaletteEntry(uint32_t origin);

const char *getAssetsPath();

extern enum EVideoType videoType;

#endif
