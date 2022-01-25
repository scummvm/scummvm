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

#ifndef TRECISION_GRAPHICS_H
#define TRECISION_GRAPHICS_H

#include "common/rect.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

namespace Common {
class SeekableReadStream;
}


namespace Trecision {
class TrecisionEngine;

struct Font {
	int8 *_data;
	uint16 _width;
};

class GraphicsManager {
	TrecisionEngine *_vm;

	Graphics::Surface _screenBuffer;
	Graphics::Surface _background;
	Graphics::Surface _smkBackground;
	Graphics::Surface _leftInventoryArrow;
	Graphics::Surface _rightInventoryArrow;
	Graphics::Surface _inventoryIcons;
	Graphics::Surface _saveSlotThumbnails;
	Graphics::Surface _textureMat;

	Graphics::PixelFormat _screenFormat;
	uint16 _bitMask[3];
	Font _fonts[256];

	Common::List<Common::Rect> _dirtyRects;

	const Graphics::PixelFormat _rgb555Format;

	uint16 aliasing(uint32 val1, uint32 val2, uint8 num);
	void drawCharPixel(uint16 y, uint16 charLeft, uint16 charRight, Common::Rect rect, Common::Rect subtitleRect, uint16 color, Graphics::Surface *externalSurface = nullptr);
	void initCursor();
	void copyToScreenBufferInner(const Graphics::Surface *surface, int x, int y);
	void paintObjAnm(uint16 curBox);
	void drawObj(int index, bool mask, Common::Rect drawRect, Common::Rect drawObjRect, bool includeDirtyRect = true);
	void eraseObj(Common::Rect drawObjRect);

public:
	GraphicsManager(TrecisionEngine *vm);
	~GraphicsManager();

	bool init();
	void clearScreen();
	void copyToScreen(int x, int y, int w, int h);
	void copyToScreenBuffer(const Graphics::Surface *surface, int x, int y, const byte *palette);
	void blitToScreenBuffer(const Graphics::Surface *surface, int x, int y, const byte *palette, bool useSmkBg);
	void paintScreen(bool flag);
	void loadBackground(Common::SeekableReadStream *stream);
	void clearScreenBuffer();
	void clearScreenBufferTop();
	void clearScreenBufferInventory();
	void clearScreenBufferSaveSlotDescriptions();
	void drawLeftInventoryArrow(byte startLine);
	void drawRightInventoryArrow(byte startLine);
	void drawInventoryIcon(byte iconIndex, byte iconSlot, byte startLine);
	void drawSaveSlotThumbnail(byte iconIndex, byte iconSlot, byte startLine);
	void setSaveSlotThumbnail(byte iconSlot, const Graphics::Surface *thumbnail);
	void readSurface(Common::SeekableReadStream *stream, Graphics::Surface *surface, uint16 width, uint16 height, uint16 count = 1);
	void readTexture(Common::SeekableReadStream *stream);
	void drawTexturePixel(uint16 textureX, uint16 textureY, uint16 screenX, uint16 screenY);

	uint16 convertToScreenFormat(uint16 color) const;

	void shadow(uint16 x, uint16 y, uint8 num);
	void pixelAliasing(uint16 x, uint16 y);
	void dissolve();

	void addDirtyRect(Common::Rect rect, bool translateRect);

	uint16 getCharWidth(byte character);
	void drawChar(byte curChar, uint16 textColor, uint16 line, Common::Rect rect, Common::Rect subtitleRect, uint16 inc, Graphics::Surface *externalSurface);

	bool isCursorVisible();
	void showCursor();
	void hideCursor();

	void loadFont();
	void loadData();
	void showDemoPic();
	
};

} // End of namespace Trecision
#endif
