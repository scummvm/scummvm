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

#ifndef M4_GRAPHICS_H
#define M4_GRAPHICS_H

#include "common/rect.h"
#include "common/system.h"
#include "common/stream.h"
#include "graphics/surface.h"

#include "m4/globals.h"

namespace M4 {

#define MADS_SURFACE_WIDTH 320
#define MADS_SURFACE_HEIGHT 156
#define MADS_SCREEN_HEIGHT 200
#define MADS_Y_OFFSET ((MADS_SCREEN_HEIGHT - MADS_SURFACE_HEIGHT) / 2)

#define TRANSPARENT_COLOR_INDEX 0xFF

struct BGR8 {
	uint8 b, g, r;
};

struct RGB8 {
	uint8 r, g, b;
};

//later use ScummVM's Rect?
struct M4Rect {
	int32 x1, y1, x2, y2;
};

class M4Surface;

// RGBList
// Implements a list of RGB entries

class RGBList {
private:
	int _size;
	RGB8 *_data;
	byte *_palIndexes;
	bool _freeData;
public:
	RGBList(int numEntries = 256, RGB8 *srcData = NULL, bool freeData = true);
	~RGBList();

	RGB8 *data() { return _data; }
	byte *palIndexes() { return _palIndexes; }
	int size() { return _size; }
	RGB8 &operator[](int idx) { return _data[idx]; }
	void setRange(int start, int count, const RGB8 *src);
	RGBList *clone() const;
};

// M4Surface
// Class representing either a buffered surface or the physical screen.

class M4Sprite;

struct SpriteInfo {
	M4Sprite *sprite;
	int hotX, hotY;
	int width, height;
	int scaleX, scaleY;
	uint8 encoding;
	byte *inverseColorTable;
	RGB8 *palette;
};

class M4Surface : protected Graphics::Surface {
private:
	byte _color;
	bool _isScreen;
	RGBList *_rgbList;
	bool _ownsData;

	void rexLoadBackground(Common::SeekableReadStream *source, RGBList **palData = NULL);
	void madsLoadBackground(int roomNumber, RGBList **palData = NULL);
	void m4LoadBackground(Common::SeekableReadStream *source);
public:
	M4Surface(bool isScreen = false) {
		create(g_system->getWidth(), isScreen ? g_system->getHeight() : MADS_SURFACE_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
		_isScreen = isScreen;
		_rgbList = NULL;
		_ownsData = true;
	}
	M4Surface(int width_, int height_) {
		create(width_, height_, Graphics::PixelFormat::createFormatCLUT8());
		_isScreen = false;
		_rgbList = NULL;
		_ownsData = true;
	}
	M4Surface(int width_, int height_, byte *srcPixels, int pitch_) {
		format = Graphics::PixelFormat::createFormatCLUT8();
		w = width_;
		h = height_;
		pitch = pitch_;
		pixels = srcPixels;
		_rgbList = NULL;
		_ownsData = false;
	}

	virtual ~M4Surface();

	// loads a .COD file into the M4Surface
	// TODO: maybe move this to the rail system? check where it makes sense
	//	   The sprite drawing needs this, too, so should be more global.
	void loadCodesM4(Common::SeekableReadStream *source);
	void loadCodesMads(Common::SeekableReadStream *source);

	// loads the specified background
	void loadBackground(int sceneNumber, RGBList **palData = NULL);
	void loadBackgroundRiddle(const char *sceneName);
	void madsLoadInterface(int index, RGBList **palData = NULL);
	void madsLoadInterface(const Common::String &filename);

	void setColor(byte value) { _color = value; }
	inline byte getColor() const { return _color; }
	void vLine(int x, int y1, int y2);
	void hLine(int x1, int x2, int y);
	void vLineXor(int x, int y1, int y2);
	void hLineXor(int x1, int x2, int y);
	void drawLine(int x1, int y1, int x2, int y2, byte color);
	void frameRect(int x1, int y1, int x2, int y2);
	void fillRect(int x1, int y1, int x2, int y2);

	void drawSprite(int x, int y, SpriteInfo &info, const Common::Rect &clipRect);

	// Surface methods
	inline Common::Rect bounds() const { return Common::Rect(0, 0, width(), height()); }
	inline int width() const { return w; }
	inline int height() const { return h; }
	inline int getPitch() const { return pitch; }
	void setSize(int sizeX, int sizeY) { create(sizeX, sizeY, Graphics::PixelFormat::createFormatCLUT8()); }
	inline byte *getBasePtr() {
		return (byte *)pixels;
	}
	inline byte *getBasePtr(int x, int y) {
		return (byte *)Graphics::Surface::getBasePtr(x, y);
	}
	inline const byte *getBasePtr(int x, int y) const {
		return (const byte *)Graphics::Surface::getBasePtr(x, y);
	}
	void freeData();
	void clear();
	void reset();
	void frameRect(const Common::Rect &r, uint8 color);
	void fillRect(const Common::Rect &r, uint8 color);
	void copyFrom(M4Surface *src, const Common::Rect &srcBounds, int destX, int destY, int transparentColor = -1);
	void copyFrom(M4Surface *src, int destX, int destY, int depth, M4Surface *depthSurface,
			int scale, int transparentColor = -1);

	void update() {
		if (_isScreen) {
			g_system->copyRectToScreen((const byte *)pixels, pitch, 0, 0, w, h);
			g_system->updateScreen();
		}
	}

	// copyTo methods
	inline void copyTo(M4Surface *dest, int transparentColor = -1) {
		dest->copyFrom(this, Common::Rect(width(), height()), 0, 0, transparentColor);
	}
	inline void copyTo(M4Surface *dest, int x, int y, int transparentColor = -1) {
		dest->copyFrom(this, Common::Rect(width(), height()), x, y, transparentColor);
	}
	inline void copyTo(M4Surface *dest, const Common::Rect &srcBounds, int destX, int destY,
				int transparentColor = -1) {
		dest->copyFrom(this, srcBounds, destX, destY, transparentColor);
	}
	inline void copyTo(M4Surface *dest, int destX, int destY, int depth, M4Surface *depthsSurface, int scale,
				int transparentColor = -1) {
		dest->copyFrom(this, destX, destY, depth, depthsSurface, scale, transparentColor);
	}

	void scrollX(int xAmount);
	void scrollY(int yAmount);

	void translate(RGBList *list, bool isTransparent = false);
	M4Surface *flipHorizontal() const;
};

enum FadeType {FT_TO_GREY, FT_TO_COLOR, FT_TO_BLOCK};

class Palette {
private:
	MadsM4Engine *_vm;
	bool _colorsChanged;
	bool _fading_in_progress;
	byte _originalPalette[256 * 3];
	byte _fadedPalette[256 * 3];
	int _usageCount[256];

