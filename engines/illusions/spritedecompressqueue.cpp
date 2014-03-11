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

#include "illusions/spritedecompressqueue.h"

namespace Illusions {

SpriteDecompressQueue::SpriteDecompressQueue() {
}

SpriteDecompressQueue::~SpriteDecompressQueue() {
}

void SpriteDecompressQueue::insert(byte *drawFlags, uint32 flags, uint32 field8, WidthHeight &dimensions,
	byte *compressedPixels, Graphics::Surface *surface) {
	SpriteDecompressQueueItem *item = new SpriteDecompressQueueItem();
	item->_drawFlags = drawFlags;
	*item->_drawFlags &= 1;
	item->_flags = flags;
	item->_dimensions = dimensions;
	item->_compressedPixels = compressedPixels;
	item->_field8 = field8;
	item->_surface = surface;
	_queue.push_back(item);
}

void SpriteDecompressQueue::decompressAll() {
	SpriteDecompressQueueListIterator it = _queue.begin();
	while (it != _queue.end()) {
		decompress(*it);
		delete *it;
		it = _queue.erase(it);
	}
}

void SpriteDecompressQueue::decompress(SpriteDecompressQueueItem *item) {
	byte *src = item->_compressedPixels;
	Graphics::Surface *dstSurface = item->_surface;
	int dstSize = item->_dimensions._height * item->_dimensions._width;
	int processedSize = 0;
	int xincr, x, xstart;
	int yincr, y;

	if (item->_flags & 1) {
		x = xstart = item->_dimensions._width - 1;
		xincr = -1;
	} else {
		x = xstart = 0;
		xincr = 1;
	}

	if (item->_flags & 2) {
		y = item->_dimensions._height - 1;
		yincr = -1;
	} else {
		y = 0;
		yincr = 1;
	}
	
	byte *dst = (byte*)dstSurface->getBasePtr(x, y);

	while (processedSize < dstSize) {
		int16 op = READ_LE_UINT16(src);
		src += 2;
		if (op & 0x8000) {
			int runCount = (op & 0x7FFF) + 1;
			uint16 runColor = READ_LE_UINT16(src);
			src += 2;
			while (runCount--) {
				WRITE_LE_UINT16(dst, runColor);
				x += xincr;
				if (x >= item->_dimensions._width || x < 0) {
					x = xstart;
					y += yincr;
					dst = (byte*)dstSurface->getBasePtr(x, y);
				} else {
					dst += 2 * xincr;
				}
			}
			processedSize += runCount;
		} else {
			int copyCount = op + 1;
			while (copyCount--) {
				uint16 color = READ_LE_UINT16(src);
				src += 2;
				WRITE_LE_UINT16(dst, color);
				x += xincr;
				if (x >= item->_dimensions._width || x < 0) {
					x = xstart;
					y += yincr;
					dst = (byte*)dstSurface->getBasePtr(x, y);
				} else {
					dst += 2 * xincr;
				}
			}
			processedSize += copyCount;
		}
	}

	*item->_drawFlags &= ~1;
 
}

} // End of namespace Illusions
