/** @file sdlengine.cpp
	@brief
	This file contains SDL engine routines

	TwinEngine: a Little Big Adventure engine

	Copyright (C) 2013 The TwinEngine team
	Copyright (C) 2008-2013 Prequengine team
	Copyright (C) 2002-2007 The TwinEngine team

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#ifndef MACOSX
#include <SDL/SDL_mixer.h>
#else
#include <SDL_mixer/SDL_mixer.h>
#endif

#ifdef GAMEMOD
#ifndef MACOSX
#include <SDL/SDL_ttf.h>
#else
#include <SDL_ttf/SDL_ttf.h>
#endif

#endif

#include "sdlengine.h"
#include "main.h"
#include "screens.h"
#include "music.h"
#include "lbaengine.h"
#include "debug.h"
#include "keyboard.h"
#include "redraw.h"

/** SDL exit callback */
//static void atexit_callback(void);

/** Original audio frequency */
#define ORIGINAL_GAME_FREQUENCY		11025
/** High quality audio frequency */
#define HIGH_QUALITY_FREQUENCY		44100

/** Main SDL screen surface buffer */
SDL_Surface *screen = NULL;
/** Auxiliar SDL screen surface buffer */
SDL_Surface *screenBuffer = NULL;
/** SDL screen color */
SDL_Color screenColors[256];
/** Auxiliar surface table  */
SDL_Surface *surfaceTable[16];

#ifdef GAMEMOD
TTF_Font *font;
#endif

/** SDL exit callback */
//static void atexit_callback(void) {
//	sdlClose();
//}

void sdlClose() {
	stopTrackMusic();
	stopMidiMusic();
	Mix_CloseAudio();
#ifdef GAMEMOD
	TTF_Quit();
#endif
	SDL_Quit();
	exit(0);
}


/** SDL initializer
	@return SDL init state */
int sdlInitialize() {
	uint8 *keyboard;
	int32 size;
	int32 i;
	int32 freq;
	//SDL_Surface* icon;

	Uint32 rmask, gmask, bmask;
//	Uint32 amask;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
#endif

	printf("Initialising SDL device. Please wait...\n");

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}

#ifdef GAMEMOD
	if (TTF_Init() < 0) {
		fprintf(stderr, "Couldn't initialize TTF: %s\n", SDL_GetError());
		exit(1);
	}

	font = TTF_OpenFont("FreeMono.ttf", 12);

	if (font == NULL) {
		fprintf(stderr, "Couldn't load %d pt font from %s: %s\n", 12, "FreeMono.ttf", SDL_GetError());
		exit(2);
	}

	TTF_SetFontStyle(font, 0);
#endif


	/*icon = SDL_LoadBMP("icon.bmp");
	SDL_WM_SetIcon(icon, NULL);*/

	if (cfgfile.Debug) {
		SDL_version compile_version;
		const SDL_version *link_version;
		SDL_VERSION(&compile_version);
		printf("Compiled with SDL version: %d.%d.%d\n", compile_version.major, compile_version.minor, compile_version.patch);
		link_version = SDL_Linked_Version();
		printf("Running with SDL version: %d.%d.%d\n\n", link_version->major, link_version->minor, link_version->patch);
	}

	printf("Initialising Sound device. Please wait...\n\n");

	// Verify if we want to use high quality sounds
	if (cfgfile.Sound > 1)
		freq = HIGH_QUALITY_FREQUENCY;
	else
		freq = ORIGINAL_GAME_FREQUENCY;

	if (Mix_OpenAudio(freq, AUDIO_S16, 2, 256) < 0) {
		printf("Mix_OpenAudio: %s\n", Mix_GetError());
		exit(1);
	}

	Mix_AllocateChannels(32);

	SDL_WM_SetCaption("Little Big Adventure: TwinEngine", "twin-e");
	SDL_PumpEvents();

	keyboard = SDL_GetKeyState(&size);

	keyboard[SDLK_RETURN] = 0;

	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_SWSURFACE);

	if (screen == NULL) {
		fprintf(stderr, "Couldn't set 640x480x8 video mode: %s\n\n", SDL_GetError());
		exit(1);
	}

	for (i = 0; i < 16; i++) {
		surfaceTable[i] = SDL_CreateRGBSurface(SDL_SWSURFACE, SCREEN_WIDTH, SCREEN_HEIGHT, 32, rmask, gmask, bmask, 0);
	}

	atexit(SDL_Quit);

	return 0;
}

