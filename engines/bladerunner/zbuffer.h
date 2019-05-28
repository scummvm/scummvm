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

#ifndef BLADERUNNER_ZBUFFER_H
#define BLADERUNNER_ZBUFFER_H

#include "bladerunner/bladerunner.h"

#include "common/rect.h"

namespace BladeRunner {

#define MAX_DIRTY_RECTS 20

class ZBufferDirtyRects {
	int          _count;
	Common::Rect _rects[MAX_DIRTY_RECTS];

public:
	ZBufferDirtyRects() :
		_count(0)
	{}

	void reset();
	bool add(Common::Rect rect);
	void extendExisting();
	int  getCount() const;
	bool popRect(Common::Rect *rect);
};

class ZBuffer {
	int     _width;
	int     _height;

	uint16 *_zbuf1;
	uint16 *_zbuf2;

	ZBufferDirtyRects *_dirtyRects;

	bool _disabled;

public:
	ZBuffer();
	~ZBuffer();

	void init(int width, int height);
	bool decodeData(const uint8 *data, int size);

	uint16 *getData() const;
	uint16 getZValue(int x, int y) const;

private:
	void blit(Common::Rect rect);

public:
	void mark(Common::Rect rect);
	void clean();
	void resetUpdates();

	// Only called from Scene::resume which is not yet implemented
	void disable();
	void enable();
};

} // End of namespace BladeRunner

#endif
