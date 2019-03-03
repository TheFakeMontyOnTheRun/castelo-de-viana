#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "Common.h"
#include "NativeBitmap.h"
#include "CPackedFileReader.h"
#include "Renderer.h"
#include "LoadImage.h"

#include "Game.h"
#include "Renderer.h"

#include <SDL/SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

double timeRendering = 0;
int desiredTimeSlice = 75;
double t0;
double t1;
double ms;
SDL_Surface *video;
struct ControlState toReturn;
enum EVideoType videoType = kVGA;
uint32_t mPalette[256];

struct ControlState getControlState() {

  SDL_Event event;

  while ( SDL_PollEvent( &event ) ) {
    
    if( event.type == SDL_QUIT ) {
#ifndef __EMSCRIPTEN__
      exit(0);
#endif
    } else if ( event.type == SDL_KEYDOWN ) {
          switch ( event.key.keysym.sym ) {
              case SDLK_q:
#ifndef __EMSCRIPTEN__
                  exit(0);
#endif


              case SDLK_ESCAPE:
                  toReturn.escape = TRUE;
                  break;

              case SDLK_RETURN:
                  toReturn.enter = TRUE;
                  break;

              case SDLK_LEFT:
                  toReturn.moveLeft = TRUE;
                  break;

              case SDLK_RIGHT:
                  toReturn.moveRight = TRUE;
                  break;

              case SDLK_UP:
                  toReturn.moveUp = TRUE;
                  break;

              case SDLK_l:
                  toReturn.secret = TRUE;
                  break;

              case SDLK_DOWN:
                  toReturn.moveDown = TRUE;
                  break;


              case SDLK_LSHIFT:
              case SDLK_RSHIFT:
                  toReturn.jump = TRUE;
                  break;

              case SDLK_LCTRL:
              case SDLK_RCTRL:
                  toReturn.sword = TRUE;
                  break;

              case SDLK_SPACE:
                  toReturn.fireArrow = TRUE;
                  break;
          }
      } else if ( event.type == SDL_KEYUP ) {
          switch ( event.key.keysym.sym ) {


              case SDLK_ESCAPE:
                  toReturn.escape = FALSE;
                  break;

              case SDLK_RETURN:
                  toReturn.enter = FALSE;
                  break;

              case SDLK_LEFT:
                  toReturn.moveLeft = FALSE;
                  break;

              case SDLK_RIGHT:
                  toReturn.moveRight = FALSE;
                  break;

              case SDLK_UP:
                  toReturn.moveUp = FALSE;
                  break;

              case SDLK_l:
                  toReturn.secret = FALSE;
                  break;

              case SDLK_DOWN:
                  toReturn.moveDown = FALSE;
                  break;


              case SDLK_LSHIFT:
              case SDLK_RSHIFT:
                  toReturn.jump = FALSE;
                  break;

              case SDLK_LCTRL:
              case SDLK_RCTRL:
                  toReturn.sword = FALSE;
                  break;

              case SDLK_SPACE:
                  toReturn.fireArrow = FALSE;
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
#ifndef __EMSCRIPTEN__
  SDL_Delay(50);
#endif
  SDL_Flip(video);
}

void soundFrequency(int frequency) {
}

void muteSound() {
}

void onQuit() {
  SDL_Quit();
}


void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
        case 1:
            *p = pixel;
            break;

        case 2:
            *(Uint16 *)p = pixel;
            break;

        case 3:
            if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                p[0] = (pixel >> 16) & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = pixel & 0xff;
            } else {
                p[0] = pixel & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = (pixel >> 16) & 0xff;
            }
            break;

        case 4:
            *(Uint32 *)p = pixel;
            break;
    }
}

void copyImageBufferToVideoMemory(uint8_t* imageBuffer ) {

  SDL_Rect rect;
  int pos = 0;

  rect.x = 0;
  rect.y = 0;
  rect.w = 320;
  rect.h = 200;
  SDL_FillRect( video, &rect, SDL_MapRGB( video->format, 0, 0, 0 ) );
    SDL_Surface* screen = video;
    if ( SDL_MUSTLOCK(screen) ) {
        if ( SDL_LockSurface(screen) < 0 ) {
            fprintf(stderr, "Can't lock screen: %s\n", SDL_GetError());
            return;
        }
    }

    int y = 0;
    for ( y = 0; y < 200; ++y ) {
        int x = 0;
        for ( x = 0; x < 320; ++x ) {
          rect.x = x;
          rect.y = y;
          rect.w = 1;
          rect.h = 1;

          pos = ( 320 * y ) + x;

        putpixel(video, x, y, mPalette[imageBuffer[ pos ]]);

    }
  }
    if ( SDL_MUSTLOCK(screen) ) {
        SDL_UnlockSurface(screen);
    }

}

#ifdef __EMSCRIPTEN__
void enterFullScreenMode() {
    EmscriptenFullscreenStrategy s;
    memset(&s, 0, sizeof(s));
    s.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT;
    s.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE;
    s.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
    emscripten_enter_soft_fullscreen(0, &s);
}
#endif



uint8_t getPaletteEntry( uint32_t origin ) {

    uint8_t shade = 0;


    shade += (((((origin & 0x0000FF)      ) << 2) >> 8)) << 6;
    shade += (((((origin & 0x00FF00)  >> 8) << 3) >> 8)) << 3;
    shade += (((((origin & 0xFF0000) >> 16) << 3) >> 8)) << 0;

    return shade;
}


void initVideoFor(enum EVideoType unused) {
  SDL_Init( SDL_INIT_VIDEO );

    memset(&toReturn, 0, sizeof(struct ControlState));

  video = SDL_SetVideoMode( 320, 200, 0, SDL_ASYNCBLIT | SDL_HWACCEL | SDL_HWSURFACE);

#ifdef __EMSCRIPTEN__
    enterFullScreenMode();
#endif

    int r, g, b;
    for (r = 0; r < 256; r += 16) {
        for (g = 0; g < 256; g += 8) {
            for (b = 0; b < 256; b += 8) {
                uint32_t pixel = 0xFF000000 + (r << 16) + (g << 8) + (b);
                uint8_t paletteEntry = getPaletteEntry(pixel);
                mPalette[paletteEntry] = pixel;
            }
        }
    }

}

const char* getAssetsPath() {
    return "sdl.pfs";
}