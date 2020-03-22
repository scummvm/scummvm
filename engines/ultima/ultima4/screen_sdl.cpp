/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/ultima4/config.h"
#include "ultima/ultima4/context.h"
#include "ultima/ultima4/cursors.h"
#include "ultima/ultima4/debug.h"
#include "ultima/ultima4/dungeonview.h"
#include "ultima/ultima4/error.h"
#include "ultima/ultima4/event.h"
#include "ultima/ultima4/graphics/image.h"
#include "ultima/ultima4/graphics/imagemgr.h"
#include "ultima/ultima4/intro.h"
#include "ultima/ultima4/savegame.h"
#include "ultima/ultima4/settings.h"
#include "ultima/ultima4/scale.h"
#include "ultima/ultima4/screen.h"
#include "ultima/ultima4/graphics/tileanim.h"
#include "ultima/ultima4/graphics/tileset.h"
#include "ultima/ultima4/ultima4.h"
#include "ultima/ultima4/u4_sdl.h"
#include "ultima/ultima4/u4file.h"
#include "ultima/ultima4/utils.h"
#include "common/system.h"
#include "engines/util.h"

namespace Ultima {
namespace Ultima4 {

using Std::vector;

struct Cursor {
	Graphics::ManagedSurface _surface;
	Common::Point _hotspot;
};

Cursor *cursors[5];
Scaler filterScaler;

Cursor *screenInitCursor(const char *const xpm[]);

extern bool verbose;


void screenRefreshThreadInit();
void screenRefreshThreadEnd();

void screenInit_sys() {
#ifdef TODO
	/* start SDL */
	if (u4_SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
		errorFatal("unable to init SDL: %s", SDL_GetError());
	SDL_EnableUNICODE(1);
	SDL_SetGamma(settings.gamma / 100.0f, settings.gamma / 100.0f, settings.gamma / 100.0f);
	atexit(SDL_Quit);

	SDL_WM_SetCaption("Ultima IV", NULL);
#ifdef ICON_FILE
	SDL_WM_SetIcon(SDL_LoadBMP(ICON_FILE), NULL);
#endif

	if (!SDL_SetVideoMode(320 * settings.scale, 200 * settings.scale, 0, SDL_HWSURFACE | SDL_ANYFORMAT | (settings.fullscreen ? SDL_FULLSCREEN : 0)))
		errorFatal("unable to set video: %s", SDL_GetError());

	if (verbose) {
		char driver[32];
		printf("screen initialized [screenInit()], using %s video driver\n", SDL_VideoDriverName(driver, sizeof(driver)));
	}
#else
	initGraphics(320 * settings._scale, 200 * settings._scale);
#endif
	/* enable or disable the mouse cursor */
	if (settings._mouseOptions.enabled) {
		g_system->showMouse(true);
#ifdef TODO
		cursors[0] = SDL_GetCursor();
#endif
		cursors[1] = screenInitCursor(w_xpm);
		cursors[2] = screenInitCursor(n_xpm);
		cursors[3] = screenInitCursor(e_xpm);
		cursors[4] = screenInitCursor(s_xpm);
	} else {
		g_system->showMouse(false);
	}

	filterScaler = scalerGet(settings._filter);
	if (!filterScaler)
		errorFatal("%s is not a valid filter", settings._filter.c_str());

	screenRefreshThreadInit();
}

void screenDelete_sys() {
	screenRefreshThreadEnd();
	delete cursors[0];
	delete cursors[1];
	delete cursors[2];
	delete cursors[3];
	delete cursors[4];
#ifdef TODO
	u4_SDL_QuitSubSystem(SDL_INIT_VIDEO);
#endif
}

/**
 * Attempts to iconify the screen.
 */
void screenIconify() {
#ifdef TODO
	SDL_WM_IconifyWindow();
#endif
}

/**
 * Force a redraw.
 */

//SDL_mutex *screenLockMutex = NULL;
int frameDuration = 0;

void screenLock() {
//	SDL_mutexP(screenLockMutex);
}

void screenUnlock() {
//	SDL_mutexV(screenLockMutex);
}

void screenRedrawScreen() {
#ifdef TODO
	screenLock();
	SDL_UpdateRect(SDL_GetVideoSurface(), 0, 0, 0, 0);
	screenUnlock();
#else
	g_system->updateScreen();
#endif
}

void screenRedrawTextArea(int x, int y, int width, int height) {
#ifdef TODO
	screenLock();
	SDL_UpdateRect(SDL_GetVideoSurface(), x * CHAR_WIDTH * settings.scale, y * CHAR_HEIGHT * settings.scale, width * CHAR_WIDTH * settings.scale, height * CHAR_HEIGHT * settings.scale);
	screenUnlock();
#else
	g_system->updateScreen();
#endif
}

void screenWait(int numberOfAnimationFrames) {
	g_system->delayMillis(numberOfAnimationFrames * frameDuration);
}

bool continueScreenRefresh = true;
//SDL_Thread *screenRefreshThread = NULL;

int screenRefreshThreadFunction(void *unused) {
#ifdef TODO
	while (continueScreenRefresh) {
		SDL_Delay(frameDuration);
		screenRedrawScreen();
	}
#endif
	return 0;
}

void screenRefreshThreadInit() {
#ifdef TODO
	screenLockMutex = SDL_CreateMutex();;

	frameDuration = 1000 / settings.screenAnimationFramesPerSecond;

	continueScreenRefresh = true;
	if (screenRefreshThread) {
		errorWarning("Screen refresh thread already exists.");
		return;
	}

	screenRefreshThread = SDL_CreateThread(screenRefreshThreadFunction, NULL);
	if (!screenRefreshThread) {
		errorWarning(SDL_GetError());
		return;
	}
#endif
}

void screenRefreshThreadEnd() {
#ifdef TODO
	continueScreenRefresh = false;
	SDL_WaitThread(screenRefreshThread, NULL);
	screenRefreshThread = NULL;
#endif
}


/**
 * Scale an image up.  The resulting image will be scale * the
 * original dimensions.  The original image is no longer deleted.
 * n is the number of tiles in the image; each tile is filtered
 * seperately. filter determines whether or not to filter the
 * resulting image.
 */
Image *screenScale(Image *src, int scale, int n, int filter) {
	Image *dest = NULL;
	bool isTransparent;
	unsigned int transparentIndex;
	bool alpha = src->isAlphaOn();

	if (n == 0)
		n = 1;

	isTransparent = src->getTransparentIndex(transparentIndex);
	src->alphaOff();

	while (filter && filterScaler && (scale % 2 == 0)) {
		dest = (*filterScaler)(src, 2, n);
		src = dest;
		scale /= 2;
	}
	if (scale == 3 && scaler3x(settings._filter)) {
		dest = (*filterScaler)(src, 3, n);
		src = dest;
		scale /= 3;
	}

	if (scale != 1)
		dest = (*scalerGet("point"))(src, scale, n);

	if (!dest)
		dest = Image::duplicate(src);

	if (isTransparent)
		dest->setTransparentIndex(transparentIndex);

	if (alpha)
		src->alphaOn();




	return dest;
}

/**
 * Scale an image down.  The resulting image will be 1/scale * the
 * original dimensions.  The original image is no longer deleted.
 */
Image *screenScaleDown(Image *src, int scale) {
	int x, y;
	Image *dest;
	bool isTransparent;
	unsigned int transparentIndex;
	bool alpha = src->isAlphaOn();

	isTransparent = src->getTransparentIndex(transparentIndex);

	src->alphaOff();

	dest = Image::create(src->width() / scale, src->height() / scale, src->isIndexed(), Image::HARDWARE);
	if (!dest)
		return NULL;

	if (!dest)
		dest = Image::duplicate(src);

	if (dest->isIndexed())
		dest->setPaletteFromImage(src);

	for (y = 0; y < src->height(); y += scale) {
		for (x = 0; x < src->width(); x += scale) {
			unsigned int index;
			src->getPixelIndex(x, y, index);
			dest->putPixelIndex(x / scale, y / scale, index);
		}
	}

	if (isTransparent)
		dest->setTransparentIndex(transparentIndex);

	if (alpha)
		src->alphaOn();

	return dest;
}

/**
 * Create a cursor object from an xpm.  Derived from example in
 * SDL documentation project.
 */
#define CURSORSIZE 32

Cursor *screenInitCursor(const char *const xpm[]) {
#ifdef TODO
	int i, row, col;
	uint8 data[(CURSORSIZE / 8)*CURSORSIZE];
	uint8 mask[(CURSORSIZE / 8)*CURSORSIZE];
	int hot_x, hot_y;

	i = -1;
	for (row = 0; row < CURSORSIZE; row++) {
		for (col = 0; col < CURSORSIZE; col++) {
			if (col % 8) {
				data[i] <<= 1;
				mask[i] <<= 1;
			} else {
				i++;
				data[i] = mask[i] = 0;
			}
			switch (xpm[4 + row][col]) {
			case 'X':
				data[i] |= 0x01;
				mask[i] |= 0x01;
				break;
			case '.':
				mask[i] |= 0x01;
				break;
			case ' ':
				break;
			}
		}
	}
	sscanf(xpm[4 + row], "%d,%d", &hot_x, &hot_y);
	return SDL_CreateCursor(data, mask, CURSORSIZE, CURSORSIZE, hot_x, hot_y);
#else
	return nullptr;
#endif
}

void screenSetMouseCursor(MouseCursor cursor) {
#ifdef TODO
	static int current = 0;

	if (cursor != current) {
		SDL_SetCursor(cursors[cursor]);
		current = cursor;
	}
#endif
}

} // End of namespace Ultima4
} // End of namespace Ultima