	void reset();
public:
	Palette(MadsM4Engine *vm);

	void setPalette(const byte *colors, uint start, uint num);
	void setPalette(const RGB8 *colors, uint start, uint num);
	void grabPalette(byte *colors, uint start, uint num);
	void grabPalette(RGB8 *colors, uint start, uint num) {
		grabPalette((byte *)colors, start, num);
	}
	void setEntry(uint index, uint8 r, uint8 g, uint8 b);
	uint8 palIndexFromRgb(byte r, byte g, byte b, RGB8 *paletteData = NULL);

	void fadeToGreen(int numSteps, uint delayAmount);
	void fadeFromGreen(int numSteps, uint delayAmount, bool fadeToBlack);
	void fadeIn(int numSteps, uint delayAmount, RGB8 *destPalette, int numColors);
	void fadeIn(int numSteps, uint delayAmount, RGBList *destPalette);
	static RGB8 *decodeMadsPalette(Common::SeekableReadStream *palStream,  int *numColors);
	int setMadsPalette(Common::SeekableReadStream *palStream, int indexStart = 0);
	void setMadsSystemPalette();

	// Methods used for reference counting color usage
	void resetColorCounts();
	void blockRange(int startIndex, int size);
	void addRange(RGBList *list);
	void deleteRange(RGBList *list);
	void deleteAllRanges();

	// Color indexes
	uint8 BLACK;
	uint8 BLUE;
	uint8 GREEN;
	uint8 CYAN;
	uint8 RED;
	uint8 VIOLET;
	uint8 BROWN;
	uint8 LIGHT_GRAY;
	uint8 DARK_GRAY;
	uint8 LIGHT_BLUE;
	uint8 LIGHT_GREEN;
	uint8 LIGHT_CYAN;
	uint8 LIGHT_RED;
	uint8 PINK;
	uint8 YELLOW;
	uint8 WHITE;
};

void decompressRle(byte *rleData, int rleSize, byte *celData, int w, int h);
void decompressRle(Common::SeekableReadStream &rleData, byte *celData, int w, int h);
int scaleValue(int value, int scale, int err);

} // End of namespace M4

#endif
