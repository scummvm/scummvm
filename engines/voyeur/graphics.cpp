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

#include "voyeur/graphics.h"
#include "engines/util.h"
#include "graphics/surface.h"

namespace Voyeur {

GraphicsManager::GraphicsManager() {
	_palFlag = false;
}

void GraphicsManager::sInitGraphics() {
	initGraphics(SCREEN_WIDTH, SCREEN_HEIGHT, false);
}

void GraphicsManager::addFadeInt() {
	_fadeIntNode._intFunc = fadeIntFunc;
	_fadeIntNode._flags = 0;
	_fadeIntNode._curTime = 0;
	_fadeIntNode._timeReset = 1;
	
	addIntNode(&_fadeIntNode);
}

void GraphicsManager::vInitColor() {
	_fadeIntNode._intFunc = vDoFadeInt;
	_cycleIntNode._intFunc = vDoCycleInt;

}

void GraphicsManager::addIntNode(IntNode *node) {
	
}

void GraphicsManager::fadeIntFunc() {

}

void GraphicsManager::vDoFadeInt() {
	
}

void GraphicsManager::vDoCycleInt() {

}

/*------------------------------------------------------------------------*/

IntNode::IntNode() {
	_nextNode = NULL;
	_intFunc = NULL;
	_curTime = 0;
	_timeReset = 0;
	_flags = 0;
}

} // End of namespace Voyeur
