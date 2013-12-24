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
#include "common/events.h"
#include "graphics/cursorman.h"
#include "graphics/palette.h"

namespace Voyeur {

IntNode::IntNode() {
	_intFunc = NULL;
	_curTime = 0;
	_timeReset = 0;
	_flags = 0;
}

IntNode::IntNode(uint16 curTime, uint16 timeReset, uint16 flags) {
	_intFunc = NULL;
	_curTime = curTime;
	_timeReset = timeReset;
	_flags = flags;
}

/*------------------------------------------------------------------------*/

IntData::IntData() {
	_field9 = false;
	_flipWait = false;
	_hasPalette = false;
	field16 = 0;
	field1A = 0;
	field1E = 0;
	field22 = 0;
	field24 = 0;
	field26 = 0;
	field2A = 0;
	field38 = 0;
	field3B = 0;
	field3D = 0;
	_palStartIndex = 0;
	_palEndIndex = 0;
	_palette = NULL;
}

/*------------------------------------------------------------------------*/

EventsManager::EventsManager(): _intPtr(_gameData),
		_fadeIntNode(0, 0, 3), _cycleIntNode(0, 0, 3) {
	_cycleStatus = 0;
	_mouseButton = 0;
	_fadeStatus = 0;
	_priorFrameTime = g_system->getMillis();
	_joe = 0;
	Common::fill(&_keyState[0], &_keyState[256], false);

	_v2A0A2 = 0;
	_videoComputerBut4 = 0;
	_videoDead = 0;
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
	if (!(_vm->_voy._field478 & 1)) {
		++_vm->_voy._switchBGNum;
		++_vm->_voy._RTVNum;
		if (_vm->_voy._RTVNum >= _vm->_voy._field4F2)
			_vm->_voy._field4F0 = 1;
	}
}

void EventsManager::vStopCycle() {
	_cycleIntNode._flags = 1;
	_cycleStatus &= 2;
}

void EventsManager::sWaitFlip() {
	while (_gameData._flipWait && !_vm->shouldQuit()) {
		pollEvents();
		g_system->delayMillis(10);
	}

	Common::Array<ViewPortResource *> &viewPorts = _vm->_graphicsManager._viewPortListPtr->_entries;
	for (uint idx = 0; idx < viewPorts.size(); ++idx) {
		ViewPortResource &viewPort = *viewPorts[idx];

		if (_vm->_graphicsManager._saveBack && (viewPort._flags & DISPFLAG_40)) {
			Common::Rect *clipPtr = _vm->_graphicsManager._clipPtr;
			_vm->_graphicsManager._clipPtr = &viewPort._clipRect;

			if (viewPort._restoreFn)
				(_vm->_graphicsManager.*viewPort._restoreFn)(&viewPort);

			_vm->_graphicsManager._clipPtr = clipPtr;
			viewPort._rectListCount[viewPort._pageIndex] = 0;
			viewPort._rectListPtr[viewPort._pageIndex]->clear();
			viewPort._flags &= ~DISPFLAG_40;
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
		voyeurTimer();

		if ((_gameCounter % GAME_FRAME_RATE) == 0)
			mainVoyeurIntFunc();

		// Display the frame
		g_system->copyRectToScreen((byte *)_vm->_graphicsManager._screenSurface.getPixels(), 
			SCREEN_WIDTH, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		g_system->updateScreen();

		// Signal the ScummVM debugger
		_vm->_debugger.onFrame();
	}
}

void EventsManager::voyeurTimer() {
	_gameData.field22 += _gameData.field24;
	_gameData.field1A += _gameData.field1E;
	// _gameData.field1C += _gameData._timerFn; *** WHY INC field by a function pointer?!

	_gameData.field16 = 0;
	_gameData.field3D = 1;

	if (--_gameData.field26 <= 0) {
		if (_gameData._flipWait) {
			_gameData.field38 = 1;
			_gameData._flipWait = false;
			_gameData.field3B = 0;
		}

		_gameData.field26 >>= 8;
	}

	videoTimer();

	// Iterate through the list of registered nodes
	Common::List<IntNode *>::iterator i;
	for (i = _intNodes.begin(); i != _intNodes.end(); ++i) {
		IntNode &node = **i;

		if (node._flags & 1)
			continue;
		if (!(node._flags & 2)) {
			if (--node._curTime != 0)
				continue;

			node._curTime = node._timeReset;
		}

		(this->*node._intFunc)();
	}

}

void EventsManager::videoTimer() {
	if (_gameData._hasPalette) {
		_gameData._hasPalette = false;

		g_system->getPaletteManager()->setPalette(_gameData._palette +
			_gameData._palStartIndex * 3, _gameData._palStartIndex, 
			_gameData._palEndIndex - _gameData._palStartIndex + 1);
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
			_vm->_voy._newMouseClicked = true;
			return;
		case Common::EVENT_RBUTTONDOWN:
			_mouseButton = 2;
			_vm->_voy._newMouseClicked = true;
			return;
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONUP:
			_vm->_voy._newMouseClicked = false;
			_mouseButton = 0;
			return;
		case Common::EVENT_MOUSEMOVE:
			_mousePos = event.mouse;
			break;
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
		_fadeStatus = cMap->_fadeStatus | 1;
		byte *vgaP = &_vm->_graphicsManager._VGAColors[_fadeFirstCol * 3];
		int mapIndex = 0;

		for (int idx = _fadeFirstCol; idx <= _fadeLastCol; ++idx, vgaP += 3) {
			ViewPortPalEntry &palEntry = _vm->_graphicsManager._viewPortListPtr->_palette[idx];
			palEntry._rEntry = vgaP[0] << 8;
			int rDiff = (cMap->_entries[mapIndex * 3] << 8) - palEntry._rEntry;
			palEntry._rChange = rDiff / cMap->_steps;

			palEntry._gEntry = vgaP[1] << 8;
			int gDiff = (cMap->_entries[mapIndex * 3 + 1] << 8) - palEntry._gEntry;
			palEntry._gChange = gDiff / cMap->_steps;

			palEntry._bEntry = vgaP[2] << 8;
			int bDiff = (cMap->_entries[mapIndex * 3 + 2] << 8) - palEntry._bEntry;
			palEntry._bChange = bDiff / cMap->_steps;
			
			palEntry._palIndex = idx;
			if (!(cMap->_fadeStatus & 1))
				++mapIndex;
		}

		if (cMap->_fadeStatus & 2)
			_intPtr.field3B = 1;
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
			_intPtr.field38 = 1;
	}

	if (_cycleStatus & 1)
		_cycleIntNode._flags &= ~1;
}

void EventsManager::addIntNode(IntNode *node) {
	_intNodes.push_back(node);
}

void EventsManager::addFadeInt() {
	IntNode &node = _fade2IntNode;
	node._intFunc = &EventsManager::fadeIntFunc;
	node._flags = 0;
	node._curTime = 0;
	node._timeReset = 1;

	addIntNode(&node);
}

void EventsManager::vDoFadeInt() {
	if (_intPtr.field3B & 1)
		return;
	if (--_fadeCount == 0) {
		_fadeIntNode._flags |= 1;
		_fadeStatus &= ~1;
		return;
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
	_intPtr.field38 = 1;
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

	addIntNode(&_fadeIntNode);
	addIntNode(&_cycleIntNode);
}

void EventsManager::setCursor(PictureResource *pic) {
	PictureResource cursor;
	cursor._bounds = pic->_bounds;
	cursor._flags = DISPFLAG_CURSOR;

	_vm->_graphicsManager.sDrawPic(pic, &cursor, Common::Point());
}

void EventsManager::setCursor(byte *cursorData, int width, int height) {
	CursorMan.replaceCursor(cursorData, width, height, 0, 0, 0);
}

void EventsManager::setCursorColor(int idx, int mode) {
	switch (mode) {
	case 0:
		_vm->_graphicsManager.setColor(idx, 90, 90, 232);
		break;
	case 1:
		_vm->_graphicsManager.setColor(idx, 232, 90, 90);
		break;
	case 2:
		_vm->_graphicsManager.setColor(idx, 90, 232, 90);
		break;
	case 3:
		_vm->_graphicsManager.setColor(idx, 90, 232, 232);
		break;
	default:
		break;
	}
}

void EventsManager::mouseOn() {
	CursorMan.showMouse(true);
}

void EventsManager::mouseOff() {
	CursorMan.showMouse(false);
}

void EventsManager::hideCursor() {
	error("TODO: hideCursor");
}

void EventsManager::getMouseInfo() {
	pollEvents();

	if (_vm->_voy._field478 & 0x10) {
		if ((_gameCounter - _joe) > 8) {
			_joe = _gameCounter;

			if (_vm->_bob) {
				_vm->_bob = false;
				_vm->_graphicsManager.setOneColor(128, 55, 5, 5);
				_vm->_graphicsManager.setColor(128, 220, 20, 20);
			} else {
				_vm->_bob = true;
				_vm->_graphicsManager.setOneColor(128, 55, 55, 55);
				_vm->_graphicsManager.setColor(128, 220, 20, 20);
			}
		}
	}

	_vm->_voy._incriminate = _vm->_voy._newIncriminate;
	_vm->_voy._mouseClicked = _vm->_voy._newMouseClicked;
	_vm->_voy._fadeFunc = _vm->_voy._newFadeFunc;
	_vm->_voy._fadeICF1 = _vm->_voy._newFadeICF1;
}

void EventsManager::checkForKey() {
	warning("TODO: checkForKey");
}

void EventsManager::startCursorBlink() {
	error("TODO: startCursorBlink");
}

void EventsManager::incrementTime(int amt) {
	for (int i = 0; i < amt; ++i)
		mainVoyeurIntFunc();
}

void EventsManager::addVideoEventStart() {
	VoyeurEvent &e = _vm->_voy._events[_vm->_voy._eventCount];
	e._hour = _vm->_gameHour;
	e._minute = _vm->_gameMinute;
	e._isAM = _vm->_voy._isAM;
	e._type = EVTYPE_VIDEO;
	e._field8 = _vm->_eventsManager._videoComputerBut4;
	e._computerOn = _vm->_voy._vocSecondsOffset;
	e._dead = _vm->_eventsManager._videoDead;
}

void EventsManager::addVideoEventEnd() {
	VoyeurEvent &e = _vm->_voy._events[_vm->_voy._eventCount];
	e._computerOff = _vm->_voy._RTVNum - _vm->_voy._field468 - _vm->_voy._vocSecondsOffset;
	if (_vm->_voy._eventCount < (TOTAL_EVENTS - 1))
		++_vm->_voy._eventCount;
}

void EventsManager::addAudioEventStart() {
	VoyeurEvent &e = _vm->_voy._events[_vm->_voy._eventCount];
	e._hour = _vm->_gameHour;
	e._minute = _vm->_gameMinute;
	e._isAM = _vm->_voy._isAM;
	e._type = EVTYPE_AUDIO;
	e._field8 = _vm->_eventsManager._videoComputerBut4;
	e._computerOn = _vm->_voy._field47A;
	e._dead = _vm->_eventsManager._videoDead;
}

void EventsManager::addAudioEventEnd() {
	VoyeurEvent &e = _vm->_voy._events[_vm->_voy._eventCount];
	e._computerOff = _vm->_voy._RTVNum - _vm->_voy._field468 - _vm->_voy._vocSecondsOffset;
	if (_vm->_voy._eventCount < (TOTAL_EVENTS - 1))
		++_vm->_voy._eventCount;
}

void EventsManager::addEvidEventStart(int v) {
	VoyeurEvent &e = _vm->_voy._events[_vm->_voy._eventCount];
	e._hour = _vm->_gameHour;
	e._minute = _vm->_gameMinute;
	e._isAM = _vm->_voy._isAM;
	e._type = EVTYPE_EVID;
	e._field8 = _vm->_eventsManager._videoComputerBut4;
	e._computerOn = _vm->_voy._vocSecondsOffset;
	e._dead = _vm->_eventsManager._videoDead;

}

void EventsManager::addEvidEventEnd(int dead) {
	VoyeurEvent &e = _vm->_voy._events[_vm->_voy._eventCount];
	e._dead = dead;
	if (_vm->_voy._eventCount < (TOTAL_EVENTS - 1))
		++_vm->_voy._eventCount;
}

void EventsManager::addComputerEventStart() {
	VoyeurEvent &e = _vm->_voy._events[_vm->_voy._eventCount];
	e._hour = _vm->_gameHour;
	e._minute = _vm->_gameMinute;
	e._isAM = _vm->_voy._isAM;
	e._type = EVTYPE_COMPUTER;
	e._field8 = _vm->_playStamp1;
	e._computerOn = _vm->_voy._computerTextId;
}

void EventsManager::addComputerEventEnd(int v) {
	VoyeurEvent &e = _vm->_voy._events[_vm->_voy._eventCount];
	e._computerOff = v;
	if (_vm->_voy._eventCount < (TOTAL_EVENTS - 1))
		++_vm->_voy._eventCount;
}

} // End of namespace Voyeur
