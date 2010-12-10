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
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_SCREEN_H
#define ASYLUM_SCREEN_H

#include "asylum/shared.h"

#include "common/array.h"

#include "graphics/surface.h"

namespace Asylum {

class AsylumEngine;
class GraphicResource;
class ResourcePack;

struct GraphicFrame;

enum GraphicItemType {
	kGraphicItemNormal = 1,
	kGraphicItemMasked = 5
};

typedef struct GraphicQueueItem {
	int32 priority;

	GraphicItemType type;
	ResourceId resourceId;
	uint32 frameIndex;
	Common::Point source;
	ResourceId resourceIdDestination;
	Common::Point destination;
	int32 flags;
	int32 transTableNum;

	GraphicQueueItem() {
		priority = 0;

		type = kGraphicItemNormal;
		resourceId = kResourceNone;
		frameIndex = 0;
		resourceIdDestination = kResourceNone;
		flags = 0;
		transTableNum = 0;
	}
} GraphicQueueItem;

class Screen {
public:
	Screen(AsylumEngine *_vm);
	~Screen();

	// Drawing
	void draw(ResourceId resourceId);
	void draw(ResourceId resourceId, uint32 frameIndex, int32 x, int32 y, int32 flags, bool colorKey = true);
	void draw(ResourceId resourceId, uint32 frameIndex, int32 x, int32 y, int32 flags, int32 transTableNum);
	void draw(ResourceId resourceId, uint32 frameIndex, int32 x, int32 y, int32 flags, ResourceId resourceId2, int32 destX, int32 destY, bool colorKey = true);

	// Misc
	void clear();
	void drawWideScreenBars(int16 barSize) const;
	void fillRect(int32 x, int32 y, int32 x2, int32 y2, int32 color);
	void copyBackBufferToScreen();
	void setFlag(int32 val) { _flag = (val < -1) ? -1 : val; }
	void takeScreenshot();

	// Palette
	void setPalette(byte *rgbPalette) const;
	void setPalette(ResourceId id);
	void setPaletteGamma(ResourceId id);
	void setupPaletteAndStartFade(uint32 red, int32 milliseconds, int32 param);
	void stopFadeAndSetPalette(ResourceId id, int32 milliseconds, int32 param);
	void paletteFade(uint32 red, int32 milliseconds, int32 param);
	void startPaletteFade(ResourceId resourceId, int32 milliseconds, int32 param);
	void updatePalette();
	void updatePalette(int32 param);
	void makeGreyPalette();
	void setupPalette(byte *buffer, int start, int count);

	// Gamma
	void setGammaLevel(ResourceId id, int32 val);

	// Transparency tables
	void setupTransTable(ResourceId resourceId);
	void setupTransTables(uint32 count, ...);
	void selectTransTable(uint32 index);
	byte *getTransTableIndex() { return _transTableIndex; }

	// Graphic queue
	void addGraphicToQueue(ResourceId resourceId, uint32 frameIndex, Common::Point point, int32 flags, int32 transTableNum, int32 priority);
	void addGraphicToQueueCrossfade(ResourceId resourceId, uint32 frameIndex, Common::Point source, int32 objectResourceId, Common::Point destination, int32 transTableNum);
	void addGraphicToQueueMasked(ResourceId resourceId, uint32 frameIndex, Common::Point point, int32 objectResourceId, Common::Point destination, int32 flags, int32 priority);
	void addGraphicToQueue(GraphicQueueItem const &item);
	void drawGraphicsInQueue();
	void clearGraphicsInQueue();
	void deleteGraphicFromQueue(ResourceId resourceId);

	// Used by Video
	void copyToBackBuffer(byte *buffer, int32 pitch, int32 x, int32 y, uint32 width, uint32 height);

	// Debug
	void drawLine(int x0, int y0, int x1, int y1);
	void copyToBackBufferClipped(Graphics::Surface *surface, int x, int y);

private:
	AsylumEngine *_vm;

	Graphics::Surface _backBuffer;
	Common::Rect _clipRect;
	Common::Array<GraphicQueueItem> _queueItems;

	int16 _flag;
	bool _useColorKey;

	// Transparency tables
	uint32 _transTableCount;
	byte *_transTableIndex;
	byte *_transTableData;
	byte *_transTableBuffer;
	void clearTransTables();

	// Graphic queue
	void graphicsSelectionSort();
	void swapGraphicItem(int32 item1, int32 item2);

	// Misc
	void clip(Common::Rect *source, Common::Rect *destination, int32 flags);

	// Screen blitting
	void blit(GraphicFrame *frame, Common::Rect *source, Common::Rect *destination, int32 flags, bool useColorKey);
	void blt(Common::Rect *dest, GraphicFrame* frame, Common::Rect *source, int32 flags, bool useColorKey);
	void bltFast(int32 dX, int32 dY, GraphicFrame* frame, Common::Rect *source, bool useColorKey);

	void copyToBackBufferWithTransparency(byte *buffer, int32 pitch, int32 x, int32 y, int32 width, int32 height);
};

} // end of namespace Asylum

#endif
