#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>
#include <sys/farptr.h>
#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <bios.h>
#include <time.h>
#include <unistd.h>

#include "Common.h"
#include "OPL2.h"
#include "controller.h"

#define STR(x) #x
#define XSTR(x) STR(x)

bool enableOPL2 = false;
extern OPL2 opl2;

odb::StaticBuffer noSound;
odb::StaticBuffer *melody;
uint8_t* currentSoundPosition;

int currentNote = 0;

short get_lpt_port(int i) {
	return _farpeekw(_dos_ds, 0x0408 + (2 * (i - 1)));
}

short setup(void) {
	cputs("OPT2LPT setup\r\n\r\n");

	char num_ports, port, i;

	num_ports = 0;
	for (i = 1; i < 4; i++) {
		if (get_lpt_port(i)) {
			num_ports++;
			port = i;
		}
	}

	if (num_ports == 0) {
		cputs("Sorry, no printer port found...\r\n");
		exit(1);
	} else if (num_ports == 1) {
		cprintf("Found one printer port: LPT%d\r\n", port);
		return get_lpt_port(port);
	} else {
		cputs("Found multiple printer ports:");
		for (i = 1; i < 4; i++) {
			if (get_lpt_port(i)) {
				cprintf(" LPT%d", i);
			}
		}
		cputs("\r\nWhich one is the OPT2LPT connected to? [");
		for (i = 1; i < 4; i++) {
			if (get_lpt_port(i)) {
				cprintf("%d", i);
			}
		}
		cputs("]? ");
		do {
			port = getch() - '0';
		} while (port < 1 || port > 3 || !get_lpt_port(port));
		cprintf("LPT%d\r\n", port);
		return get_lpt_port(port);
	}
	return 0;
}


void playSound(odb::StaticBuffer* sound) {

	if ( sound == NULL || sound->size == 0 ) {
		melody = &noSound;
		return;
	}

	melody = sound;
	currentSoundPosition = melody->data;
}


void playMusic(const char *music) {
	if (enableOPL2) {
		if (music != NULL && strlen(music) > 0 ) {
			music_set("", "", "");
			return;
		}
		music_set(music, music, music);
		return;
	}
}

void muteSound() {
	if (!enableOPL2) {
		nosound();
		melody = NULL;
		currentNote = 0;
	} else {
		playMusic("");
	}
}

void playTune(const char *music) {
	if (enableOPL2) {
		hackTune(music);
	}
}

void setupOPL2() {
	short lpt_base = setup();
	opl2.init(lpt_base);
	music_setup();
	enableOPL2 = true;
}

void soundFrequency(int frequency) {
	if (!enableOPL2) {
		if (frequency != 0 && currentNote != frequency) {
			sound(frequency);
			currentNote = frequency;
		}
	}
}


void soundTick() {
	if (enableOPL2) {
		music_loop();
	} else {
		if (melody == NULL || melody->size == 0 ) {
			return;
		}

		if (currentSoundPosition != (melody->data + melody->size)) {
			soundFrequency(*currentSoundPosition);
			currentSoundPosition = currentSoundPosition++;
		} else {
			muteSound();
			melody = &noSound;
		}
	}
}

void stopSounds() {
	if (enableOPL2) {
		music_shutdown();
	} else {
		muteSound();
	}
}
