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

#ifndef GRAPHICS_MFC_GFX_SURFACE_H
#define GRAPHICS_MFC_GFX_SURFACE_H

#include "graphics/managed_surface.h"
#include "graphics/mfc/atltypes.h"

namespace Graphics {
namespace MFC {
namespace Gfx {

class Surface;

/**
 * Surface class
 */
class Surface {
public:
	class YIterator;

	/**
	 * Column iterator
	 */
	class XIterator {
	private:
		Surface *_surface;
		YIterator *_rowIter;
		int _x = 0;
		int _xMin, _xMax;
		byte *_pixelP = nullptr;
		byte _dummyPixel = 0;

	public:
		XIterator(YIterator *rowIter);

		XIterator &operator=(int x);
		XIterator &operator++();
		bool operator<(int xEnd) const;
		operator byte *();
	};

	/**
	 * Row iterator
	 */
	class YIterator {
		friend class XIterator;
	private:
		Surface *_surface;
		int _y = 0;
		int _yMin, _yMax;

	public:
		YIterator(Surface *surface);

		YIterator &operator=(int y);
		YIterator &operator++();
		bool operator<(int yMax) const;
	};

private:
	Graphics::ManagedSurface _surface;
	Common::Rect _clipRect;
	CPoint _viewportOrg;

public:
	Graphics::PixelFormat &format = _surface.format;
	int16 &w = _surface.w;
	int16 &h = _surface.h;
	int32 &pitch = _surface.pitch;

public:
	void create(int16 width, int16 height);
	void create(int16 width, int16 height, const Graphics::PixelFormat &pixelFormat);
	void create(Graphics::ManagedSurface &surf, const Common::Rect &bounds);

	void setClipRect(const Common::Rect &r);
	Common::Rect getClipRect() const;
	void resetClip();
	int intersectClipRect(const Common::Rect &r);
	int offsetClipRect(int x, int y);

	CPoint getViewportOrg() const;
	void setViewportOrg(const CPoint &pt);
	void offsetViewportOrg(int x, int y);

	byte *getBasePtr(int x, int y);
	void *getPixels() {
		return _surface.getPixels();
	}
	const void *getPixels() const {
		return _surface.getPixels();
	}
	void addDirtyRect(const Common::Rect &r);
	void setPalette(const byte *colors, int start, int num) {
		_surface.setPalette(colors, start, num);
	}
	void fillRect(const Common::Rect &r, uint color);
	void frameRect(const Common::Rect &r, uint color);
	void drawEllipse(int x0, int y0, int x1, int y1, uint32 color, bool filled);
	void drawLine(int x0, int y0, int x1, int y1, uint32 color);
	void hLine(int x0, int y, int x1, uint32 color);
	uint32 getPixel(int x, int y);
	Graphics::ManagedSurface getSubArea(const Common::Rect &r);
};

} // namespace Gfx
} // namespace MFC
} // namespace Graphics

#endif
