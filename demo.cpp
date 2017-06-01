#include <conio.h>
#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <bios.h>
#include <algorithm>
#include <array>
#include <random>
#include <iostream>
#include <time.h>
#include <unistd.h>
#include <memory>
#include "NativeBitmap.h"
#include "LoadImage.h"

std::shared_ptr<odb::NativeBitmap> tile0;

int px = 160;
int py = 100;
int vx = 0;
int vy = 0;
int counter = 0;
std::array< unsigned int, 320 * 200> imageBuffer;
std::array< unsigned char, 320 * 200 >  buffer;

void initMode13h() {
  union REGS regs;

  regs.h.ah = 0x00;
  regs.h.al = 0x4;
  int86(0x10,&regs,&regs);
}

void copyImageBufferToVideoMemory() {
  
  int origin = 0;
  int offset = 0;
  
  for ( int y = 0; y < 200; ++y ) {
    
    if ( y < 0 || y >= 200 ) {
      continue;
    }
    
    for ( int x = 0; x < 320; ++x ) {
      
      if ( x < 0 || x >= 320 ) {
	    continue;
      }
      
      offset = (y * 320) + x;
      origin = imageBuffer[ offset ];
      
      if ( buffer[ offset ] != origin ) {
        union REGS regs;
        regs.h.ah = 0x0C;
        regs.h.al = origin;
        regs.h.bh = 0;
        regs.x.cx = x;
        regs.x.dx = y;
        int86(0x10,&regs,&regs);
      }
      
      buffer[ offset ] = origin;
    }
  }
}

void render() {  
  std::fill( std::begin( imageBuffer), std::end( imageBuffer ), 0 );

  int y0 = 0;
  int y1 = 0;
  int x0 = 0;
  int x1 = 0;

  y0 = py;
  y1 = 32 + py;
  x0 = px;
  x1 = 32 + px;
    int* pixelData = tile0->getPixelData();

  for ( int y = y0; y < y1; ++y ) {
    for ( int x = x0; x < x1; ++x ) {
        int pixel = (pixelData[ ( 32 * ( y - y0 )) + ( x - x0 )]);

	    imageBuffer[ ( 320 * y ) + x ] = pixel;
    }
  }

  copyImageBufferToVideoMemory();
    usleep(  20000 );
}

int main( int argc, char **argv ) {
    tile0 = odb::loadBitmap("tile0.png");

  bool done = false;

  char lastKey = 0;
  
  initMode13h();

  while( !done ) {

    px += vx;
    py += vy;

    //    vx = vy = 0;
    
    if ( px < 0 ) {
      px = 0;
    }

    if ( py < 0 ) {
      py = 0;
    }

    if ( px >= 315 ) {
      px = 315;
    }

    if ( py >= 195 ) {
      py = 195;
    }
    
    int level = 0;
    ++counter;
    render();
    
    while( kbhit() ) {
      lastKey = getch();
      switch (lastKey){
      case 'q':
	done = true;
	break;
      case 'w':
	vy = -5 + level;
	break;
      case 's':
	vy = 5 + level;
	break;
      case 'a':
	vx = -5 + level;
	break;
      case 'd':
	vx = 5 + level;
	break;
      }
    }
  }
 
  return 0;
}
