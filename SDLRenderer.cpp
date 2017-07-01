//
// Created by monty on 01-07-2017.
//

#include <algorithm>
#include <array>
#include <random>
#include <memory>
#include <fstream>
#include <sstream>
#include "NativeBitmap.h"
#include "LoadImage.h"

#include "Game.h"
#include "Renderer.h"

#include <SDL/SDL.h>

double timeRendering = 0;
int desiredTimeSlice = 75;
double t0;
double t1;
double ms;
SDL_Surface *video;

ControlState getControlState() {
  ControlState toReturn;
  SDL_Event event;
  
  while ( SDL_PollEvent( &event ) ) {
    
    if( event.type == SDL_QUIT ) {
#ifndef __EMSCRIPTEN__
      exit(0);
#endif
    }
    
    if ( event.type == SDL_KEYDOWN ) {
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
    }
  }  
  return toReturn;
}
  
void beginFrame() {
}

void doneWithFrame() {
  SDL_Delay(50);
  SDL_Flip(video);
}

void soundFrequency(int frequency) {
}

void muteSound() {
}

void onQuit() {
  SDL_Quit();
}

void copyImageBufferToVideoMemory(const std::array<unsigned int, 320 * 200>& imageBuffer ) {

  SDL_Rect rect;
  int pos = 0;

  rect.x = 0;
  rect.y = 0;
  rect.w = 640;
  rect.h = 480;
  SDL_FillRect( video, &rect, 0 );
  
  for ( int y = 0; y < 200; ++y ) {
    for ( int x = 0; x < 320; ++x ) {
      rect.x = x * 2;
      rect.y = y * 2;
      rect.w = 2;
      rect.h = 2;

      pos = ( 320 * y ) + x;
      
      SDL_FillRect( video, &rect, imageBuffer[ pos ] );

    }
  }

  
}

void initVideo() {
  SDL_Init( SDL_INIT_EVERYTHING );
  video = SDL_SetVideoMode( 640, 480, 32, 0 );
}

std::string getResPath() {
    return "resSDL/";
}
