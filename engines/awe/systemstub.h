/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef AWE_SYSTEMSTUB_H
#define AWE_SYSTEMSTUB_H

#include "awe/intern.h"

namespace Awe {

struct PlayerInput {
	enum {
		DIR_LEFT = 1 << 0,
		DIR_RIGHT = 1 << 1,
		DIR_UP = 1 << 2,
		DIR_DOWN = 1 << 3
	};

	uint8_t dirMask;
	bool action; // run,shoot
	bool jump;
	bool code;
	bool pause;
	bool quit;
	bool back;
	char lastChar;
	bool fastMode;
	bool screenshot;
};

struct DisplayMode {
	enum {
		WINDOWED,
		FULLSCREEN,    // stretch
		FULLSCREEN_AR, // 16:10 aspect ratio
	} mode;
	int width, height; // window dimensions
	bool opengl;       // GL renderer
};

struct SystemStub {
	typedef void (*AudioCallback)(void *param, uint8_t *stream, int len);

	PlayerInput _pi;
	DisplayMode _dm;

	SystemStub() {
		memset(&_pi, 0, sizeof(_pi));
	}
	virtual ~SystemStub() {
	}

	virtual void init(const char *title, const DisplayMode *dm) = 0;
	virtual void fini() = 0;

	// GL rendering
	virtual void prepareScreen(int &w, int &h, float ar[4]) = 0;
	virtual void updateScreen() = 0;
	// framebuffer rendering
	virtual void setScreenPixels555(const uint16_t *data, int w, int h) = 0;

	virtual void processEvents() = 0;
	virtual void sleep(uint32_t duration) = 0;
	virtual uint32_t getTimeStamp() = 0;
};

extern SystemStub *SystemStub_SDL_create();

} // namespace Awe

#endif
