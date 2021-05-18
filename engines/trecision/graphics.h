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

#ifndef TRECISION_GRAPHICS_H
#define TRECISION_GRAPHICS_H

#include "graphics/pixelformat.h"
#include "graphics/surface.h"

namespace Common {
class SeekableReadStream;
}

namespace Trecision {
struct SDObj;
class TrecisionEngine;

class GraphicsManager {
	TrecisionEngine *_vm;

	Graphics::Surface _screenBuffer;
	Graphics::Surface _background;
	Graphics::Surface _smkBackground;
	Graphics::Surface _leftInventoryArrow;
	Graphics::Surface _rightInventoryArrow;
	Graphics::Surface _inventoryIcons;
	Graphics::Surface _saveSlotThumbnails;

	Graphics::PixelFormat _screenFormat;
	uint16 _bitMask[3];
	uint8 *_font;

	static const Graphics::PixelFormat kImageFormat;

	uint16 aliasing(uint32 val1, uint32 val2, uint8 num);
	void drawCharPixel(uint16 y, uint16 charLeft, uint16 charRight, Common::Rect rect, Common::Rect subtitleRect, uint16 color, Graphics::Surface *externalSurface = nullptr);
	void initCursor();

public:
	GraphicsManager(TrecisionEngine *vm);
	~GraphicsManager();

	bool init();
	void clearScreen();
	void copyToScreen(int x, int y, int w, int h);
	void copyToScreenBuffer(Graphics::Surface *surface, int x, int y);
	void blitToScreenBuffer(Graphics::Surface *surface, int x, int y, uint16 mask, bool useSmkBg);
	uint16 *getScreenBufferPtr();
	void loadBackground(Common::SeekableReadStream *stream, uint16 width, uint16 height);
	void clearScreenBuffer();
	void clearScreenBufferTop();
	void clearScreenBufferInventory();
	void clearScreenBufferSaveSlotDescriptions();
	void drawLeftInventoryArrow(byte startLine);
	void drawRightInventoryArrow(byte startLine);
	void drawInventoryIcon(byte iconIndex, byte iconSlot, byte startLine);
	void drawSaveSlotThumbnail(byte iconIndex, byte iconSlot, byte startLine);
	void setSaveSlotThumbnail(byte iconSlot, Graphics::Surface *thumbnail);
	void readSurface(Common::SeekableReadStream *stream, Graphics::Surface *surface, uint16 width, uint16 height, uint16 count = 1);

	uint16 palTo16bit(uint8 r, uint8 g, uint8 b) const;
	void updatePixelFormat(uint16 *p, uint32 len) const;

	void shadow(uint16 x, uint16 y, uint8 num);
	void pixelAliasing(uint16 x, uint16 y);

	void dissolve(uint8 val);

	void drawObj(SDObj d);
	void eraseObj(SDObj d);

	uint16 getCharWidth(byte character);
	void drawChar(byte curChar, uint16 sColor, uint16 tColor, uint16 line, Common::Rect rect, Common::Rect subtitleRect, uint16 inc, Graphics::Surface *externalSurface);

	bool isCursorVisible();
	void showCursor();
	void hideCursor();

	void loadData();
	void showDemoPic();
};

} // End of namespace Trecision
#endif
