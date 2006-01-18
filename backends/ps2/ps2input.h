/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */

#ifndef __PS2INPUT_H__
#define __PS2INPUT_H__

#include "common/system.h"

class OSystem_PS2;
class Ps2Pad;

class Ps2Input {
public:
	Ps2Input(OSystem_PS2 *system, bool mouseLoaded, bool kbdLoaded);
	~Ps2Input(void);
	void newRange(uint16 minx, uint16 miny, uint16 maxx, uint16 maxy);
	bool pollEvent(OSystem::Event *event);
	void warpTo(uint16 x, uint16 y);
private:
	int mapKey(int key, int mod);
	bool getKeyEvent(OSystem::Event *event, uint16 buttonCode, bool down);
	OSystem_PS2 *_system;
	Ps2Pad		*_pad;

	uint16 _minx, _maxx, _miny, _maxy;

	uint16 _posX, _posY;
	uint16 _mButtons;
	uint16 _padLastButtons;
	uint32 _lastPadCheck;
	uint16 _padAccel;

	bool _mouseLoaded, _kbdLoaded;
	int _keyFlags;
	static const int _padCodes[16], _padFlags[16];
	static const int _usbToSdlk[0x100];
};

#endif // __PS2INPUT_H__

