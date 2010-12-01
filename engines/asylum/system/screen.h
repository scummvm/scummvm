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

typedef struct GraphicQueueItem {
	ResourceId resourceId;
	uint32 frameIndex;
	Common::Point point;
	int32 flags;
	int32 transTableNum;
	int32 priority;

	GraphicQueueItem() {
		resourceId = kResourceNone;
		frameIndex = 0;
		flags = 0;
		transTableNum = 0;
		priority = 0;
	}
} GraphicQueueItem;

class Screen {
public:
	Screen(AsylumEngine *_vm);
	~Screen();

	// Drawing
	void draw(ResourceId resourceId, uint32 frameIndex, int32 x, int32 y, int32 flags, bool colorKey = true);
	void draw(ResourceId resourceId, uint32 frameIndex, int32 x, int32 y, int32 flags, int32 transTableNum);
	void draw(ResourceId resourceId, uint32 frameIndex, int32 x, int32 y, int32 flags, ResourceId resourceId2, int32 destX, int32 destY, bool colorKey = true);

	void clear() const;

	// Palette
	void setPalette(byte *rgbPalette) const;
	void setPalette(ResourceId id);
	void paletteFade(uint32 red, int32 milliseconds, int32 param);
	void startPaletteFade(ResourceId resourceId, int32 milliseconds, int32 param);

	// Gamma
	void setGammaLevel(ResourceId id, int32 val);

	// Misc
	void drawWideScreenBars(int16 barSize) const;

	// Transparency tables
	void setupTransTable(ResourceId resourceId);
	void setupTransTables(uint32 count, ...);
	void selectTransTable(uint32 index);
	byte *getTransTableIndex() { return _transTableIndex; }

	// Graphic queue
	void addGraphicToQueue(ResourceId resourceId, uint32 frameIndex, Common::Point point, int32 flags, int32 transTableNum, int32 priority);
	void addGraphicToQueueCrossfade(ResourceId resourceId, uint32 frameIndex, Common::Point point, int32 objectResourceId, Common::Point objectPoint, int32 transTableNum);
	void addGraphicToQueueMasked(ResourceId resourceId, uint32 frameIndex, Common::Point point, int32 objectResourceId, Common::Point objectPoint, int32 flags, int32 priority);
	void addGraphicToQueue(GraphicQueueItem const &item);
	void drawGraphicsInQueue();
	void clearGraphicsInQueue();
	void graphicsSelectionSort();
	void swapGraphicItem(int32 item1, int32 item2);
	void deleteGraphicFromQueue(ResourceId resourceId);

	// TODO Make those private
	void copyToBackBufferWithTransparency(byte *buffer, int32 pitch, int32 x, int32 y, int32 width, int32 height);
	void copyBackBufferToScreen();

	// Debug
	void copyToBackBufferClipped(Graphics::Surface *surface, int x, int y);

private:
	Graphics::Surface _backBuffer;
	AsylumEngine *_vm;

	Common::Array<GraphicQueueItem> _queueItems;

	// Transparency tables
	uint32 _transTableCount;
	byte *_transTableIndex;
	byte *_transTableData;
	byte *_transTableBuffer;
	void clearTransTables();

	// Screen copying
	void copyToBackBuffer(byte *buffer, int32 pitch, int32 x, int32 y, uint32 width, uint32 height);
};

} // end of namespace Asylum

#endif
