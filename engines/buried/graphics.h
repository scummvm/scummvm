/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BURIED_GRAPHICS_H
#define BURIED_GRAPHICS_H

#include "common/rect.h"
#include "common/scummsys.h"

namespace Common {
class SeekableReadStream;
}

namespace Graphics {
class Font;
struct Surface;
}

namespace Buried {

class BuriedEngine;

enum Cursor {
	kCursorNone            =     0,

	// Windows cursors
	kCursorArrow           = 32512,
	kCursorWait            = 32514,

	// Buried in Time Cursors
	kCursorEmptyArrow      =   100,
	kCursorFinger          =   101,
	kCursorMagnifyingGlass =   102,
	kCursorOpenHand        =   103,
	kCursorClosedHand      =   104,
	kCursorPutDown         =   105,
	kCursorNextPage        =   106,
	kCursorPrevPage        =   107,
	kCursorMoveUp          =   108,
	kCursorMoveDown        =   109,
	kCursorLocateA         =   110,
	kCursorLocateB         =   111,
	kCursorArrowUp         =   112,
	kCursorArrowLeft       =   113,
	kCursorArrowDown       =   114,
	kCursorArrowRight      =   115
};

enum TextAlign {
	kTextAlignLeft,
	kTextAlignCenter,
	kTextAlignRight
};

class GraphicsManager {
public:
	GraphicsManager(BuriedEngine *vm);
	~GraphicsManager();

	byte *getDefaultPalette() const { return _palette; }
	Graphics::Font *createFont(int size, bool bold = false) const;
	Cursor setCursor(Cursor newCursor);
	void toggleCursor(bool show);
	Graphics::Surface *getBitmap(uint32 bitmapID);
	Graphics::Surface *getBitmap(const Common::String &fileName, bool required = true);
	uint32 getColor(byte r, byte g, byte b);

	void invalidateRect(const Common::Rect &rect, bool erase = true);
	const Common::Rect &getDirtyRect() const { return _dirtyRect; }

	void markMouseMoved() { _mouseMoved = true; }
	void updateScreen(bool drawWindows = true);
	Graphics::Surface *getScreen() const { return _screen; }
	bool needsErase() const { return _needsErase; }

	void blit(const Graphics::Surface *surface, int x, int y);
	void blit(const Graphics::Surface *surface, int x, int y, uint width, uint height);
	void blit(const Graphics::Surface *surface, const Common::Rect &srcRect, const Common::Rect &dstRect);
	void fillRect(const Common::Rect &rect, uint32 color);
	void keyBlit(Graphics::Surface *dst, int xDst, int yDst, int w, int h, const Graphics::Surface *src, uint xSrc, uint ySrc, uint32 transColor);
	void keyBlit(Graphics::Surface *dst, int xDst, int yDst, int w, int h, const Graphics::Surface *src, uint xSrc, uint ySrc, byte rTrans, byte gTrans, byte bTrans);
	void opaqueTransparentBlit(Graphics::Surface *dst, int xDst, int yDst, int w, int h, const Graphics::Surface *src, uint xSrc, uint ySrc, int opacityValue, byte r, byte g, byte b);
	bool checkPointAgainstMaskedBitmap(const Graphics::Surface *bitmap, int x, int y, const Common::Point &point, byte rTrans, byte gTrans, byte bTrans);
	void crossBlit(Graphics::Surface *dst, int xDst, int yDst, uint w, uint h, const Graphics::Surface *src, uint xSrc, uint ySrc);
	void renderText(Graphics::Surface *dst, Graphics::Font *font, const Common::String &text, int x, int y, int w, int h, uint32 color, int lineHeight, TextAlign textAlign = kTextAlignLeft, bool centerVertically = false);
	void renderText(Graphics::Surface *dst, Graphics::Font *font, const Common::U32String &text, int x, int y, int w, int h, uint32 color, int lineHeight, TextAlign textAlign = kTextAlignLeft, bool centerVertically = false);
	void drawEllipse(const Common::Rect &rect, uint32 color);

	Graphics::Surface *remapPalettedFrame(const Graphics::Surface *frame, const byte *palette);

	int computeHPushOffset(int speed);
	int computeVPushOffset(int speed);

private:
	BuriedEngine *_vm;
	Cursor _curCursor;

	Common::Rect _dirtyRect;
	bool _mouseMoved;
	Graphics::Surface *_screen;
	byte *_palette;
	bool _needsErase;

	byte *createDefaultPalette() const;
	Graphics::Surface *getBitmap(Common::SeekableReadStream *stream);

	Graphics::Font *createArialFont(int size, bool bold) const;

	Graphics::Font *createMSGothicFont(int size, bool bold) const;
};

/**
 * RAII method of changing a cursor temporarily and reset
 * to the previous one on destruction.
 */
class TempCursorChange {
public:
	TempCursorChange(Cursor cursor);
	~TempCursorChange();

private:
	Cursor _prevCursor;
};

} // End of namespace Buried

#endif
