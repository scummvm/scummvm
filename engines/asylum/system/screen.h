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

#include "asylum/asylum.h"
#include "asylum/respack.h"
#include "asylum/system/graphics.h"

#include "common/system.h"  // for OSystem
#include "graphics/surface.h"

namespace Asylum {

typedef struct GraphicQueueItem {
	ResourceId resourceId;
	int32 frameIdx;
	int32 x;
	int32 y;
	int32 flags;
	int32 transTableNum;
	int32 priority;

} GraphicQueueItem;

class Screen {
public:
	Screen(AsylumEngine *_vm);
	~Screen();

	void copyToBackBuffer(byte *buffer, int32 pitch, int32 x, int32 y, int32 width, int32 height);
	void copyToBackBufferWithTransparency(byte *buffer, int32 pitch, int32 x, int32 y, int32 width, int32 height);
	void copyBackBufferToScreen();
	void copyRectToScreen(byte *buffer, int32 pitch, int32 x, int32 y, int32 width, int32 height);
	void copyRectToScreenWithTransparency(byte *buffer, int32 pitch, int32 x, int32 y, int32 width, int32 height);
	void setPalette(byte *rgbPalette);
	void setPalette(ResourcePack *resPack, ResourceId id) {
		setPalette(resPack->getResource(id)->data + 32);
	}

	void setGammaLevel(ResourcePack *resPack, ResourceId id, int32 val);

	void drawWideScreen(int16 barSize);
	void clearScreen();

	void paletteFade(uint32 red, int32 milliseconds, int32 param);
	void startPaletteFade(ResourceId resourceId, int32 milliseconds, int32 param);

	void addGraphicToQueue(ResourceId resourceId, int32 frameIdx, int32 x, int32 y, int32 flags, int32 transTableNum, int32 priority);
	void addCrossFadeGraphicToQueue(ResourceId resourceId, int32 frameIdx, int32 x, int32 y, int32 redId2, int32 x2, int32 y2, int32 flags, int32 priority);
	void addGraphicToQueue(GraphicQueueItem item);
	void drawGraphicsInQueue();
	void clearGraphicsInQueue();
	void graphicsSelectionSort();
	void swapGraphicItem(int32 item1, int32 item2);
	void deleteGraphicFromQueue(ResourceId resourceId);

private:
	Graphics::Surface _backBuffer;
	AsylumEngine *_vm;

	Common::Array<GraphicQueueItem> _queueItems;
};

} // end of namespace Asylum

#endif
