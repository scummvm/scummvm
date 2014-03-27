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

#include "illusions/spritedrawqueue.h"
#include "illusions/screen.h"

namespace Illusions {

SpriteDrawQueue::SpriteDrawQueue(Screen *screen)
	: _screen(screen) {
}

SpriteDrawQueue::~SpriteDrawQueue() {
}

bool SpriteDrawQueue::draw(SpriteDrawQueueItem *item) {

	// Check if the sprite has finished decompressing
	if (item->_kind != 0 && (*item->_drawFlags & 1)) {		
		insert(item, item->_priority);
		return false;
	}

	if (!_screen->isDisplayOn()) {
		if (item->_drawFlags)
			*item->_drawFlags &= ~4;
		return true;			
	}	

	Common::Rect srcRect, dstRect;
	
	// Check if the sprite is on-screen
	if (!calcItemRect(item, srcRect, dstRect))
		return true;

	if (item->_scale == 100) {
		if (item->_flags & 1)
			_screen->drawSurface10(dstRect.left, dstRect.top, item->_surface, srcRect, _screen->getColorKey2());
		else
			_screen->drawSurface11(dstRect.left, dstRect.top, item->_surface, srcRect);
	} else {
		if (item->_flags & 1)
			_screen->drawSurface20(dstRect, item->_surface, srcRect, _screen->getColorKey2());
		else
			_screen->drawSurface21(dstRect, item->_surface, srcRect);
	}
	
	if (item->_drawFlags)
		*item->_drawFlags &= ~4;

	return true;
}

void SpriteDrawQueue::drawAll() {
	SpriteDrawQueueListIterator it = _queue.begin();
	while (it != _queue.end()) {
		if (draw(*it)) {
			delete *it;
			it = _queue.erase(it);
		} else
			++it;
	}
}

void SpriteDrawQueue::insertSprite(byte *drawFlags, Graphics::Surface *surface, WidthHeight &dimensions,
	Common::Point &drawPosition, Common::Point &controlPosition, uint32 priority, int16 scale, uint16 flags) {
	SpriteDrawQueueItem *item = new SpriteDrawQueueItem();
	item->_drawFlags = drawFlags;
	*item->_drawFlags &= 4;
	item->_surface = surface;
	item->_dimensions = dimensions;
	item->_controlPosition = controlPosition;
	item->_scale = scale;
	item->_priority = priority;
	item->_drawPosition = drawPosition;
	item->_kind = 1;
	item->_flags = flags;
	insert(item, priority);
}

void SpriteDrawQueue::insertSurface(Graphics::Surface *surface, WidthHeight &dimensions,
	Common::Point &drawPosition, uint32 priority) {
	SpriteDrawQueueItem *item = new SpriteDrawQueueItem();
	item->_surface = surface;
	item->_dimensions = dimensions;
	item->_drawFlags = 0;
	item->_kind = 0;
	item->_drawPosition.x = -drawPosition.x;
	item->_drawPosition.y = -drawPosition.y;
	item->_controlPosition.x = 0;
	item->_controlPosition.y = 0;
	item->_flags = 0;
	item->_scale = 100;
	item->_priority = priority;// << 16;
	insert(item, priority);
}

void SpriteDrawQueue::insertTextSurface(Graphics::Surface *surface, WidthHeight &dimensions,
	Common::Point &drawPosition, uint32 priority) {
	SpriteDrawQueueItem *item = new SpriteDrawQueueItem();
	item->_surface = surface;
	item->_drawPosition = drawPosition;
	item->_dimensions = dimensions;
	item->_drawFlags = 0;
	item->_kind = 0;
	item->_controlPosition.x = 0;
	item->_controlPosition.y = 0;
	item->_flags = 0;
	item->_priority = priority;
	item->_scale = 100;
	insert(item, priority);
}

void SpriteDrawQueue::insert(SpriteDrawQueueItem *item, uint32 priority) {
	SpriteDrawQueueListIterator insertionPos = Common::find_if(_queue.begin(), _queue.end(),
		FindInsertionPosition(priority));
	_queue.insert(insertionPos, item);
}

bool SpriteDrawQueue::calcItemRect(SpriteDrawQueueItem *item, Common::Rect &srcRect, Common::Rect &dstRect) {

	srcRect.left = 0;
	srcRect.top = 0;
	srcRect.right = item->_dimensions._width;
	srcRect.bottom = item->_dimensions._height;
	
	dstRect.left = item->_drawPosition.x - item->_scale * item->_controlPosition.x / 100;
	dstRect.top = item->_drawPosition.y - item->_scale * item->_controlPosition.y / 100;
	dstRect.right = item->_drawPosition.x + item->_scale * (item->_dimensions._width - item->_controlPosition.x) / 100;
	dstRect.bottom = item->_drawPosition.y + item->_scale * (item->_dimensions._height - item->_controlPosition.y) / 100;
	
	/* CHECKME This seems to be unused basically and only called from debug code
		Left here just in case...
	if (gfx_seemsAlways0) {
		dstRect.left += screenOffsetPt.x;
		dstRect.right = screenOffsetPt.x + dstRect.right;
		dstRect.top = screenOffsetPt.y + dstRect.top;
		dstRect.bottom = screenOffsetPt.y + dstRect.bottom;
	}
	*/

	// Check if the sprite is on-screen
	if (dstRect.left >= 640 || dstRect.right <= 0 || dstRect.top >= 480 || dstRect.bottom <= 0)
		return false;

	// Clip the sprite rect if neccessary

	if (dstRect.left < 0) {
		srcRect.left += -100 * dstRect.left / item->_scale;
		dstRect.left = 0;
	}

	if (dstRect.top < 0) {
		srcRect.top += -100 * dstRect.top / item->_scale;
		dstRect.top = 0;
	}

	if (dstRect.right > 640) {
		srcRect.right += 100 * (640 - dstRect.right) / item->_scale;
		dstRect.right = 640;
	}

	if (dstRect.bottom > 480) {
		srcRect.bottom += 100 * (480 - dstRect.bottom) / item->_scale;
		dstRect.bottom = 480;
	}

	return true;
}

} // End of namespace Illusions
