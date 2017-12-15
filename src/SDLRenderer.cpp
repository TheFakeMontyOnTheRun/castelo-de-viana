//
// Created by monty on 01-07-2017.
//

#include <algorithm>
#include <array>
#include <random>
#include <memory>
#include <fstream>
#include <sstream>
#include <unordered_map>


using std::vector;
#include "NativeBitmap.h"
#include "IFileLoaderDelegate.h"
#include "CPackedFileReader.h"
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
ControlState toReturn;

ControlState getControlState() {

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
                  toReturn.escape = true;
                  break;

              case SDLK_RETURN:
                  toReturn.enter = true;
                  break;

              case SDLK_LEFT:
                  toReturn.moveLeft = true;
                  break;

              case SDLK_RIGHT:
                  toReturn.moveRight = true;
                  break;

              case SDLK_UP:
                  toReturn.moveUp = true;
                  break;

              case SDLK_l:
                  toReturn.secret = true;
                  break;

              case SDLK_DOWN:
                  toReturn.moveDown = true;
                  break;


              case SDLK_LSHIFT:
              case SDLK_RSHIFT:
                  toReturn.jump = true;
                  break;

              case SDLK_LCTRL:
              case SDLK_RCTRL:
                  toReturn.sword = true;
                  break;

              case SDLK_SPACE:
                  toReturn.fireArrow = true;
                  break;
          }
      } else if ( event.type == SDL_KEYUP ) {
          switch ( event.key.keysym.sym ) {


              case SDLK_ESCAPE:
                  toReturn.escape = false;
                  break;

              case SDLK_RETURN:
                  toReturn.enter = false;
                  break;

              case SDLK_LEFT:
                  toReturn.moveLeft = false;
                  break;

              case SDLK_RIGHT:
                  toReturn.moveRight = false;
                  break;

              case SDLK_UP:
                  toReturn.moveUp = false;
                  break;

              case SDLK_l:
                  toReturn.secret = false;
                  break;

              case SDLK_DOWN:
                  toReturn.moveDown = false;
                  break;


              case SDLK_LSHIFT:
              case SDLK_RSHIFT:
                  toReturn.jump = false;
                  break;

              case SDLK_LCTRL:
              case SDLK_RCTRL:
                  toReturn.sword = false;
                  break;

              case SDLK_SPACE:
                  toReturn.fireArrow = false;
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

void copyImageBufferToVideoMemory(const std::array<unsigned int, 320 * 200>& imageBuffer ) {

  SDL_Rect rect;
  int pos = 0;

  rect.x = 0;
  rect.y = 0;
  rect.w = 320;
  rect.h = 200;
  SDL_FillRect( video, &rect, SDL_MapRGB( video->format, 0, 0, 0 ) );
    auto screen = video;
    if ( SDL_MUSTLOCK(screen) ) {
        if ( SDL_LockSurface(screen) < 0 ) {
            fprintf(stderr, "Can't lock screen: %s\n", SDL_GetError());
            return;
        }
    }

    for ( int y = 0; y < 200; ++y ) {
    for ( int x = 0; x < 320; ++x ) {
      rect.x = x;
      rect.y = y;
      rect.w = 1;
      rect.h = 1;

      pos = ( 320 * y ) + x;

        putpixel(video, x, y, imageBuffer[ pos ]);

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

void initVideo() {
  SDL_Init( SDL_INIT_VIDEO );



  video = SDL_SetVideoMode( 320, 200, 0, SDL_ASYNCBLIT | SDL_HWACCEL | SDL_HWSURFACE);

#ifdef __EMSCRIPTEN__
    enterFullScreenMode();
#endif
}

std::string getAssetsPath() {
    return "sdl.pfs";
}