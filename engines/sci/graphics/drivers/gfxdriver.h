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


#ifndef SCI_GRAPHICS_DRIVERS_GFXDRIVER_H
#define SCI_GRAPHICS_DRIVERS_GFXDRIVER_H

#include "common/rendermode.h"
#include "common/rect.h"
#include "graphics/pixelformat.h"
#include "sci/detection.h"

namespace Graphics {
	class Cursor;
}

namespace Sci {

struct PaletteMod;

class GfxDriver {
public:
	enum DrawFlags : uint32 {
		kHiResMode		=	1 << 0,
		kMovieMode		=	1 << 1
	};

	GfxDriver(uint16 screenWidth, uint16 screenHeight, int numColors) : _screenW(screenWidth), _screenH(screenHeight), _numColors(numColors), _ready(false), _pixelSize(1) {}
	virtual ~GfxDriver() {}
	virtual void initScreen(const Graphics::PixelFormat *srcRGBFormat = nullptr) = 0; // srcRGBFormat: expect incoming data to have the specified rgb pixel format (used for Mac hicolor videos)
	virtual void setPalette(const byte *colors, uint start, uint num, bool update, const PaletteMod *palMods, const byte *palModMapping) = 0;
	virtual void copyRectToScreen(const byte *src, int srcX, int srcY, int pitch, int destX, int destY, int w, int h, const PaletteMod *palMods, const byte *palModMapping) = 0;
	virtual void replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) = 0;
	virtual void replaceMacCursor(const Graphics::Cursor *cursor) = 0;
	virtual Common::Point getMousePos() const;
	virtual void setMousePos(const Common::Point &pos) const;
	virtual void setShakePos(int shakeXOffset, int shakeYOffset) const;
	virtual void clearRect(const Common::Rect &r) const;
	virtual void copyCurrentBitmap(byte *dest, uint32 size) const = 0;
	virtual void copyCurrentPalette(byte *dest, int start, int num) const;
	virtual void drawTextFontGlyph(const byte *src, int pitch, int hiresDestX, int hiresDestY, int hiresW, int hiresH, int transpColor, const PaletteMod *palMods, const byte *palModMapping) = 0;
	virtual byte remapTextColor(byte color) const { return color; }
	virtual void setColorMap(const byte *colorMap) {}
	virtual Common::Point getRealCoords(Common::Point &pos) const { return pos; }
	virtual void setFlags(uint32 flags) {}
	virtual void clearFlags(uint32 flags) {}
	virtual bool supportsPalIntensity() const = 0;
	virtual bool supportsHiResGraphics() const = 0;
	virtual bool driverBasedTextRendering() const = 0;
	uint16 numColors() const { return _numColors; }
	byte pixelSize() const { return _pixelSize; }

protected:
	bool _ready;
	static bool checkDriver(const char *const *driverNames, int listSize);
	const uint16 _screenW;
	const uint16 _screenH;
	uint16 _numColors;
	byte _pixelSize;
};

} // End of namespace Sci

namespace SciGfxDriver {

Common::RenderMode getRenderMode();
Sci::GfxDriver *create(Common::RenderMode renderMode, int width, int height);

} // End of namespace SciGfxDriver

#endif // SCI_GRAPHICS_DRIVERS_GFXDRIVER_H
