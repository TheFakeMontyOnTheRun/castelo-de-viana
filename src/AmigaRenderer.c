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
struct Window *my_window;

struct NewWindow my_new_window= {
  50,            /* LeftEdge    pencerenin X pozisyonu */
  30,            /* TopEdge     pencerenin Y pozisyonu */
  300,           /* Width       pencerenin genisligi */
  200,            /* Height      pencerenin yuksekligi */
  0,             /* DetailPen   colour reg. 0 ile text cizilir */
  1,             /* BlockPen    colour reg. 1 ile block cizilir */
  ACTIVEWINDOW|
  VANILLAKEY|
  CLOSEWINDOW|
  RAWKEY,        /* IDCMPFlags  */
  SMART_REFRESH| /* Flags       */
  WINDOWDRAG|    /*             */
  WINDOWDEPTH|   /*             */
  ACTIVATE,      /*             acildiginda pencereyi aktif hale getir*/
  NULL,          /* FirstGadget */
  NULL,          /* CheckMark   */
  (UBYTE*)"Squares",	 /* Title       pencere basligi */
  NULL,          /* Screen      */
  NULL,          /* BitMap      */
  320,             /* MinWidth    */
  200,             /* MinHeight   */
  320,             /* MaxWidth    */
  200,             /* MaxHeight   */
  WBENCHSCREEN   /* Type        Workbench Screen. */
};

SHORT my_points[]= {
  0, 0,
  1, 1
};

SHORT quadPoints[]= {
  0,0, 50,0, 50,30, 0,30, 0,0,  
};

struct Border my_border=   {
  0, 0,        /* LeftEdge, TopEdge. */
  3,           /* FrontPen, colour register 3. */
  0,           /* BackPen, for the moment unused. */
  JAM1,        /* DrawMode, draw the lines with colour 3. */
  2,           /* mypoint 6 noktanın koordinatlari cizilecek */
  my_points,   /* koordinatlara ait dizinin pointer'i */
  NULL,        /* NextBorder */
};


struct Border quad =   {
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

  //  Wait( 1 << my_window->UserPort->mp_SigBit );

  if ( my_message = (struct IntuiMessage *) GetMsg(my_window->UserPort)) {
    messageClass = my_message->Class;
    code = my_message->Code;
    qualifier = my_message->Qualifier;
    x = my_message->MouseX;
    y = my_message->MouseY;
    seconds = my_message->Seconds;
    micros = my_message->Micros;
    ReplyMsg((struct Message*)my_message);

  }


  return toReturn;
}

void beginFrame() {
  clearBuffers();
}

void doneWithFrame() {
  Delay(1);
}

void muteSound() {
}

void onQuit() {
  CloseWindow( my_window );
  CloseLibrary( (struct Library*)IntuitionBase );
}

void putpixel(int x0, int y0, uint32_t pixel){
//  WritePixel(my_window->RPort, x, y);
  quadPoints[0] = 2 * x0;
  quadPoints[1] = 2 * y0;
  quadPoints[2] = 2 * x0 + 1;
  quadPoints[3] = 2 * y0;
  quadPoints[4] = 2 * x0 + 1;
  quadPoints[5] = 2 * y0 + 1;
  quadPoints[6] = 2 * x0;
  quadPoints[7] = 2 * y0 + 1;
  quadPoints[8] = 2 * x0;
  quadPoints[9] = 2 * y0;

  quad.FrontPen = index;

  DrawBorder(my_window->RPort, &quad, 0, 0);
}

void copyImageBufferToVideoMemory(uint8_t* imageBuffer ) {
  int pos = 0;

  int y = 0;
  for ( y = 0; y < 200; ++y ) {
    int x = 0;
    for ( x = 0; x < 320; ++x ) {
      pos = ( 320 * y ) + x;

      putpixel(x, y, imageBuffer[ pos ]);

    }
  }
}


uint8_t getPaletteEntry( uint32_t origin ) {

  uint8_t shade = 0;


  shade += (((((origin & 0x0000FF)      ) << 2) >> 8)) << 6;
  shade += (((((origin & 0x00FF00)  >> 8) << 3) >> 8)) << 3;
  shade += (((((origin & 0xFF0000) >> 16) << 3) >> 8)) << 0;

  return shade;
}


void initVideoFor(enum EVideoType unused) {
  IntuitionBase = (struct IntuitionBase *) OpenLibrary( "intuition.library", 0 );

  if( IntuitionBase == NULL ) {
    puts("nope 1!");
    exit(0);
  }

  my_window = (struct Window *) OpenWindow( &my_new_window );

  if(my_window == NULL) {
    puts("nope 2!");
    CloseLibrary( (struct Library*) IntuitionBase );
    exit(0);
  }
}

const char* getAssetsPath() {
  return "sdl.pfs";
}
