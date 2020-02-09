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

#ifndef NEVERHOOD_GRAPHICS_H
#define NEVERHOOD_GRAPHICS_H

#include "common/array.h"
#include "common/file.h"
#include "graphics/surface.h"
#include "neverhood/neverhood.h"

namespace Neverhood {

struct NPoint {
	int16 x, y;
};

typedef Common::Array<NPoint> NPointArray;

struct NDimensions {
	int16 width, height;
};

struct NRect {
	int16 x1, y1, x2, y2;

    static NRect make(int16 x01, int16 y01, int16 x02, int16 y02) {
        NRect r;
        r.set(x01, y01, x02, y02);
		return r;
    }

	void set(int16 x01, int16 y01, int16 x02, int16 y02) {
		x1 = x01;
		y1 = y01;
		x2 = x02;
		y2 = y02;
	}

	bool contains(int16 x, int16 y) const {
		return x >= x1 && x <= x2 && y >= y1 && y <= y2;
	}

};

typedef Common::Array<NRect> NRectArray;

// TODO: Use Common::Rect
struct NDrawRect {
	int16 x, y, width, height;
	NDrawRect() : x(0), y(0), width(0), height(0) {}
	NDrawRect(int16 x0, int16 y0, int16 width0, int16 height0) : x(x0), y(y0), width(width0), height(height0) {}
	int16 x2() { return x + width; }
	int16 y2() { return y + height; }
	void set(int16 x0, int16 y0, int16 width0, int16 height0) {
		x = x0;
		y = y0;
		width = width0;
		height = height0;
	}
};

class AnimResource;
class SpriteResource;
class MouseCursorResource;

class BaseSurface {
public:
	BaseSurface(NeverhoodEngine *vm, int priority, int16 width, int16 height, Common::String name);
	virtual ~BaseSurface();
	virtual void draw();
	void clear();
	void drawSpriteResource(SpriteResource &spriteResource);
	void drawSpriteResourceEx(SpriteResource &spriteResource, bool flipX, bool flipY, int16 width, int16 height);
	void drawAnimResource(AnimResource &animResource, uint frameIndex, bool flipX, bool flipY, int16 width, int16 height);
	void drawMouseCursorResource(MouseCursorResource &mouseCursorResource, int frameNum);
	void copyFrom(Graphics::Surface *sourceSurface, int16 x, int16 y, NDrawRect &sourceRect);
	int getPriority() const { return _priority; }
	void setPriority(int priority) { _priority = priority; }
	NDrawRect& getDrawRect() { return _drawRect; }
	NDrawRect& getSysRect() { return _sysRect; }
	NRect& getClipRect() { return _clipRect; }
	void setClipRect(NRect clipRect) { _clipRect = clipRect; }
	void setClipRects(NRect *clipRects, uint clipRectsCount) { _clipRects = clipRects; _clipRectsCount = clipRectsCount; }
	void clearClipRects() { _clipRects = NULL; _clipRectsCount = 0; }
	bool getVisible() const { return _visible; }
	void setVisible(bool value) { _visible = value; }
	void setTransparent(bool value) { _transparent = value; }
	Graphics::Surface *getSurface() { return _surface; }
	const Common::String getName() const { return _name; }
protected:
	NeverhoodEngine *_vm;
	int _priority;
	bool _visible;
	Common::String _name;
	Graphics::Surface *_surface;
	NDrawRect _drawRect;
	NDrawRect _sysRect;
	NRect _clipRect;
	NRect *_clipRects;
	uint _clipRectsCount;
	bool _transparent;
	// Version changes each time the pixels are touched in any way
	byte _version;
};

class ShadowSurface : public BaseSurface {
public:
	ShadowSurface(NeverhoodEngine *vm, int priority, int16 width, int16 height, BaseSurface *shadowSurface);
	void draw() override;
protected:
	BaseSurface *_shadowSurface;
};

class FontSurface : public BaseSurface {
public:
	FontSurface(NeverhoodEngine *vm, NPointArray *tracking, uint charsPerRow, uint16 numRows, byte firstChar, uint16 charWidth, uint16 charHeight);
	FontSurface(NeverhoodEngine *vm, uint32 fileHash, uint charsPerRow, uint16 numRows, byte firstChar, uint16 charWidth, uint16 charHeight);
	~FontSurface() override;
	void drawChar(BaseSurface *destSurface, int16 x, int16 y, byte chr);
	void drawString(BaseSurface *destSurface, int16 x, int16 y, const byte *string, int stringLen = -1);
	int16 getStringWidth(const byte *string, int stringLen);
	uint16 getCharWidth() const { return _charWidth; }
	uint16 getCharHeight() const { return _charHeight; }
	static FontSurface *createFontSurface(NeverhoodEngine *vm, uint32 fileHash);
protected:
	uint _charsPerRow;
	uint16 _numRows;
	byte _firstChar;
	uint16 _charWidth;
	uint16 _charHeight;
	NPointArray *_tracking;
};

// Misc

void parseBitmapResource(const byte *sprite, bool *rle, NDimensions *dimensions, NPoint *position, const byte **palette, const byte **pixels);
void unpackSpriteRle(const byte *source, int width, int height, byte *dest, int destPitch, bool flipX, bool flipY, byte oldColor = 0, byte newColor = 0);
void unpackSpriteNormal(const byte *source, int width, int height, byte *dest, int destPitch, bool flipX, bool flipY);
int calcDistance(int16 x1, int16 y1, int16 x2, int16 y2);

} // End of namespace Neverhood

#endif /* NEVERHOOD_GRAPHICS_H */
