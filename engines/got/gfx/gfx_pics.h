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

#ifndef GOT_GFX_GFX_PICS_H
#define GOT_GFX_GFX_PICS_H

#include "common/array.h"
#include "graphics/managed_surface.h"

namespace Got {
namespace Gfx {

/**
 * The original graphics screen was set up to use 4 panes,
 * where each pane had a single pixel, repeating every 4 pixels.
 * Because of this, graphics were stored with all the data for
 * each pane one at a time. This helper methods takes care of
 * "de-paneing" the graphics into a provided surface
*/
extern void convertPaneDataToSurface(const byte *src, Graphics::ManagedSurface &surf);

class GfxPics {
private:
	Graphics::ManagedSurface *_array = nullptr;
	size_t _size = 0;

protected:
public:
	~GfxPics() {
		clear();
	}

	void clear();
	void resize(uint newSize);
	void load(const Common::String &name, int blockSize);

	Graphics::ManagedSurface &operator[](uint idx) {
		return _array[idx];
	}
	size_t size() const {
		return _size;
	}

	GfxPics &operator=(const GfxPics &src);
};

class BgPics : public GfxPics {
private:
	int _area = 1;

public:
	void load();

	bool getArea() const {
		return _area;
	}
	void setArea(int area);
};

class Pics : public GfxPics {
private:
	Common::String _resName;
	int _blockSize = -1;

public:
	Pics(const Common::String &resName, int blockSize = -1,
		 bool immediateLoad = true) : _resName(resName), _blockSize(blockSize) {
		if (immediateLoad)
			load();
	}
	Pics() {}

	void load() {
		GfxPics::load(_resName, _blockSize);
	}
};

} // namespace Gfx
} // namespace Got

#endif
