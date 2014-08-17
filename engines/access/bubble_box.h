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

#ifndef ACCESS_BUBBLE_BOX_H
#define ACCESS_BUBBLE_BOX_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/stream.h"
#include "common/types.h"
#include "graphics/surface.h"
#include "access/data.h"

namespace Access {

class AccessEngine;

class Box: public Manager {
public:
	int _edgeSize;
	Common::Rect _bounds;
public:
	Box(AccessEngine *vm);

	void doBox(int item, int box);
};

class BubbleBox: public Box {
public:
	Common::String _bubbleTitle;
	const char *_bubblePtr;
	int _maxChars;
	Common::Array<Common::Rect> _bubbles;
public:
	BubbleBox(AccessEngine *vm);

	void load(Common::SeekableReadStream *stream);

	void clearBubbles();

	void placeBubble();
	void placeBubble1();

	void calcBubble();

	void printBubble();

	void drawBubble(int index);


};

} // End of namespace Access

#endif /* ACCESS_BUBBLE_BOX_H */
