#include <jni.h>
#include <string.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>


#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <stdint.h>
#include <unistd.h>

#include "Common.h"
#include "NativeBitmap.h"
#include "Game.h"
#include "Renderer.h"
#include "CPackedFileReader.h"
#include "LoadImage.h"
#include "CPackedFileReader.h"

void sysTick();
void loadGraphics();

enum EVideoType videoType = kVGA;

uint8_t framebufferFinal[320 * 240 * 4];
uint32_t palette[256];
char mBufferedCommand;
AAssetManager *defaultAssetManager = NULL;

void onQuit(void) {
}


uint8_t getPaletteEntry( uint32_t origin ) {

    uint8_t shade = 0;


    shade += (((((origin & 0x0000FF)      ) << 2) >> 8)) << 6;
    shade += (((((origin & 0x00FF00)  >> 8) << 3) >> 8)) << 3;
    shade += (((((origin & 0xFF0000) >> 16) << 3) >> 8)) << 0;

    return shade;
}
void initVideoFor(enum EVideoType unused) {
    videoType = kVGA;

    int r, g, b;

    for (r = 0; r < 256; r += 16) {
        for (g = 0; g < 256; g += 8) {
            for (b = 0; b < 256; b += 8) {
                uint32_t pixel = 0xFF000000 + (r << 16) + (g << 8) + (b);
                uint8_t paletteEntry = getPaletteEntry(pixel);
                palette[paletteEntry] = pixel;
            }
        }
    }
}

void copyImageBufferToVideoMemory(uint8_t* imageBuffer ) {
    uint8_t *ptr = &framebufferFinal[0];

    memset(ptr, 0, 320 * 240 * 4);

    for (int y = 0; y < 240; ++y) {
        for (int x = 0; x < 320; ++x) {

            int32_t pixel = palette[imageBuffer[(int)(320 * ((200 * y) / 240 )) + x]];

            uint8_t r = (pixel & 0x000000FF) - 0x38;
            uint8_t g = ((pixel & 0x0000FF00) >> 8) - 0x18;
            uint8_t b = ((pixel & 0x00FF0000) >> 16) - 0x10;

            *ptr = r;
            ++ptr;
            *ptr = g;
            ++ptr;
            *ptr = b;
            ++ptr;
            *ptr = 255;
            ++ptr;

        }
    }
}

void clearRenderer(void) {}

JNIEXPORT void JNICALL
Java_pt_b13h_castleofviana_MainActivity_initAssets(JNIEnv *env, jobject thiz,
                                                      jobject assetManager) {

    AAssetManager *asset_manager = AAssetManager_fromJava(env, assetManager);
    defaultAssetManager = asset_manager;
    srand(time(NULL));


    initVideoFor(kVGA);
    loadGraphics();
    init();
    prepareScreenFor(kIntro);
}

JNIEXPORT void JNICALL
Java_pt_b13h_castleofviana_MainActivity_getPixelsFromNative(JNIEnv *env, jobject thiz,
                                                               jbyteArray array) {
    sysTick();
    jbyte *narr = (*env)->GetByteArrayElements(env, array, NULL);
    memcpy(narr, &framebufferFinal[0], 320 * 240 * 4);
}

int soundToPlay = -1;

void setupOPL2(int port) {}

void stopSounds() {}

void playSound(const int action) {
    soundToPlay = action;
}

void soundTick() {}

void muteSound() {}

JNIEXPORT jint JNICALL
Java_pt_b13h_castleofviana_MainActivity_getSoundToPlay(JNIEnv *env, jobject thiz) {
    int toReturn = soundToPlay;
    soundToPlay = -1;
    return toReturn;
}


JNIEXPORT void JNICALL
Java_pt_b13h_castleofviana_MainActivity_sendCommand(JNIEnv *env, jobject thiz, jint cmd) {
    mBufferedCommand = cmd;
}

struct ControlState getControlState() {
    struct ControlState toReturn;
    memset(&toReturn, 0, sizeof(struct ControlState));



    toReturn.moveUp = mBufferedCommand &    (1 << 0);
    toReturn.moveRight = mBufferedCommand & (1 << 1);
    toReturn.moveDown = mBufferedCommand &  (1 << 2);
    toReturn.moveLeft = mBufferedCommand &  (1 << 3);
    toReturn.sword = mBufferedCommand &     (1 << 4);
    toReturn.jump = mBufferedCommand &      (1 << 5);
    toReturn.fireArrow = mBufferedCommand & (1 << 6);
    toReturn.enter = mBufferedCommand & (    1 << 7);


    mBufferedCommand = 0;

    return toReturn;
}



void playMusic(int instrument, const char* music) {
}

void playTune(const char* music) {

}

void beginFrame() {

}

void doneWithFrame() {

}

const char* getAssetsPath() {
    return "sdl.pfs";
}

