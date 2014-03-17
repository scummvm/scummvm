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

#ifndef ILLUSIONS_SPRITEDRAWQUEUE_H
#define ILLUSIONS_SPRITEDRAWQUEUE_H

#include "illusions/illusions.h"
#include "illusions/graphics.h"
#include "common/list.h"
#include "common/rect.h"
#include "graphics/surface.h"

namespace Illusions {

class Screen;

struct SpriteDrawQueueItem {
	byte *_drawFlags;
	int16 _kind;
	int16 _scale;
	uint16 _flags;
	//field_A dw
	int _priority;
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
		Common::Point &drawPosition, Common::Point &controlPosition, int priority, int16 scale, uint16 flags);
	void insertSurface(Graphics::Surface *surface, WidthHeight &dimensions,
		Common::Point &drawPosition, int priority);
	void insertTextSurface(Graphics::Surface *surface, WidthHeight &dimensions,
		Common::Point &drawPosition, int priority);
protected:
	typedef Common::List<SpriteDrawQueueItem*> SpriteDrawQueueList;
	typedef SpriteDrawQueueList::iterator SpriteDrawQueueListIterator;

	struct FindInsertionPosition : public Common::UnaryFunction<const SpriteDrawQueueItem*, bool> {
		int _priority;
		FindInsertionPosition(int priority) : _priority(priority) {}
		bool operator()(const SpriteDrawQueueItem *item) const {
			return item->_priority >= _priority;
		}
	};

	Screen *_screen;
	SpriteDrawQueueList _queue;	
	void insert(SpriteDrawQueueItem *item, int priority);
	bool calcItemRect(SpriteDrawQueueItem *item, Common::Rect &srcRect, Common::Rect &dstRect);
};

} // End of namespace Illusions

#endif // ILLUSIONS_SPRITEDRAWQUEUE_H
