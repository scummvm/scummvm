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

#ifndef BACKENDS_GRAPHICS_ATARI_SCREEN_H
#define BACKENDS_GRAPHICS_ATARI_SCREEN_H

#include <unordered_set>
#include <mint/ostruct.h>	// _RGB

#include "common/ptr.h"

#include "atari-cursor.h"
#include "atari-surface.h"

template<>
struct std::hash<Common::Rect>
{
	std::size_t operator()(Common::Rect const& rect) const noexcept
	{
		return 31 * (31 * (31 * rect.left + rect.top) + rect.right) + rect.bottom;
	}
};

class Palette {
public:
	void clear() {
		memset(_data, 0, sizeof(_data));
		entries = 0;
	}

	uint16 *const tt = reinterpret_cast<uint16*>(_data);
	_RGB *const falcon = reinterpret_cast<_RGB*>(_data);

	int entries = 0;

private:
	byte _data[256*4] = {};
};

struct Screen {
	using DirtyRects = std::unordered_set<Common::Rect>;

	Screen(bool tt, int width, int height, const Graphics::PixelFormat &format, const Palette *palette);

	void reset(int width, int height, const Graphics::Surface &boundingSurf);
	// must be called before any rectangle drawing
	void addDirtyRect(const Graphics::Surface &srcSurface, int x, int y, int w, int h, bool directRendering);

	void clearDirtyRects() {
		dirtyRects.clear();
		fullRedraw = false;
	}

	Common::ScopedPtr<AtariSurface> surf;
	const Palette *palette;
	DirtyRects dirtyRects;
	bool fullRedraw = false;

	Cursor cursor;

	int rez = -1;
	int mode = -1;
	const Common::ScopedPtr<AtariSurface> &offsettedSurf = _offsettedSurf;

private:
	static constexpr size_t ALIGN = 16;	// 16 bytes

	enum SteTtRezValue {
		kRezValueSTLow  = 0,	// 320x200@4bpp, ST palette
		kRezValueSTMid  = 1,	// 640x200@2bpp, ST palette
		kRezValueSTHigh = 2,	// 640x400@1bpp, ST palette
		kRezValueTTLow  = 7,	// 320x480@8bpp, TT palette
		kRezValueTTMid  = 4,	// 640x480@4bpp, TT palette
		kRezValueTTHigh = 6		// 1280x960@1bpp, TT palette
	};

	bool _tt;
	Common::ScopedPtr<AtariSurface> _offsettedSurf;
};

#endif // BACKENDS_GRAPHICS_ATARI_SCREEN_H