/** Frames per second sdl delay
	@param fps frames per second */
void fpsCycles(int32 fps) {
	SDL_Delay(1000 / (fps));
}

/** Deplay certain seconds till proceed
	@param time time in seconds to delay */
void sdldelay(uint32 time) {
	SDL_Delay(time);
}

/** Deplay certain seconds till proceed - Can skip delay
	@param time time in seconds to delay */
void delaySkip(uint32 time) {
	uint32 startTicks = SDL_GetTicks();
	uint32 stopTicks = 0;
	skipIntro = 0;
	do {
		readKeys();
		if (skipIntro == 1)
			break;
		stopTicks = SDL_GetTicks() - startTicks;
		SDL_Delay(1);
		//lbaTime++;
	} while (stopTicks <= time);
}

/** Set a new palette in the SDL screen buffer
	@param palette palette to set */
void setPalette(uint8 * palette) {
	SDL_Color *screenColorsTemp = (SDL_Color *) palette;

	SDL_SetColors(screenBuffer, screenColorsTemp, 0, 256);
	SDL_BlitSurface(screenBuffer, NULL, screen, NULL);
	SDL_UpdateRect(screen, 0, 0, 0, 0);
}

/** Fade screen from black to white */
void fadeBlackToWhite() {
	int32 i;

	SDL_Color colorPtr[256];

	SDL_UpdateRect(screen, 0, 0, 0, 0);

	for (i = 0; i < 256; i += 3) {
		memset(colorPtr, i, 1024);
		SDL_SetPalette(screen, SDL_PHYSPAL, colorPtr, 0, 256);
	}
}

/** Blit surface in the screen */
void flip() {
	SDL_BlitSurface(screenBuffer, NULL, screen, NULL);
	SDL_UpdateRect(screen, 0, 0, 0, 0);
}

/** Blit surface in the screen in a determinate area
	@param left left position to start copy
	@param top top position to start copy
	@param right right position to start copy
	@param bottom bottom position to start copy */
void copyBlockPhys(int32 left, int32 top, int32 right, int32 bottom) {
	SDL_Rect rectangle;

	rectangle.x = left;
	rectangle.y = top;
	rectangle.w = right - left + 1 ;
	rectangle.h = bottom - top + 1 ;

	SDL_BlitSurface(screenBuffer, &rectangle, screen, &rectangle);
	SDL_UpdateRect(screen, left, top, right - left + 1, bottom - top + 1);
}

/** Create SDL screen surface
	@param buffer screen buffer to blit surface
	@param width screen width size
	@param height screen height size */
void initScreenBuffer(uint8 *buffer, int32 width, int32 height) {
	screenBuffer = SDL_CreateRGBSurfaceFrom(buffer, width, height, 8, SCREEN_WIDTH, 0, 0, 0, 0);
}

/** Cross fade feature
	@param buffer screen buffer
	@param palette new palette to cross fade */
