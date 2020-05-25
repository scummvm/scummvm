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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "voyeur/events.h"
#include "voyeur/voyeur.h"
#include "voyeur/staticres.h"
#include "common/events.h"
#include "graphics/cursorman.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
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
	_flipWait = false;
	_hasPalette = false;
	_flashTimer = 0;
	_flashStep = 0;
	_skipFading = false;
	_palStartIndex = 0;
	_palEndIndex = 0;
	_palette = NULL;
}

/*------------------------------------------------------------------------*/

EventsManager::EventsManager(VoyeurEngine *vm) : _intPtr(_gameData),
		_fadeIntNode(0, 0, 3), _cycleIntNode(0, 0, 3), _vm(vm) {
	_cycleStatus = 0;
	_fadeStatus = 0;
	_priorFrameTime = g_system->getMillis();
	_gameCounter = 0;
	_counterFlag = false;
	_recordBlinkCounter = 0;
	_cursorBlinked = false;

	Common::fill(&_cycleTime[0], &_cycleTime[4], 0);
	Common::fill(&_cycleNext[0], &_cycleNext[4], (byte *)nullptr);
	_cyclePtr = NULL;

	_leftClick = _rightClick = false;
	_mouseClicked = _newMouseClicked = false;
	_newLeftClick = _newRightClick = false;

	_videoDead = 0;

	_fadeFirstCol = _fadeLastCol = 0;
	_fadeCount = 1;

	for (int i = 0; i < 4; i++)
		_cycleNext[i] = nullptr;
}

void EventsManager::startMainClockInt() {
	_mainIntNode._intFunc = &EventsManager::mainVoyeurIntFunc;
	_mainIntNode._flags = 0;
	_mainIntNode._curTime = 0;
	_mainIntNode._timeReset = 60;
}

void EventsManager::mainVoyeurIntFunc() {
	if (!(_vm->_voy->_eventFlags & EVTFLAG_TIME_DISABLED)) {
		++_vm->_voy->_switchBGNum;

		if (_vm->_debugger->_isTimeActive) {
			// Increase camera discharge
			++_vm->_voy->_RTVNum;

			// If the murder threshold has been set, and is passed, then flag the victim
			// as murdered, which prevents sending the tape from succeeding
			if (_vm->_voy->_RTVNum >= _vm->_voy->_murderThreshold)
				_vm->_voy->_victimMurdered = true;
		}
	}
}

void EventsManager::sWaitFlip() {
	Common::Array<ViewPortResource *> &viewPorts = _vm->_screen->_viewPortListPtr->_entries;
	for (uint idx = 0; idx < viewPorts.size(); ++idx) {
		ViewPortResource &viewPort = *viewPorts[idx];

		if (_vm->_screen->_saveBack && (viewPort._flags & DISPFLAG_40)) {
			Common::Rect *clipPtr = _vm->_screen->_clipPtr;
			_vm->_screen->_clipPtr = &viewPort._clipRect;

			if (viewPort._restoreFn)
				(_vm->_screen->*viewPort._restoreFn)(&viewPort);

			_vm->_screen->_clipPtr = clipPtr;
			viewPort._rectListCount[viewPort._pageIndex] = 0;
			viewPort._rectListPtr[viewPort._pageIndex]->clear();
			viewPort._flags &= ~DISPFLAG_40;
		}
	}

	while (_gameData._flipWait && !_vm->shouldQuit()) {
		pollEvents();
		g_system->delayMillis(10);
	}
}

void EventsManager::checkForNextFrameCounter() {
	// Check for next game frame
	uint32 milli = g_system->getMillis();
	if ((milli - _priorFrameTime) >= GAME_FRAME_TIME) {
		_counterFlag = !_counterFlag;
		if (_counterFlag)
			++_gameCounter;
		_priorFrameTime = milli;

		// Run the timer-based updates
		voyeurTimer();

		if ((_gameCounter % GAME_FRAME_RATE) == 0)
			mainVoyeurIntFunc();

		// If mouse position display is on, display the position
		if (_vm->_debugger->_showMousePosition)
			showMousePosition();

		// Display the frame
		_vm->_screen->update();
	}
}

void EventsManager::showMousePosition() {
	const Graphics::Font &font(*FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont));
	Common::String mousePos = Common::String::format("(%d,%d)", _mousePos.x, _mousePos.y);
	if (_vm->_voyeurArea == AREA_INTERFACE) {
		Common::Point pt = _mousePos + _vm->_mansionViewPos - Common::Point(40, 27);
		if (pt.x < 0) pt.x = 0;
		if (pt.y < 0) pt.y = 0;

		mousePos += Common::String::format(" - (%d,%d)", pt.x, pt.y);
	}

	_vm->_screen->fillRect(Common::Rect(0, 0, 110, font.getFontHeight()), 0);
	font.drawString(_vm->_screen, mousePos, 0, 0, 110, 63);
}

