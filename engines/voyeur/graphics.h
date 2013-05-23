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

#ifndef VOYEUR_GRAPHICS_H
#define VOYEUR_GRAPHICS_H

#include "common/scummsys.h"
#include "common/array.h"
#include "graphics/surface.h"

namespace Voyeur {

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define PALETTE_COUNT 256
#define PALETTE_SIZE (256 * 3)

typedef void (*IntFuncPtr)();

class IntNode {
public:
	IntNode *_nextNode;
	IntFuncPtr _intFunc;
	uint32 _curTime;
	uint32 _timeReset;
	uint32 _flags;
public:
	IntNode();
};

class GraphicsManager {
private:
	static void fadeIntFunc();
	static void vDoFadeInt();
	static void vDoCycleInt();
	void addIntNode(IntNode *node);
public:
	bool _palFlag;
	IntNode _fadeIntNode;
	IntNode _cycleIntNode;
	IntNode _evintnode;
	IntNode _mainintnode;
	byte _VGAColors[PALETTE_SIZE];
	Common::Array<byte *> _colorChain;
public:
	GraphicsManager();
	void sInitGraphics();

	void addFadeInt();
	void vInitColor();
};

} // End of namespace Voyeur

#endif /* VOYEUR_GRAPHICS_H */
