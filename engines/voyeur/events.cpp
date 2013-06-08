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

#include "voyeur/events.h"
#include "voyeur/voyeur.h"
#include "graphics/palette.h"

namespace Voyeur {

IntNode::IntNode() {
	_intFunc = NULL;
	_curTime = 0;
	_timeReset = 0;
	_flags = 0;
}

/*------------------------------------------------------------------------*/

EventsManager::EventsManager(): _intPtr(_audioStruc) {
	_cycleStatus = 0;
	_mouseButton = 0;
	_fadeStatus = 0;
	_priorFrameTime = g_system->getMillis();
	Common::fill(&_keyState[0], &_keyState[256], false);
}

void EventsManager::resetMouse() {
	// No implementation
}

void EventsManager::startMainClockInt() {
	_mainIntNode._intFunc = &EventsManager::mainVoyeurIntFunc;
	_mainIntNode._flags = 0;
	_mainIntNode._curTime = 0;
	_mainIntNode._timeReset = _vm->_graphicsManager._palFlag ? 50 : 60;
}

void EventsManager::mainVoyeurIntFunc() {
	// TODO
}

void EventsManager::vStopCycle() {
	_cycleIntNode._flags = 1;
	_cycleStatus &= 2;
}

void EventsManager::sWaitFlip() {
	// TODO: See if this needs a proper wait loop with event polling
	//while (_intPtr._field39) ;

	Common::Array<ViewPortResource *> &viewPorts = _vm->_graphicsManager._viewPortListPtr->_entries;
	for (uint idx = 0; idx < viewPorts.size(); ++idx) {
		ViewPortResource &viewPort = *viewPorts[idx];

		if (_vm->_graphicsManager._saveBack && (viewPort._flags & 0x40)) {
			Common::Rect *clipPtr = _vm->_graphicsManager._clipPtr;
			_vm->_graphicsManager._clipPtr = &viewPort._clipRect;

			if (viewPort._restoreFn)
				(_vm->_graphicsManager.*viewPort._restoreFn)(&viewPort);

			_vm->_graphicsManager._clipPtr = clipPtr;
			viewPort._rectListCount[viewPort._pageIndex] = 0;
			viewPort._flags &= 0xFFBF;
		}
	}
}

void EventsManager::checkForNextFrameCounter() {
	// Check for next game frame
	uint32 milli = g_system->getMillis();
	if ((milli - _priorFrameTime) >= GAME_FRAME_TIME) {
		++_gameCounter;
		_priorFrameTime = milli;

		// Run the timer-based updates
		videoTimer();

		// Display the frame
		g_system->copyRectToScreen((byte *)_vm->_graphicsManager._screenSurface.pixels, 
			SCREEN_WIDTH, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		g_system->updateScreen();

		// Signal the ScummVM debugger
		_vm->_debugger.onFrame();
	}
}

void EventsManager::videoTimer() {
	if (_audioStruc._hasPalette) {
		_audioStruc._hasPalette = false;

		g_system->getPaletteManager()->setPalette(_audioStruc._palette,
			_audioStruc._palStartIndex, 
			_audioStruc._palEndIndex - _audioStruc._palStartIndex + 1);
	}
}

void EventsManager::delay(int cycles) {
	uint32 totalMilli = cycles * 1000 / GAME_FRAME_RATE;
	uint32 delayEnd = g_system->getMillis() + totalMilli;

	while (!_vm->shouldQuit() && g_system->getMillis() < delayEnd) {
		g_system->delayMillis(10);

		pollEvents();
	}
}

void EventsManager::pollEvents() {
	checkForNextFrameCounter();

	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		// Handle keypress
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RTL:
			return;

		case Common::EVENT_KEYDOWN:
			_keyState[(byte)toupper(event.kbd.ascii)] = true;
			return;
		case Common::EVENT_KEYUP:
			_keyState[(byte)toupper(event.kbd.ascii)] = false;
			return;
		case Common::EVENT_LBUTTONDOWN:
			_mouseButton = 1;
			return;
		case Common::EVENT_RBUTTONDOWN:
			_mouseButton = 2;
			return;
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONUP:
			_mouseButton = 0;
			return;
		default:
 			break;
		}
	}
}