void EventsManager::voyeurTimer() {
	_gameData._flashTimer += _gameData._flashStep;

	if (_gameData._flipWait) {
		_gameData._flipWait = false;
		_gameData._skipFading = false;
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

void EventsManager::delayClick(int cycles) {
	uint32 totalMilli = cycles * 1000 / GAME_FRAME_RATE;
	uint32 delayEnd = g_system->getMillis() + totalMilli;

	do {
		g_system->delayMillis(10);
		getMouseInfo();
	} while (!_vm->shouldQuit() && g_system->getMillis() < delayEnd
			&& !_vm->_eventsManager->_mouseClicked);
}

void EventsManager::pollEvents() {
	checkForNextFrameCounter();

	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event) && !_vm->shouldQuit()) {
		// Handle keypress
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
		case Common::EVENT_KEYUP:
			return;

		case Common::EVENT_KEYDOWN:
			return;
		case Common::EVENT_LBUTTONDOWN:
			_newLeftClick = true;
			_newMouseClicked = true;
			return;
		case Common::EVENT_RBUTTONDOWN:
			_newRightClick = true;
			_newMouseClicked = true;
			return;
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONUP:
			_newMouseClicked = false;
			_newLeftClick = false;
			_newRightClick = false;
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
		byte *vgaP = &_vm->_screen->_VGAColors[_fadeFirstCol * 3];
		int mapIndex = 0;

		for (int idx = _fadeFirstCol; idx <= _fadeLastCol; ++idx, vgaP += 3) {
			ViewPortPalEntry &palEntry = _vm->_screen->_viewPortListPtr->_palette[idx];
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
			_intPtr._skipFading = true;
		_fadeIntNode._flags &= ~1;
	} else {
		byte *vgaP = &_vm->_screen->_VGAColors[_fadeFirstCol * 3];
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
	if (_intPtr._skipFading)
		return;
	if (--_fadeCount == 0) {
		_fadeIntNode._flags |= 1;
		_fadeStatus &= ~1;
		return;
	}

	for (int i = _fadeFirstCol; i <= _fadeLastCol; ++i) {
		ViewPortPalEntry &palEntry = _vm->_screen->_viewPortListPtr->_palette[i];
		byte *vgaP = &_vm->_screen->_VGAColors[palEntry._palIndex * 3];

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
}

void EventsManager::vDoCycleInt() {
	for (int idx = 3; idx >= 0; --idx) {
		if (_cyclePtr->_type[idx] && --_cycleTime[idx] <= 0) {
			byte *pSrc = _cycleNext[idx];
			byte *pPal = _vm->_screen->_VGAColors;

			if (_cyclePtr->_type[idx] != 1) {
				// New palette data being specified - loop to set entries
				do {
					int palIndex = READ_LE_UINT16(pSrc);
					pPal[palIndex * 3] = pSrc[3];
					pPal[palIndex * 3 + 1] = pSrc[4];
					pPal[palIndex * 3 + 2] = pSrc[5];
					pSrc += 6;

					if ((int16)READ_LE_UINT16(pSrc) >= 0) {
						// Resetting back to start of cycle data
						pSrc = _cycleNext[idx];
						break;
					}
				} while (*(pSrc + 2) == 0);

				_cycleNext[idx] = pSrc;
				_cycleTime[idx] = pSrc[2];
			} else {
				// Palette rotation to be done
				_cycleTime[idx] = pSrc[4];

				if (pSrc[5] == 1) {
					// Move palette entry to end of range
					int start = READ_LE_UINT16(pSrc);
					int end = READ_LE_UINT16(&pSrc[2]);
					assert(start < 0x100 && end < 0x100);

					// Store the RGB of the first entry to be moved
					byte r = pPal[start * 3];
					byte g = pPal[start * 3 + 1];
					byte b = pPal[start * 3 + 2];

					Common::copy(&pPal[start * 3 + 3], &pPal[end * 3 + 3], &pPal[start * 3]);

					// Place the original saved entry at the end of the range
					pPal[end * 3] = r;
					pPal[end * 3 + 1] = g;
					pPal[end * 3 + 2] = b;

					if (_fadeStatus & 1) {
						//dx = start, di = end
						warning("TODO: Adjustment of ViewPortListResource");
					}
				} else {
					// Move palette entry to start of range
					int start = READ_LE_UINT16(pSrc);
					int end = READ_LE_UINT16(&pSrc[2]);
					assert(start < 0x100 && end < 0x100);

					// Store the RGB of the entry to be moved
					byte r = pPal[end * 3];
					byte g = pPal[end * 3 + 1];
					byte b = pPal[end * 3 + 2];

					// Move the remainder of the range forwards one entry
					Common::copy_backward(&pPal[start * 3], &pPal[end * 3], &pPal[end * 3 + 3]);

					// Place the original saved entry at the end of the range
					pPal[start * 3] = r;
					pPal[start * 3 + 1] = g;
					pPal[start * 3 + 2] = b;

					if (_fadeStatus & 1) {
						//dx = start, di = end
						warning("TODO: Adjustment of ViewPortListResource");
					}
				}
			}

			_intPtr._hasPalette = true;
		}
	}
}


void EventsManager::fadeIntFunc() {
	switch (_vm->_voy->_fadingType) {
	case 1:
		if (_vm->_voy->_fadingAmount1 < 63)
			_vm->_voy->_fadingAmount1 += _vm->_voy->_fadingStep1;
		if (_vm->_voy->_fadingAmount2 < 63)
			_vm->_voy->_fadingAmount2 += _vm->_voy->_fadingStep2;
		if (_vm->_voy->_fadingAmount1 > 63)
			_vm->_voy->_fadingAmount1 = 63;
		if (_vm->_voy->_fadingAmount2 > 63)
			_vm->_voy->_fadingAmount2 = 63;
		if ((_vm->_voy->_fadingAmount1 == 63) && (_vm->_voy->_fadingAmount2 == 63))
			_vm->_voy->_fadingType = 0;
		break;
	case 2:
		if (_vm->_voy->_fadingAmount1 > 0)
			_vm->_voy->_fadingAmount1 -= _vm->_voy->_fadingStep1;
		if (_vm->_voy->_fadingAmount2 > 0)
			_vm->_voy->_fadingAmount2 -= _vm->_voy->_fadingStep2;
		if (_vm->_voy->_fadingAmount1 < 0)
			_vm->_voy->_fadingAmount1 = 0;
		if (_vm->_voy->_fadingAmount2 < 0)
			_vm->_voy->_fadingAmount2 = 0;
		if ((_vm->_voy->_fadingAmount1 == 0) && (_vm->_voy->_fadingAmount2 == 0))
			_vm->_voy->_fadingType = 0;
		break;
	default:
		break;
	}
}

void EventsManager::deleteIntNode(IntNode *node) {
	_intNodes.remove(node);
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

	_vm->_screen->sDrawPic(pic, &cursor, Common::Point());
}

void EventsManager::setCursor(byte *cursorData, int width, int height, int keyColor) {
	CursorMan.replaceCursor(cursorData, width, height, width / 2, height / 2, keyColor);
}

void EventsManager::setCursorColor(int idx, int mode) {
	switch (mode) {
	case 0:
		_vm->_screen->setColor(idx, 90, 90, 232);
		break;
	case 1:
		_vm->_screen->setColor(idx, 232, 90, 90);
		break;
	case 2:
		_vm->_screen->setColor(idx, 90, 232, 90);
		break;
	case 3:
		_vm->_screen->setColor(idx, 90, 232, 232);
		break;
	default:
		break;
	}
}

void EventsManager::showCursor() {
	CursorMan.showMouse(true);
}

void EventsManager::hideCursor() {
	CursorMan.showMouse(false);
}

void EventsManager::getMouseInfo() {
	pollEvents();

	if (_vm->_voy->_eventFlags & EVTFLAG_RECORDING) {
		if ((_gameCounter - _recordBlinkCounter) > 8) {
			_recordBlinkCounter = _gameCounter;

			if (_cursorBlinked) {
				_cursorBlinked = false;
				_vm->_screen->setOneColor(128, 220, 20, 20);
				_vm->_screen->setColor(128, 220, 20, 20);
			} else {
				_cursorBlinked = true;
				_vm->_screen->setOneColor(128, 220, 220, 220);
				_vm->_screen->setColor(128, 220, 220, 220);
			}
		}
	}

	_mouseClicked = _newMouseClicked;
	_leftClick = _newLeftClick;
	_rightClick = _newRightClick;

	_newMouseClicked = false;
	_newLeftClick = false;
	_newRightClick = false;
}

void EventsManager::startCursorBlink() {
	if (_vm->_voy->_eventFlags & EVTFLAG_RECORDING) {
		_vm->_screen->setOneColor(128, 55, 5, 5);
		_vm->_screen->setColor(128, 220, 20, 20);
		_intPtr._hasPalette = true;

		_vm->_screen->drawDot();
		//copySection();
	}
}

void EventsManager::incrementTime(int amt) {
	for (int i = 0; i < amt; ++i)
		mainVoyeurIntFunc();
}

void EventsManager::stopEvidDim() {
	deleteIntNode(&_evIntNode);
}

Common::String EventsManager::getEvidString(int eventIndex) {
	assert(eventIndex <= _vm->_voy->_eventCount);
	VoyeurEvent &e = _vm->_voy->_events[eventIndex];
	return Common::String::format("%03d %.2d:%.2d %s %s", eventIndex + 1,
		e._hour, e._minute, e._isAM ? AM : PM, EVENT_TYPE_STRINGS[e._type - 1]);
}

} // End of namespace Voyeur
