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
		DIR_LEFT  = 1 << 0,
		DIR_RIGHT = 1 << 1,
		DIR_UP    = 1 << 2,
		DIR_DOWN  = 1 << 3
	};

	uint8 dirMask;
	bool button;
	bool code;
	bool pause;
	bool quit;
	char lastChar;
	bool save, load;
	bool fastMode;
	int8 stateSlot;
};

struct SystemStub {
	PlayerInput _pi;

	virtual ~SystemStub() {}

	virtual void init(const char *title) = 0;
	virtual void destroy() = 0;

	virtual void setPalette(uint8 s, uint8 n, const uint8 *buf) = 0;
	virtual void copyRect(uint16 x, uint16 y, uint16 w, uint16 h, const uint8 *buf, uint32 pitch) = 0;

	virtual void processEvents() = 0;
	virtual void sleep(uint32 duration) = 0;
	virtual uint32 getTimeStamp() = 0;
};

extern SystemStub *SystemStub_SDL_create();

} // namespace Awe

#endif
