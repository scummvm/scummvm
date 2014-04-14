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

#ifndef ILLUSIONS_SCREEN_H
#define ILLUSIONS_SCREEN_H

#include "illusions/graphics.h"
#include "common/list.h"
#include "common/rect.h"
#include "graphics/surface.h"

namespace Illusions {

class IllusionsEngine;
class FontResource;

struct SpriteDecompressQueueItem {
	byte *_drawFlags;
	uint32 _flags;
	uint32 _field8;
	WidthHeight _dimensions;
	byte *_compressedPixels;
	Graphics::Surface *_surface;
};

class SpriteDecompressQueue {
public:
	SpriteDecompressQueue(Screen *screen);
	~SpriteDecompressQueue();
	void insert(byte *drawFlags, uint32 flags, uint32 field8, WidthHeight &dimensions,
		byte *compressedPixels, Graphics::Surface *surface);
	void decompressAll();
protected:
	typedef Common::List<SpriteDecompressQueueItem*> SpriteDecompressQueueList;
	typedef SpriteDecompressQueueList::iterator SpriteDecompressQueueListIterator;
	Screen *_screen;
	SpriteDecompressQueueList _queue;
	void decompress(SpriteDecompressQueueItem *item);
};

struct SpriteDrawQueueItem {
	byte *_drawFlags;
	int16 _kind;
	int16 _scale;
	uint16 _flags;
	uint32 _priority;
	Graphics::Surface *_surface;
	WidthHeight _dimensions;
	Common::Point _drawPosition;
	Common::Point _controlPosition;
};

class SpriteDrawQueue {
public:
	SpriteDrawQueue(Screen *screen);
	~SpriteDrawQueue();
	bool draw(SpriteDrawQueueItem *item);
	void drawAll();
	void insertSprite(byte *drawFlags, Graphics::Surface *surface, WidthHeight &dimensions,
		Common::Point &drawPosition, Common::Point &controlPosition, uint32 priority, int16 scale, uint16 flags);
	void insertSurface(Graphics::Surface *surface, WidthHeight &dimensions,
		Common::Point &drawPosition, uint32 priority);
	void insertTextSurface(Graphics::Surface *surface, WidthHeight &dimensions,
		Common::Point &drawPosition, uint32 priority);
protected:
	typedef Common::List<SpriteDrawQueueItem*> SpriteDrawQueueList;
	typedef SpriteDrawQueueList::iterator SpriteDrawQueueListIterator;
	struct FindInsertionPosition : public Common::UnaryFunction<const SpriteDrawQueueItem*, bool> {
		uint32 _priority;
		FindInsertionPosition(uint32 priority) : _priority(priority) {}
		bool operator()(const SpriteDrawQueueItem *item) const {
			return item->_priority >= _priority;
		}
	};
	Screen *_screen;
	SpriteDrawQueueList _queue;	
	void insert(SpriteDrawQueueItem *item, uint32 priority);
	bool calcItemRect(SpriteDrawQueueItem *item, Common::Rect &srcRect, Common::Rect &dstRect);
};

// TODO Split into two classes (8bit and 16bit)?

class Screen {
public:
	Screen(IllusionsEngine *vm, int16 width, int16 height, int bpp);
	~Screen();
	Graphics::Surface *allocSurface(int16 width, int16 height);
	Graphics::Surface *allocSurface(SurfInfo &surfInfo);
	bool isDisplayOn();
	void setDisplayOn(bool isOn);
	uint16 getColorKey2();
	void updateSprites();
	void decompressSprite(SpriteDecompressQueueItem *item);
	void drawSurface(Common::Rect &dstRect, Graphics::Surface *surface, Common::Rect &srcRect, int16 scale, uint32 flags);
	void setPalette(byte *colors, uint start, uint count);
	void setPaletteEntry(int16 index, byte r, byte g, byte b);
	void getPalette(byte *colors);
	void shiftPalette(int16 fromIndex, int16 toIndex);
	void updatePalette();
	void drawText(FontResource *font, Graphics::Surface *surface, int16 x, int16 y, uint16 *text, uint count);
	int16 drawChar(FontResource *font, Graphics::Surface *surface, int16 x, int16 y, uint16 c);
	int16 getScreenWidth() const { return _backSurface->w; }
	int16 getScreenHeight() const { return _backSurface->h; }
public:
	IllusionsEngine *_vm;
	bool _displayOn;
	uint16 _colorKey1;
	uint16 _colorKey2;
	SpriteDecompressQueue *_decompressQueue;
	SpriteDrawQueue *_drawQueue;
	Graphics::Surface *_backSurface;
	
	bool _needRefreshPalette;
	byte _mainPalette[768];
	byte _colorTransTbl[256];
	
	void setSystemPalette(byte *palette);
	void buildColorTransTbl();

	void decompressSprite8(SpriteDecompressQueueItem *item);
	void drawSurface8(Common::Rect &dstRect, Graphics::Surface *surface, Common::Rect &srcRect, int16 scale, uint32 flags);
	void drawSurface81(int16 destX, int16 destY, Graphics::Surface *surface, Common::Rect &srcRect);
	void drawSurface82(Common::Rect &dstRect, Graphics::Surface *surface, Common::Rect &srcRect);

	void decompressSprite16(SpriteDecompressQueueItem *item);
	void drawSurface16(Common::Rect &dstRect, Graphics::Surface *surface, Common::Rect &srcRect, int16 scale, uint32 flags);
	void drawSurface10(int16 destX, int16 destY, Graphics::Surface *surface, Common::Rect &srcRect, uint16 colorKey);
	void drawSurface11(int16 destX, int16 destY, Graphics::Surface *surface, Common::Rect &srcRect);
	void drawSurface20(Common::Rect &dstRect, Graphics::Surface *surface, Common::Rect &srcRect, uint16 colorKey);
	void drawSurface21(Common::Rect &dstRect, Graphics::Surface *surface, Common::Rect &srcRect);
};

} // End of namespace Illusions

#endif // ILLUSIONS_SCREEN_H
