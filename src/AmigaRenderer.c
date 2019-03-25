#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <intuition/intuition.h>
#include <exec/types.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>


#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "AmigaInt.h"

#include "Common.h"
#include "NativeBitmap.h"
#include "Game.h"
#include "Renderer.h"

#include "CPackedFileReader.h"
#include "LoadImage.h"

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
extern struct ExecBase *SysBase;

struct Window *my_window;
struct Screen  *myscreen;
struct NewScreen xnewscreen 	=			{
		0,               /* LeftEdge  Daima 0 olmali */
		0,               /* TopEdge   */
		640,             /* Width     */
		200,             /* Height    */
		8,               /* Depth     16 colours. */
		0,               /* DetailPen */
		1,               /* BlockPen */
		0, /* ViewModes High-resolution, Interlaced */
		CUSTOMSCREEN,    /* Type customized screen. */
		NULL,            /* Font */
		"Castle of Viana",     /* Title */
		NULL,            /* Gadget */
		NULL             /* BitMap */
};
struct NewWindow my_new_window = {
		0,            /* LeftEdge    pencerenin X pozisyonu */
		0,            /* TopEdge     pencerenin Y pozisyonu */
		320,           /* Width       pencerenin genisligi */
		200,            /* Height      pencerenin yuksekligi */
		0,             /* DetailPen   colour reg. 0 ile text cizilir */
		1,             /* BlockPen    colour reg. 1 ile block cizilir */
		ACTIVEWINDOW |
		VANILLAKEY |
		CLOSEWINDOW |
		RAWKEY,        /* IDCMPFlags  */
		SMART_REFRESH | /* Flags       */
		WINDOWDRAG |    /*             */
		WINDOWDEPTH |   /*             */
		ACTIVATE,      /*             acildiginda pencereyi aktif hale getir*/
		NULL,          /* FirstGadget */
		NULL,          /* CheckMark   */
		(UBYTE *) "Squares",     /* Title       pencere basligi */
		NULL,          /* Screen      */
		NULL,          /* BitMap      */
		320,             /* MinWidth    */
		200,             /* MinHeight   */
		320,             /* MaxWidth    */
		200,             /* MaxHeight   */
		CUSTOMSCREEN   /* Type        Workbench Screen. */
};

SHORT my_points[] = {
		0, 0,
		1, 1
};

SHORT quadPoints[] = {
		0, 0, 50, 0, 50, 30, 0, 30, 0, 0,
};

struct Border my_border = {
		0, 0,        /* LeftEdge, TopEdge. */
		3,           /* FrontPen, colour register 3. */
		0,           /* BackPen, for the moment unused. */
		JAM1,        /* DrawMode, draw the lines with colour 3. */
		2,           /* mypoint 6 noktanın koordinatlari cizilecek */
		my_points,   /* koordinatlara ait dizinin pointer'i */
		NULL,        /* NextBorder */
};


struct Border quad = {
		0, 0,        /* LeftEdge, TopEdge. */
		3,           /* FrontPen, colour register 3. */
		0,           /* BackPen, for the moment unused. */
		JAM1,        /* DrawMode, draw the lines with colour 3. */
		5,           /* mypoint 6 noktanın koordinatlari cizilecek */
		quadPoints,   /* koordinatlara ait dizinin pointer'i */
		NULL,        /* NextBorder */
};


struct ControlState toReturn;
enum EVideoType videoType = kVGA;

struct ControlState getControlState() {

	struct IntuiMessage *my_message;
	BOOL close_me;
	ULONG messageClass;
	USHORT code;
	SHORT x, y;
	BOOL mouse_moved;
	USHORT qualifier;
	ULONG seconds;
	ULONG micros;
	memset(&toReturn, 0, sizeof(toReturn));

	if (my_message = (struct IntuiMessage *) GetMsg(my_window->UserPort)) {
		messageClass = my_message->Class;
		code = my_message->Code;
		qualifier = my_message->Qualifier;
		x = my_message->MouseX;
		y = my_message->MouseY;
		seconds = my_message->Seconds;
		micros = my_message->Micros;
		ReplyMsg((struct Message *) my_message);



		if (messageClass == VANILLAKEY) {
			switch(code) {

				case 'q':
					toReturn.escape = TRUE;
					break;

				case 'i':
					toReturn.enter = TRUE;
					break;

				case 'a':
					toReturn.moveLeft = TRUE;
					break;

				case 'd':
					toReturn.moveRight = TRUE;
					break;

				case 'w':
					toReturn.moveUp = TRUE;
					break;

				case 'l':
					toReturn.secret = TRUE;
					break;

				case 's':
					toReturn.moveDown = TRUE;
					break;


				case 'e':
					toReturn.jump = TRUE;
					break;

				case 'o':
					toReturn.sword = TRUE;
					break;

				case 'p':
					toReturn.fireArrow = TRUE;
					break;

			}
		}

	}


	return toReturn;
}

void beginFrame() {
	clearBuffers();
}

void doneWithFrame() {

}

void muteSound() {
}

void onQuit() {
	CloseWindow(my_window);
	CloseScreen(myscreen);
	CloseLibrary((struct Library *) IntuitionBase);
}

void putpixel(int x, int y, uint32_t pixel) {
}

void copyImageBufferToVideoMemory(uint8_t *imageBuffer) {
	WriteChunkyPixels(my_window->RPort, 0, 0, 319, 199, imageBuffer, 320);
}


uint8_t getPaletteEntry(uint32_t origin) {

	uint8_t shade = 0;


	shade += (((((origin & 0x0000FF)) << 2) >> 8)) << 6;
	shade += (((((origin & 0x00FF00) >> 8) << 3) >> 8)) << 3;
	shade += (((((origin & 0xFF0000) >> 16) << 3) >> 8)) << 0;

	return shade;
}

struct RGB8 {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

void initVideoFor(enum EVideoType unused) {
	int r, g, b;
	int c;
	struct RGB8 palete[256];
	struct ColorMap* cm;
	struct Window  *window;
	struct IntuiMessage *msg;
	struct DisplayInfo displayinfo;
	struct TagItem  taglist[3];
	int OpenA2024 = FALSE;
	int IsV36 = FALSE;
	int IsPAL;

	IntuitionBase = (struct IntuitionBase *) OpenLibrary("intuition.library", 0);

	if (IntuitionBase == NULL) {
		puts("nope 1!");
		exit(0);
	}






	if ((myscreen = OpenScreen(&xnewscreen)) == NULL) {
	}


	my_new_window.Screen = myscreen;

	my_window = (struct Window *) OpenWindow(&my_new_window);

	if (my_window == NULL) {
		puts("nope 2!");
		CloseLibrary((struct Library *) IntuitionBase);
		exit(0);
	}

	for (r = 0; r < 256; r += 16) {
		for (g = 0; g < 256; g += 8) {
			for (b = 0; b < 256; b += 8) {
				uint32_t pixel = 0xFF000000 + (r << 16) + (g << 8) + (b);
				uint8_t paletteEntry = getPaletteEntry(pixel);
				palete[paletteEntry].r = ((16 * b) / 256);
				palete[paletteEntry].g = ((16 * g) / 256);
				palete[paletteEntry].b = ((16 * r) / 256);
			}
		}
	}


	for (c = 0; c < 256; ++c ) {
		SetRGB4( &myscreen->ViewPort, c, palete[c].r, palete[c].g, palete[c].b );
	}
}

const char *getAssetsPath() {
	return "sdl.pfs";
}
