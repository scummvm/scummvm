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

#ifndef ILLUSIONS_SPRITEDECOMPRESSQUEUE_H
#define ILLUSIONS_SPRITEDECOMPRESSQUEUE_H

#include "illusions/illusions.h"
#include "illusions/graphics.h"
#include "common/list.h"
#include "graphics/surface.h"

namespace Illusions {

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
	SpriteDecompressQueue();
	~SpriteDecompressQueue();
	void insert(byte *drawFlags, uint32 flags, uint32 field8, WidthHeight &dimensions,
		byte *compressedPixels, Graphics::Surface *surface);
	void decompressAll();
protected:
	typedef Common::List<SpriteDecompressQueueItem*> SpriteDecompressQueueList;
	typedef SpriteDecompressQueueList::iterator SpriteDecompressQueueListIterator;
	
	SpriteDecompressQueueList _queue;
	
	void decompress(SpriteDecompressQueueItem *item);

};

} // End of namespace Illusions

#endif // ILLUSIONS_SPRITEDRAWQUEUE_H
