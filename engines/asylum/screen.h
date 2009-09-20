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

#ifndef ASYLUM_SCREEN_H_
#define ASYLUM_SCREEN_H_

#include "asylum/asylum.h"
#include "asylum/graphics.h"
#include "asylum/respack.h"

#include "common/system.h"  // for OSystem
#include "graphics/surface.h"

namespace Asylum {

typedef struct GraphicQueueItem {
	uint32 resId;
	uint32 frameIdx;
	uint32 x;
	uint32 y;
	uint32 flags;
	uint32 transTableNum;
    uint32 priority;

} GraphicQueueItem;

class Screen {
public:
	Screen(AsylumEngine *_vm);
	~Screen();

	void copyToBackBuffer(byte *buffer, int pitch, int x, int y, int width, int height);
	void copyToBackBufferWithTransparency(byte *buffer, int pitch, int x, int y, int width, int height);
	void copyBackBufferToScreen();
	void copyRectToScreen(byte *buffer, int pitch, int x, int y, int width, int height);
	void copyRectToScreenWithTransparency(byte *buffer, int pitch, int x, int y, int width, int height);
	void setPalette(byte *rgbPalette);
	void setPalette(ResourcePack *resPack, int entry) { setPalette(resPack->getResource(entry)->data + 32); }
    void drawWideScreen(int16 barSize);
    void clearScreen();

    void addGraphicToQueue(uint32 resId, uint32 frameIdx, uint32 x, uint32 y, uint32 flags, uint32 transTableNum, uint32 priority);
    void addCrossFadeGraphicToQueue(uint32 resId, uint32 frameIdx, uint32 x, uint32 y, uint32 redId2, uint32 x2, uint32 y2, uint32 flags, uint32 priority);
    void addGraphicToQueue(GraphicQueueItem item);
    void drawGraphicsInQueue();
    void clearGraphicsInQueue();
    void graphicsSelectionSort();
    void swapGraphicItem(int item1, int item2);
    void deleteGraphicFromQueue(uint32 resId);

private:
	Graphics::Surface _backBuffer;
	AsylumEngine *_vm;

    Common::Array<GraphicQueueItem> _queueItems;
};

} // end of namespace Asylum

#endif
