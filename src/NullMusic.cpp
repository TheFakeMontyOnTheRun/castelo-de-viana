#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string>

#include "Common.h"
#include "NativeBitmap.h"
#include "CPackedFileReader.h"
#include "Renderer.h"
#include "LoadImage.h"
#include "Game.h"

int dummy;
std::string dummy2;

void playMusic(int instrument, const char *music) {
    dummy = instrument;
    dummy2 = (char *) music;
}

void setupOPL2() {
}

void stopSounds() {
}

void playTune(const std::string& music) {
    dummy2 = music;
}

void soundTick() {

}