void EventsManager::startFade(CMapResource *cMap) {
	_fadeIntNode._flags |= 1;
	if (_cycleStatus & 1)
		_cycleIntNode._flags |= 1;

	_fadeFirstCol = cMap->_start;
	_fadeLastCol = cMap->_end;
	_fadeCount = cMap->_steps + 1;

	if (cMap->_steps > 0) {
		_fadeStatus = cMap->_fadeStatus |= 1;
		byte *vgaP = &_vm->_graphicsManager._VGAColors[_fadeFirstCol * 3];
		int mapIndex = 0;

		for (int idx = _fadeFirstCol; idx <= _fadeLastCol; ++idx) {
			ViewPortPalEntry &palEntry = _vm->_graphicsManager._viewPortListPtr->_palette[idx];
			palEntry._rEntry = vgaP[0] << 8;
			uint32 rComp = (uint16)((cMap->_entries[mapIndex * 3] << 8) - palEntry._rEntry) | 0x80;
			palEntry._rChange = rComp / cMap->_steps;

			palEntry._gEntry = vgaP[1] << 8;
			uint32 gComp = (uint16)((cMap->_entries[mapIndex * 3 + 1] << 8) - palEntry._gEntry) | 0x80;
			palEntry._gChange = gComp / cMap->_steps;

			palEntry._bEntry = vgaP[2] << 8;
			uint32 bComp = (uint16)((cMap->_entries[mapIndex * 3 + 2] << 8) -palEntry._bEntry) | 0x80;
			palEntry._bChange = bComp / cMap->_steps;
			palEntry._palIndex = bComp % cMap->_steps;

			if (!(cMap->_fadeStatus & 1))
				++mapIndex;
		}

		if (cMap->_fadeStatus & 2)
			_intPtr._field3B = 1;
		_fadeIntNode._flags &= ~1;
	} else {
		byte *vgaP = &_vm->_graphicsManager._VGAColors[_fadeFirstCol * 3];
		int mapIndex = 0;

		for (int idx = _fadeFirstCol; idx <= _fadeLastCol; ++idx, vgaP += 3) {
			Common::copy(&cMap->_entries[mapIndex], &cMap->_entries[mapIndex + 3], vgaP);

			if (!(cMap->_fadeStatus & 1))
				mapIndex += 3;
		}

		if (_intPtr._palStartIndex > _fadeFirstCol)
			_intPtr._palStartIndex = _fadeFirstCol;
		if (_intPtr._palEndIndex < _fadeLastCol)
			_intPtr._palEndIndex = _fadeLastCol;

		_intPtr._hasPalette = true;
		if (!(cMap->_fadeStatus & 2))
			_intPtr._field38 = 1;
	}

	if (_cycleStatus & 1)
		_cycleIntNode._flags &= ~1;
}

void EventsManager::addIntNode(IntNode *node) {
	_intNodes.push_back(node);
}

void EventsManager::addFadeInt() {
	IntNode &node = _fadeIntNode;
	node._intFunc = &EventsManager::fadeIntFunc;
	node._flags = 0;
	node._curTime = 0;
	node._timeReset = 1;

	addIntNode(&node);
}

void EventsManager::vDoFadeInt() {
	if (_intPtr._field3B & 1)
		return;
	if (--_fadeCount == 0) {
		_fadeIntNode._flags |= 1;
		_fadeStatus &= ~1;
	}


	for (int i = _fadeFirstCol; i <= _fadeLastCol; ++i) {
		ViewPortPalEntry &palEntry = _vm->_graphicsManager._viewPortListPtr->_palette[i];
		byte *vgaP = &_vm->_graphicsManager._VGAColors[palEntry._palIndex * 3];

		palEntry._rEntry += palEntry._rChange;
		palEntry._gEntry += palEntry._gChange;
		palEntry._bEntry += palEntry._bChange;

		vgaP[0] = palEntry._rEntry >> 8;
		vgaP[1] = palEntry._gEntry >> 8;
		vgaP[2] = palEntry._bEntry >> 8;
	}

	if (_intPtr._palStartIndex > _fadeFirstCol)
		_intPtr._palStartIndex = _fadeFirstCol;
	if (_intPtr._palEndIndex < _fadeLastCol)
		_intPtr._palEndIndex = _fadeLastCol;

	_intPtr._hasPalette = true;
	_intPtr._field38 = 1;
}

void EventsManager::vDoCycleInt() {
	// TODO: more
}


void EventsManager::fadeIntFunc() {
	// TODO: more
}

void EventsManager::vInitColor() {
	_fadeIntNode._intFunc = &EventsManager::vDoFadeInt;
	_cycleIntNode._intFunc = &EventsManager::vDoCycleInt;
	// TODO: more
}

} // End of namespace Voyeur
