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

#include "neverhood/graphics.h"
#include "neverhood/resource.h"
#include "neverhood/screen.h"

namespace Neverhood {

BaseSurface::BaseSurface(NeverhoodEngine *vm, int priority, int16 width, int16 height, Common::String name)
	: _vm(vm), _priority(priority), _visible(true), _transparent(true),
	_clipRects(NULL), _clipRectsCount(0), _version(0), _name(name) {

	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = width;
	_drawRect.height = height;
	_sysRect.x = 0;
	_sysRect.y = 0;
	_sysRect.width = (width + 3) & 0xFFFC; // align by 4 bytes
	_sysRect.height = height;
	_clipRect.x1 = 0;
	_clipRect.y1 = 0;
	_clipRect.x2 = 640;
	_clipRect.y2 = 480;
	_surface = new Graphics::Surface();
	_surface->create(_sysRect.width, _sysRect.height, Graphics::PixelFormat::createFormatCLUT8());
}

BaseSurface::~BaseSurface() {
	_surface->free();
	delete _surface;
}

void BaseSurface::draw() {
	if (_surface && _visible && _drawRect.width > 0 && _drawRect.height > 0) {
		if (_clipRects && _clipRectsCount) {
			_vm->_screen->drawSurfaceClipRects(_surface, _drawRect, _clipRects, _clipRectsCount, _transparent, _version);
		} else if (_sysRect.x == 0 && _sysRect.y == 0) {
			_vm->_screen->drawSurface2(_surface, _drawRect, _clipRect, _transparent, _version);
		} else {
			_vm->_screen->drawUnk(_surface, _drawRect, _sysRect, _clipRect, _transparent, _version);
		}
	}
}

void BaseSurface::clear() {
	_surface->fillRect(Common::Rect(0, 0, _surface->w, _surface->h), 0);
	++_version;
}

void BaseSurface::drawSpriteResource(SpriteResource &spriteResource) {
	if (spriteResource.getDimensions().width <= _drawRect.width &&
		spriteResource.getDimensions().height <= _drawRect.height) {
		clear();
		spriteResource.draw(_surface, false, false);
		++_version;
	}
}

void BaseSurface::drawSpriteResourceEx(SpriteResource &spriteResource, bool flipX, bool flipY, int16 width, int16 height) {
	if (spriteResource.getDimensions().width <= _sysRect.width &&
		spriteResource.getDimensions().height <= _sysRect.height) {
		if (width > 0 && width <= _sysRect.width)
			_drawRect.width = width;
		if (height > 0 && height <= _sysRect.height)
			_drawRect.height = height;
		if (_surface) {
			clear();
			spriteResource.draw(_surface, flipX, flipY);
			++_version;
		}
	}
}

void BaseSurface::drawAnimResource(AnimResource &animResource, uint frameIndex, bool flipX, bool flipY, int16 width, int16 height) {
	if (width > 0 && width <= _sysRect.width)
		_drawRect.width = width;
	if (height > 0 && height <= _sysRect.height)
		_drawRect.height = height;
	if (_surface) {
		clear();
		if (frameIndex < animResource.getFrameCount()) {
			animResource.draw(frameIndex, _surface, flipX, flipY);
			++_version;
		}
	}
}

void BaseSurface::drawMouseCursorResource(MouseCursorResource &mouseCursorResource, int frameNum) {
	if (frameNum < 3) {
		mouseCursorResource.draw(frameNum, _surface);
		++_version;
	}
}

void BaseSurface::copyFrom(Graphics::Surface *sourceSurface, int16 x, int16 y, NDrawRect &sourceRect) {
	// Copy a rectangle from sourceSurface, 0 is the transparent color
	// Clipping is performed against the right/bottom border since x, y will always be >= 0

	if (x + sourceRect.width > _surface->w)
		sourceRect.width = _surface->w - x - 1;

	if (y + sourceRect.height > _surface->h)
		sourceRect.height = _surface->h - y - 1;

	byte *source = (byte*)sourceSurface->getBasePtr(sourceRect.x, sourceRect.y);
	byte *dest = (byte*)_surface->getBasePtr(x, y);
	int height = sourceRect.height;
	while (height--) {
		for (int xc = 0; xc < sourceRect.width; xc++)
			if (source[xc] != 0)
				dest[xc] = source[xc];
		source += sourceSurface->pitch;
		dest += _surface->pitch;
	}
	++_version;
}

// ShadowSurface

ShadowSurface::ShadowSurface(NeverhoodEngine *vm, int priority, int16 width, int16 height, BaseSurface *shadowSurface)
	: BaseSurface(vm, priority, width, height, "shadow"), _shadowSurface(shadowSurface) {
	// Empty
}

void ShadowSurface::draw() {
	if (_surface && _visible && _drawRect.width > 0 && _drawRect.height > 0) {
		_vm->_screen->drawSurface2(_surface, _drawRect, _clipRect, _transparent, _version, _shadowSurface->getSurface());
	}
}

// FontSurface

FontSurface::FontSurface(NeverhoodEngine *vm, NPointArray *tracking, uint charsPerRow, uint16 numRows, byte firstChar, uint16 charWidth, uint16 charHeight)
	: BaseSurface(vm, 0, charWidth * charsPerRow, charHeight * numRows, "font"), _charsPerRow(charsPerRow), _numRows(numRows),
	_firstChar(firstChar), _charWidth(charWidth), _charHeight(charHeight), _tracking(NULL) {

	_tracking = new NPointArray();
	*_tracking = *tracking;

}

FontSurface::FontSurface(NeverhoodEngine *vm, uint32 fileHash, uint charsPerRow, uint16 numRows, byte firstChar, uint16 charWidth, uint16 charHeight)
	: BaseSurface(vm, 0, charWidth * charsPerRow, charHeight * numRows, "font"), _charsPerRow(charsPerRow), _numRows(numRows),
	_firstChar(firstChar), _charWidth(charWidth), _charHeight(charHeight), _tracking(NULL) {

	SpriteResource fontSpriteResource(_vm);
	fontSpriteResource.load(fileHash, true);
	drawSpriteResourceEx(fontSpriteResource, false, false, 0, 0);
}

FontSurface::~FontSurface() {
	delete _tracking;
}

void FontSurface::drawChar(BaseSurface *destSurface, int16 x, int16 y, byte chr) {
	NDrawRect sourceRect;
	chr -= _firstChar;
	sourceRect.x = (chr % _charsPerRow) * _charWidth;
	sourceRect.y = (chr / _charsPerRow) * _charHeight;
	sourceRect.width = _charWidth;
	sourceRect.height = _charHeight;
	destSurface->copyFrom(_surface, x, y, sourceRect);
}

void FontSurface::drawString(BaseSurface *destSurface, int16 x, int16 y, const byte *string, int stringLen) {

	if (stringLen < 0)
		stringLen = strlen((const char*)string);

	for (; stringLen > 0; --stringLen, ++string) {
		drawChar(destSurface, x, y, *string);
		x += _tracking ? (*_tracking)[*string - _firstChar].x : _charWidth;
	}

}

int16 FontSurface::getStringWidth(const byte *string, int stringLen) {
	return string ? stringLen * _charWidth : 0;
}

FontSurface *FontSurface::createFontSurface(NeverhoodEngine *vm, uint32 fileHash) {
	FontSurface *fontSurface;
	DataResource fontData(vm);
	SpriteResource fontSprite(vm);
	fontData.load(calcHash("asRecFont"));
	uint16 numRows = fontData.getPoint(calcHash("meNumRows")).x;
	uint16 firstChar = fontData.getPoint(calcHash("meFirstChar")).x;
	uint16 charWidth = fontData.getPoint(calcHash("meCharWidth")).x;
	uint16 charHeight = fontData.getPoint(calcHash("meCharHeight")).x;
	NPointArray *tracking = fontData.getPointArray(calcHash("meTracking"));
	fontSprite.load(fileHash, true);
	fontSurface = new FontSurface(vm, tracking, 16, numRows, firstChar, charWidth, charHeight);
	fontSurface->drawSpriteResourceEx(fontSprite, false, false, 0, 0);
	return fontSurface;
}

// Misc

enum BitmapFlags {
	BF_RLE				= 1,
	BF_HAS_DIMENSIONS	= 2,
	BF_HAS_POSITION		= 4,
	BF_HAS_PALETTE		= 8,
	BF_HAS_IMAGE		= 16
};

void parseBitmapResource(const byte *sprite, bool *rle, NDimensions *dimensions, NPoint *position, const byte **palette, const byte **pixels) {

	uint16 flags;

	flags = READ_LE_UINT16(sprite);
	sprite += 2;

	if (rle)
		*rle = flags & BF_RLE;

	if (flags & BF_HAS_DIMENSIONS) {
		if (dimensions) {
			dimensions->width = READ_LE_UINT16(sprite);
			dimensions->height = READ_LE_UINT16(sprite + 2);
		}
		sprite += 4;
	} else if (dimensions) {
		dimensions->width = 1;
		dimensions->height = 1;
	}

	if (flags & BF_HAS_POSITION) {
		if (position) {
			position->x = READ_LE_UINT16(sprite);
			position->y = READ_LE_UINT16(sprite + 2);
		}
		sprite += 4;
	} else if (position) {
		position->x = 0;
		position->y = 0;
	}

	if (flags & BF_HAS_PALETTE) {
		if (palette)
			*palette = sprite;
		sprite += 1024;
	} else if (palette)
		*palette = NULL;

	if (flags & BF_HAS_IMAGE) {
		if (pixels)
			*pixels = sprite;
	} else if (pixels)
		*pixels = NULL;

}

void unpackSpriteRle(const byte *source, int width, int height, byte *dest, int destPitch, bool flipX, bool flipY, byte oldColor, byte newColor) {

	const bool replaceColors = oldColor != newColor;

	int16 rows, chunks;
	int16 skip, copy;

	if (flipY) {
		dest += destPitch * (height - 1);
		destPitch = -destPitch;
	}

	rows = READ_LE_UINT16(source);
	chunks = READ_LE_UINT16(source + 2);
	source += 4;

	do {
		if (chunks == 0) {
			dest += rows * destPitch;
		} else {
			while (rows-- > 0) {
				uint16 rowChunks = chunks;
				while (rowChunks-- > 0) {
					skip = READ_LE_UINT16(source);
					copy = READ_LE_UINT16(source + 2);
					source += 4;
					if (!flipX) {
						memcpy(dest + skip, source, copy);
					} else {
						byte *flipDest = dest + width - skip - 1;
						for (int xc = 0; xc < copy; xc++) {
							*flipDest-- = source[xc];
						}
					}
					source += copy;
				}
				if (replaceColors)
					for (int xc = 0; xc < width; xc++)
						if (dest[xc] == oldColor)
							dest[xc] = newColor;
				dest += destPitch;
			}
		}
		rows = READ_LE_UINT16(source);
		chunks = READ_LE_UINT16(source + 2);
		source += 4;
	} while (rows > 0);

}

void unpackSpriteNormal(const byte *source, int width, int height, byte *dest, int destPitch, bool flipX, bool flipY) {

	const int sourcePitch = (width + 3) & 0xFFFC;

	if (flipY) {
		dest += destPitch * (height - 1);
		destPitch = -destPitch;
	}

	if (!flipX) {
		while (height-- > 0) {
			memcpy(dest, source, width);
			source += sourcePitch;
			dest += destPitch;
		}
	} else {
		while (height-- > 0) {
			dest += width - 1;
			for (int xc = 0; xc < width; xc++)
				*dest-- = source[xc];
			source += sourcePitch;
			dest += destPitch;
		}
	}

}

int calcDistance(int16 x1, int16 y1, int16 x2, int16 y2) {
	const int16 deltaX = ABS(x1 - x2);
	const int16 deltaY = ABS(y1 - y2);
	return (int)sqrt((double)(deltaX * deltaX + deltaY * deltaY));
}

} // End of namespace Neverhood
