#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <cpctelera.h>


#ifdef AMIGA
#include "AmigaInt.h"

#else

#ifdef CPC

#include "CPCInt.h"

#else

#include <stdint.h>
#include <unistd.h>

#endif

#endif


#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "CPCInt.h"

#include "Common.h"
#include "NativeBitmap.h"
#include "Game.h"
#include "Renderer.h"

#include "CPackedFileReader.h"
#include "LoadImage.h"

// Size of each part of the CPCtelera Logo
#define CPCT_W 40
#define CPCT_H 96

// Size of the ByteRealms Logo
#define BR_W   62
#define BR_H   90

// Pointers to the hardware backbuffer, placed in bank 1
// of the memory (0x4000-0x7FFF)
#define SCR_BUFF  (u8*)0x4000


struct ControlState toReturn;
enum EVideoType videoType = kVGA;

struct ControlState getControlState() {


    return toReturn;
}

void beginFrame() {
    clearBuffers();
}

void changeVideoMemoryPage(u8 waitcycles) {
    static u8 cycles = 0;   // Static value to count the number of times this function has been called
    static u8 page = 0;   // Static value to remember the last page shown (0 = page 40, 1 = page C0)

    // Count 1 more cycle and check if we have arrived to waitcycles
    if (++cycles >= waitcycles) {
        cycles = 0;    // We have arrived, restore count to 0

        // Depending on which was the last page shown, we show the other
        // now, and change the page for the next time
        if (page) {
            cpct_setVideoMemoryPage(cpct_pageC0);  // Set video memory at banck 3 (0xC000 - 0xFFFF)
            page = 0;                              // Next page = 0
        } else {
            cpct_setVideoMemoryPage(cpct_page40);  // Set video memory at banck 1 (0x4000 - 0x7FFF)
            page = 1;                              // Next page = 1
        }
    }
}

void doneWithFrame() {
// Draw the ByteRealms logo at its current Y location on the screen. Moving
    // the logo does not leave a trail because we move it pixel to pixel and the
    // sprite has a 0x00 frame around it in its pixel definition.
    pvmem = cpct_getScreenPtr(CPCT_VMEM_START, 10, br_y);  // Locate sprite at (10,br_y) in Default Video Memory
    cpct_drawSprite(G_BR, pvmem, BR_W, BR_H);       // Draw the sprite

    // Change video memory page (from Screen Memory to Back Buffer and vice-versa)
    // every 2.5 secs (125 VSYNCs)
    changeVideoMemoryPage(125);

    // Calculate next location of the ByteRealms logo
    br_y += vy;                            // Add current velocity to Y coordinate
    if (br_y < 1 || br_y + BR_H > 199)   // Check if it exceeds boundaries
        vy = -vy;                           // When we exceed boundaries, we change velocity sense

    // Synchronize next frame drawing with VSYNC
    cpct_waitVSYNC();
}

void muteSound() {
}

void onQuit() {

}


void putpixel(int x, int y, uint32_t pixel) {

}

void copyImageBufferToVideoMemory(uint8_t *imageBuffer) {
    int pos = 0;

    int y = 0;
    for (y = 0; y < 200; ++y) {
        int x = 0;
        for (x = 0; x < 320; ++x) {
            pos = (320 * y) + x;

            putpixel(x, y, imageBuffer[pos]);

        }
    }
}


uint8_t getPaletteEntry(uint32_t origin) {

    uint8_t shade = 0;


    shade += (((((origin & 0x0000FF)) << 2) >> 8)) << 6;
    shade += (((((origin & 0x00FF00) >> 8) << 3) >> 8)) << 3;
    shade += (((((origin & 0xFF0000) >> 16) << 3) >> 8)) << 0;

    return shade;
}


void initVideoFor(enum EVideoType unused) {
    u8 br_y = 55; // Y coordinate of the ByteRealms Logo
    i8 vy = 1;  // Velocity of the ByteRealms Logo in Y axis
    u8 *pvmem;     // Pointer to video memory (or backbuffer) where to draw sprites

    // Initialize CPC Mode and Colours
    cpct_disableFirmware();             // Disable firmware to prevent it from interfering
    cpct_fw2hw(G_palette, 16);   // Convert Firmware colours to Hardware colours
    cpct_setPalette(G_palette, 16);   // Set up palette using hardware colours
    cpct_setBorder(G_palette[0]);    // Set up the border to the background colour (white)
    cpct_setVideoMode(0);               // Change to Mode 0 (160x200, 16 colours)

    // Clean up Screen and BackBuffer filling them up with 0's
    cpct_memset(CPCT_VMEM_START, 0x00, 0x4000);
    cpct_memset(SCR_BUFF, 0x00, 0x4000);

    // Lets Draw CPCtelera's Squared Logo on the BackBuffer. We draw it at
    // byte coordinates (0, 52) with respect to the start of the Backbuffer.
    // We have to draw it into 2 parts because drawSprite function cannot draw
    // sprites wider than 63 bytes (and we have to draw 80). So we draw the
    // logo in two 40-bytes wide parts.
    pvmem = cpct_getScreenPtr(SCR_BUFF, 0, 52);
    cpct_drawSprite(G_CPCt_left, pvmem, CPCT_W, CPCT_H);
    cpct_drawSprite(G_CPCt_right, pvmem + CPCT_W, CPCT_W, CPCT_H);
}

const char *getAssetsPath() {
    return "sdl.pfs";
}
