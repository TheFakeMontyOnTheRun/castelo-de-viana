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

#include "SDL.h"

SDL_Window *window;
SDL_Renderer *renderer;
uint32_t palette[256];

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

struct ControlState toReturn;
enum EVideoType videoType = kVGA;

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

}

void muteSound() {
}

void onQuit() {

}



void copyImageBufferToVideoMemory(uint8_t* imageBuffer ) {
	SDL_Rect rect;
	uint32_t pixel;
	int x, y;

	for (y = 0; y < 200; ++y) {
		for (x = 0; x < 320; ++x) {

			rect.x = 2 * x;
			rect.y = (24 * y) / 10;
			rect.w = 2;
			rect.h = 3;

			pixel = palette[imageBuffer[(320 * y) + x]];

			SDL_SetRenderDrawColor(renderer, (pixel & 0x000000FF) - 0x38,
								   ((pixel & 0x0000FF00) >> 8) - 0x18,
								   ((pixel & 0x00FF0000) >> 16) - 0x10, 255);
			SDL_RenderFillRect(renderer, &rect);
		}
	}

	SDL_RenderPresent(renderer);

	SDL_Delay(1000 / 60);
#ifndef __EMSCRIPTEN__
#endif
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

	int r, g, b;
	assert(unused != kCGA);

	SDL_Init(SDL_INIT_EVERYTHING);
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
	window =
			SDL_CreateWindow("The Mistral Report", SDL_WINDOWPOS_CENTERED,
							 SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);

	renderer = SDL_CreateRenderer(window, -1, 0);

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

const char* getAssetsPath() {
	return "sdl.pfs";
}
