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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ACCESS_DATA_H
#define ACCESS_DATA_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/rect.h"
#include "graphics/surface.h"

namespace Access {

class AccessEngine;

class Manager {
protected:
	AccessEngine *_vm;
public:
	Manager(AccessEngine *vm) : _vm(vm) {}
};

struct TimerEntry {
	int _initTm;
	int _timer;
	bool _flag;

	TimerEntry() {
		_initTm = _timer = 0;
		_flag = false;
	}
};

class ExtraCell {
public:
	int _vidTable;
	int _vidTable1;
	int _vidSTable;
	int _vidSTable1;
};

class SpriteFrame : public Graphics::Surface {
public:
	uint16 _width, _height;
	byte *_data;
	uint32 _size;

	~SpriteFrame();
};

class SpriteResource {
public:
	Common::Array<SpriteFrame *> _frames;
public:
	SpriteResource(AccessEngine *vm, const byte *data, uint32 size);
	~SpriteResource();

	int getCount() { return _frames.size(); }

	SpriteFrame *getFrame(int idx) { return _frames[idx]; }
};

} // End of namespace Access

#endif /* ACCESS_DATA_H */
