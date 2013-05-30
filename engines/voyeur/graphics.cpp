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
#include "voyeur/game.h"
#include "voyeur/voyeur.h"
#include "engines/util.h"
#include "graphics/surface.h"

namespace Voyeur {

GraphicsManager::GraphicsManager() {
	_SVGAPage = 0;
	_SVGAMode = 0;
	_palFlag = false;
	_MCGAMode = false;
	_clipPtr = NULL;
}

void GraphicsManager::sInitGraphics() {
	initGraphics(SCREEN_WIDTH, SCREEN_HEIGHT, false);
}

void GraphicsManager::addFadeInt() {
	IntNode &node = _vm->_eventsManager._fadeIntNode;
	node._intFunc = fadeIntFunc;
	node._flags = 0;
	node._curTime = 0;
	node._timeReset = 1;

	_vm->_intPtr.addIntNode(&node);
}

void GraphicsManager::vInitColor() {
	_vm->_eventsManager._fadeIntNode._intFunc = vDoFadeInt;
	_vm->_eventsManager._cycleIntNode._intFunc = vDoCycleInt;
	// TODO: more
}

void GraphicsManager::fadeIntFunc() {

}

void GraphicsManager::vDoFadeInt() {
	
}

void GraphicsManager::vDoCycleInt() {

}

void GraphicsManager::setupMCGASaveRect(ViewPortResource *viewPort) {
	_MCGAMode = true;

	if (viewPort->_activePage) {
		viewPort->_activePage->_flags |= 1;
		Common::Rect *clipRect = _clipPtr;
		_clipPtr = &viewPort->_clipRect;

		sDrawPic(viewPort->_activePage, viewPort->_picResource, viewPort, NULL);

		_clipPtr = clipRect;
	}

	viewPort->_field42 = -1;
}

void GraphicsManager::addRectOptSaveRect(ViewPortResource *viewPort, void *v2, void *v3) {

}

void GraphicsManager::restoreMCGASaveRect(ViewPortResource *viewPort) {

}

void GraphicsManager::addRectNoSaveBack(ViewPortResource *viewPort, void *v2, void *v3) {

}

void GraphicsManager::sDrawPic(PictureResource *pic, PictureResource *pic2, ViewPortResource *viewPort, void *v3) {

}

} // End of namespace Voyeur
