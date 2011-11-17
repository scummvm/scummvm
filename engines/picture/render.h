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
 * $URL: https://www.switchlink.se/svn/picture/screen.h $
 * $Id: screen.h 34 2008-09-17 11:01:43Z johndoe $
 *
 */

#ifndef PICTURE_RENDER_H
#define PICTURE_RENDER_H

#include "graphics/surface.h"

#include "picture/segmap.h"
#include "picture/screen.h"
#include "picture/microtiles.h"

namespace Picture {

enum RenderType {
	kSprite,
	kText,
	kMask
};

enum RenderFlags {
	kNone			= 1 << 0,
	kRefresh		= 1 << 1,
	kRemoved		= 1 << 2,
	kMoved			= 1 << 3,
	kUnchanged		= 1 << 4
};

struct RenderTextItem {
	byte color;
	uint fontResIndex;
	byte *text;
	int len;
};

struct RenderQueueItem {
	RenderType type;
	uint flags;
	Common::Rect rect;
	int16 priority;
	union {
		SpriteDrawItem sprite;
		RenderTextItem text;
		SegmapMaskRect mask;
	};
};

class RenderQueue {
public:
	RenderQueue(PictureEngine *vm);
	~RenderQueue();

	void addSprite(SpriteDrawItem &sprite);
	void addText(int16 x, int16 y, byte color, uint fontResIndex, byte *text, int len);
	void addMask(SegmapMaskRect &mask);
	void update();
	void clear();
	
protected:
	typedef Common::List<RenderQueueItem> RenderQueueArray;

	PictureEngine *_vm;
	RenderQueueArray *_currQueue, *_prevQueue;
	MicroTileArray *_updateUta;

	bool rectIntersectsItem(const Common::Rect &rect);
    RenderQueueItem *findItemInQueue(RenderQueueArray *queue, const RenderQueueItem &item);
    bool hasItemChanged(const RenderQueueItem &item1, const RenderQueueItem &item2);
    void invalidateItemsByRect(const Common::Rect &rect, const RenderQueueItem *item);
    
    void addDirtyRect(const Common::Rect &rect);
    void restoreDirtyBackground();
    void updateDirtyRects();

};

} // End of namespace Picture

#endif /* PICTURE_RENDER_H */