void crossFade(uint8 *buffer, uint8 *palette) {
	int32 i;
	SDL_Surface *backupSurface;
	SDL_Surface *newSurface;
	SDL_Surface *tempSurface;
	Uint32 rmask, gmask, bmask;
//	Uint32 amask;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
#endif

	backupSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, SCREEN_WIDTH, SCREEN_HEIGHT, 32, rmask, gmask, bmask, 0);
	newSurface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, SCREEN_WIDTH, SCREEN_HEIGHT, 32, rmask, gmask, bmask, 0);

	tempSurface = SDL_CreateRGBSurfaceFrom(buffer, SCREEN_WIDTH, SCREEN_HEIGHT, 8, SCREEN_WIDTH, 0, 0, 0, 0);
	SDL_SetColors(tempSurface, (SDL_Color *) palette, 0, 256);

	SDL_BlitSurface(screen, NULL, backupSurface, NULL);
	SDL_BlitSurface(tempSurface, NULL, newSurface, NULL);

	for (i = 0; i < 8; i++) {
		SDL_BlitSurface(backupSurface, NULL, surfaceTable[i], NULL);
		SDL_SetAlpha(newSurface, SDL_SRCALPHA | SDL_RLEACCEL, i * 32);
		SDL_BlitSurface(newSurface, NULL, surfaceTable[i], NULL);
		SDL_BlitSurface(surfaceTable[i], NULL, screen, NULL);
		SDL_UpdateRect(screen, 0, 0, 0, 0);
		delaySkip(50);
	}

	SDL_BlitSurface(newSurface, NULL, screen, NULL);
	SDL_UpdateRect(screen, 0, 0, 0, 0);

	SDL_FreeSurface(backupSurface);
	SDL_FreeSurface(newSurface);
	SDL_FreeSurface(tempSurface);
}

/** Switch between window and fullscreen modes */
void toggleFullscreen() {
	cfgfile.FullScreen = 1 - cfgfile.FullScreen;
	SDL_FreeSurface(screen);

	reqBgRedraw = 1;

	if (cfgfile.FullScreen) {
		screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_SWSURFACE);
		copyScreen(workVideoBuffer, frontVideoBuffer);
		SDL_ShowCursor(1);
	} else {
		screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_SWSURFACE | SDL_FULLSCREEN);
		copyScreen(workVideoBuffer, frontVideoBuffer);

#ifdef _DEBUG
		SDL_ShowCursor(1);
#else
		SDL_ShowCursor(0);
#endif
	}
}

