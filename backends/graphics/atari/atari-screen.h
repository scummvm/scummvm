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
#include <mint/ostruct.h>

#include "common/rect.h"
#include "graphics/surface.h"

template<>
struct std::hash<Common::Rect>
{
	std::size_t operator()(Common::Rect const& rect) const noexcept
	{
		return 31 * (31 * (31 * rect.left + rect.top) + rect.right) + rect.bottom;
	}
};

class AtariGraphicsManager;

class Palette {
public:
	void clear() {
		memset(data, 0, sizeof(data));
	}

	uint16 *const tt = reinterpret_cast<uint16*>(data);
	_RGB *const falcon = reinterpret_cast<_RGB*>(data);

private:
	byte data[256*4] = {};
};

struct Screen {
	using DirtyRects = std::unordered_set<Common::Rect>;

	Screen(AtariGraphicsManager *manager, int width, int height, const Graphics::PixelFormat &format, const Palette *palette);
	~Screen();

	void reset(int width, int height, int bitsPerPixel);
	// must be called before any rectangle drawing
	void addDirtyRect(const Graphics::Surface &srcSurface, const Common::Rect &rect, bool directRendering);

	void clearDirtyRects() {
		dirtyRects.clear();
		fullRedraw = false;
	}

	void storeBackground(const Common::Rect &rect);
	void restoreBackground(const Common::Rect &rect);

	Graphics::Surface surf;
	const Palette *palette;
	bool cursorPositionChanged = true;
	bool cursorSurfaceChanged = true;
	bool cursorVisibilityChanged = false;
	DirtyRects dirtyRects;
	bool fullRedraw = false;
	Common::Rect oldCursorRect;
	int rez = -1;
	int mode = -1;
	Graphics::Surface *const offsettedSurf = &_offsettedSurf;

	int oldScreenSurfaceWidth = -1;
	int oldScreenSurfaceHeight = -1;
	int oldScreenSurfacePitch = -1;
	int oldOffsettedSurfaceWidth = -1;
	int oldOffsettedSurfaceHeight = -1;

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

	const AtariGraphicsManager *_manager;

	Graphics::Surface _offsettedSurf;
	// used by direct rendering
	Graphics::Surface _cursorBackgroundSurf;
};

#endif // BACKENDS_GRAPHICS_ATARI_SCREEN_H
