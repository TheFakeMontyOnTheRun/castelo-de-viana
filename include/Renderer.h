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

#ifdef __cplusplus
extern "C"
#endif
struct ControlState getControlState();

#ifdef __cplusplus
extern "C"
#endif
void copyImageBufferToVideoMemory(uint8_t *imageBuffer);

#ifdef __cplusplus
extern "C"
#endif
void onQuit();

#ifdef __cplusplus
extern "C"
#endif
void beginFrame();

#ifdef __cplusplus
extern "C"
#endif
void doneWithFrame();

void soundFrequency(int frequency);

#ifdef __cplusplus
extern "C"
#endif
void muteSound();

#ifdef __cplusplus
extern "C"
#endif
void onQuit();

#ifdef __cplusplus
extern "C"
#endif
void initVideoFor(enum EVideoType videoType);

#ifdef __cplusplus
extern "C"
#endif
void setupOPL2();

void stopSounds();

#ifdef __cplusplus
extern "C"
#endif
void soundTick();

#ifdef __cplusplus
extern "C"
#endif
uint8_t getPaletteEntry(uint32_t origin);

#ifdef __cplusplus
extern "C"
#endif
const char *getAssetsPath();

extern enum EVideoType videoType;

#endif