/** Handle keyboard pressed keys */
void readKeys() {
	SDL_Event event;
	int32 localKey;
	int32 i, j, size;
	int32 find = 0;
	int16 temp;
	uint8 temp2;
	int8 found = 0;
	uint8 *keyboard;

	localKey = 0;
	skippedKey = 0;
	skipIntro = 0;

	SDL_PumpEvents();

	keyboard = SDL_GetKeyState(&size);

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
			sdlClose();
			break;
		case SDL_MOUSEBUTTONDOWN:
			switch (event.button.button) {
			case SDL_BUTTON_RIGHT:
				rightMouse = 1;
				break;
			case SDL_BUTTON_LEFT:
				leftMouse = 1;
				break;
			}
			break;
		case SDL_KEYUP:
			pressedKey = 0;
			break;
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_ESCAPE:
				localKey = 0x1;
				break;
			case SDLK_SPACE:
				localKey = 0x39;
				break;
			case SDLK_RETURN:
			case SDLK_KP_ENTER:
				localKey = 0x1C;
				break;
			case SDLK_LSHIFT:
			case SDLK_RSHIFT:
				localKey = 0x36;
				break;
			case SDLK_LALT:
			case SDLK_RALT:
				localKey = 0x38;
				break;
			case SDLK_LCTRL:
			case SDLK_RCTRL:
				localKey = 0x1D;
				break;
			case SDLK_PAGEUP:
				localKey = 0x49;
				break;
			case SDLK_p:  // pause
				localKey = 0x19;
				break;
			case SDLK_h:  // holomap
				localKey = 0x23;
				break;
			case SDLK_j:
				localKey = 0x24;
				break;
			case SDLK_w: // Especial key to do the action
				localKey = 0x11;
				break;
			case SDLK_F1:
				localKey = 0x3B;
				break;
			case SDLK_F2:
				localKey = 0x3C;
				break;
			case SDLK_F3:
				localKey = 0x3D;
				break;
			case SDLK_F4:
				localKey = 0x3E;
				break;
			case SDLK_F6:
				localKey = 0x40;
				break;
			case SDLK_F12:
				toggleFullscreen();
				break;
#ifdef GAMEMOD
			case SDLK_r:  // next room
				localKey = 0x13;
				break;
			case SDLK_f:  // previous room
				localKey = 0x21;
				break;
			case SDLK_t:  // apply celling grid
				localKey = 0x14;
				break;
			case SDLK_g:  // increase celling grid index
				localKey = 0x22;
				break;
			case SDLK_b:  // decrease celling grid index
				localKey = 0x30;
				break;
#endif
			default:
				break;
			}
			break;
		}
	}

	for (j = 0; j < size; j++) {
		if (keyboard[j]) {
			switch (j) {
			case SDLK_RETURN:
			case SDLK_KP_ENTER:
				localKey = 0x1C;
				break;
			case SDLK_SPACE:
				localKey = 0x39;
				break;
			case SDLK_UP:
			case SDLK_KP8:
				localKey = 0x48;
				break;
			case SDLK_DOWN:
			case SDLK_KP2:
				localKey = 0x50;
				break;
			case SDLK_LEFT:
			case SDLK_KP4:
				localKey = 0x4B;
				break;
			case SDLK_RIGHT:
			case SDLK_KP6:
				localKey = 0x4D;
				break;
			case SDLK_LCTRL:
			case SDLK_RCTRL:
				localKey = 0x1D;
				break;
			/*case SDLK_LSHIFT:
			case SDLK_RSHIFT:
				localKey = 0x36;
				break;*/
			case SDLK_LALT:
			case SDLK_RALT:
				localKey = 0x38;
				break;
			case SDLK_F1:
				localKey = 0x3B;
				break;
			case SDLK_F2:
				localKey = 0x3C;
				break;
			case SDLK_F3:
				localKey = 0x3D;
				break;
			case SDLK_F4:
				localKey = 0x3E;
				break;
#ifdef GAMEMOD
				// change grid camera
			case SDLK_s:
				localKey = 0x1F;
				break;
			case SDLK_x:
				localKey = 0x2D;
				break;
			case SDLK_z:
				localKey = 0x2C;
				break;
			case SDLK_c:
				localKey = 0x2E;
				break;
#endif
			}
		}

		for (i = 0; i < 28; i++) {
			if (pressedKeyMap[i] == localKey) {
				find = i;
				found = 1;
				break;
			}
		}

		if (found != 0) {
			temp = pressedKeyCharMap[find];
			temp2 = temp & 0x00FF;

			if (temp2 == 0) {
				// pressed valid keys
				if (!(localKey & 0x80)) {
					pressedKey |= (temp & 0xFF00) >> 8;
				} else {
					pressedKey &= -((temp & 0xFF00) >> 8);
				}
			}
			// pressed inactive keys
			else {
				skippedKey |= (temp & 0xFF00) >> 8;
			}
		}

		//if (found==0) {
			skipIntro = localKey;
		//}
	}
}

#ifdef GAMEMOD

/** Display SDL text in screen
	@param X X coordinate in screen
	@param Y Y coordinate in screen
	@param string text to display
	@param center if the text should be centered accoding with the giving positions */
void ttfDrawText(int32 X, int32 Y, int8 *string, int32 center) {
	SDL_Color white = { 0xFF, 0xFF, 0xFF, 0 };
	SDL_Color *forecol = &white;
	SDL_Rect rectangle;

	SDL_Surface *text;

	text = TTF_RenderText_Solid(font, string, *forecol);

	if (center)
		rectangle.x = X - (text->w / 2);
	else
		rectangle.x = X;
	rectangle.y = Y - 2;
	rectangle.w = text->w;
	rectangle.h = text->h;

	SDL_BlitSurface(text, NULL, screenBuffer, &rectangle);
	SDL_FreeSurface(text);
}

/** Gets SDL mouse positions
	@param mouseData structure that contains mouse position info */
void getMousePositions(MouseStatusStruct *mouseData) {
	SDL_GetMouseState(&mouseData->X, &mouseData->Y);

	mouseData->left = leftMouse;
	mouseData->right = rightMouse;

	leftMouse = 0;
	rightMouse = 0;
}

#endif
