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

#include "voyeur/voyeur.h"
#include "voyeur/staticres.h"

namespace Voyeur {

void VoyeurEngine::addVideoEventStart() {
	VoyeurEvent &e = _voy._events[_voy._evidence[19]];
	e._computerNum = _gameHour;
	e._computerBut[0] = _gameMinute;
	e._computerBut[1] = _voy._isAM;
	e._computerBut[2] = 1;
	e._computerBut[3] = _eventsManager._videoComputerBut4;
	e._dead = _eventsManager._videoDead;
}

void VoyeurEngine::addComputerEventEnd() {
	error("TODO: addComputerEventEnd");
}

void VoyeurEngine::addPlainEvent() {
	error("TODO: addPlainEvent");
}

void VoyeurEngine::playStamp() {
	_stampLibPtr = NULL;
	_filesManager.openBoltLib("stampblt.blt", _stampLibPtr);

	_stampLibPtr->getBoltGroup(0);
	_resolvePtr = &RESOLVE_TABLE[0];
	initStamp();

	PtrResource *threadsList = _stampLibPtr->boltEntry(3)._ptrResource;
	ThreadResource *threadP = threadsList->_entries[0]->_threadResource;
	threadP->initThreadStruct(0, 0);

	_voy._isAM = 0;
	_gameHour = 9;
	_gameMinute = 0;
	_eventsManager._v2A0A2 = 0;
	_voy._field46E = 1;

	int buttonId;
	bool breakFlag = false;
	while (!breakFlag && !shouldQuit()) {
		_eventsManager.getMouseInfo();
		_playStamp1 = _playStamp2 = -1;
		_eventsManager._videoComputerBut4 = -1;

		threadP->parsePlayCommands();

		bool flag = breakFlag = (_voy._field478 & 2) != 0;
		 
		switch (_voy._field470) {
		case 5:
			buttonId = threadP->doInterface();
			
			if (buttonId == -2) {
				switch (threadP->doApt()) {
				case 0:
					_voy._field472 = 140;
					break;
				case 1:
					_voy._field478 = -2;
					_voy._field46E = 1;
					threadP->chooseSTAMPButton(22);
					_voy._field472 = 143;
					break;
				case 2:
					_voy._field478 = -2;
					reviewTape();
					_voy._field46E = 1;
					_voy._field472 = 142;
					break;
				case 3:
					_voy._field478 = -2;
					threadP->chooseSTAMPButton(21);
					break;
				case 4:
					breakFlag = true;
					break;
				case 5:
					doGossip();
					_voy._field46E = 1;
					_voy._field472 = 141;
					_voy._field478 = -1;
					break;
				default:
					break;
				}
			} else {
				threadP->chooseSTAMPButton(buttonId);
			}
			break;

		case 6:
			threadP->doRoom();
			break;

		case 16:
			_voy._transitionId = 17;
			buttonId = threadP->doApt();
			
			switch (buttonId) {
			case 1:
				threadP->chooseSTAMPButton(22);
				flag = true;
				break;
			case 2:
				reviewTape();
				_voy._field46E = 1;
				break;
			case 4:
				flag = true;
				breakFlag = true;
				break;
			default:
				break;
			}
			break;

		case 17:
			doTapePlaying();
			if (!checkForMurder() && _voy._transitionId <= 15)
				checkForIncriminate();

			if (_voy._videoEventId != -1)
				playAVideoEvent(_voy._videoEventId);
			_voy._field478 &= 0x10;
			threadP->chooseSTAMPButton(0);
			break;

		case 130: {
			//_tmflag = 1;
			if (_bVoy->getBoltGroup(_playStamp1)) {
				_graphicsManager._backgroundPage = _bVoy->boltEntry(_playStamp1)._picResource;
				_graphicsManager._backColors = _bVoy->boltEntry(_playStamp1 + 1)._cMapResource;

				buttonId = getChooseButton();
				if (_voy._fadeFunc)
					buttonId = 4;

				_bVoy->freeBoltGroup(_playStamp1);
				_graphicsManager.screenReset();
				_playStamp1 = -1;
				flag = true;

				if (buttonId == 4) {
					_voy._field470 = 131;
					_eventsManager.checkForKey();
					threadP->chooseSTAMPButton(buttonId);
					flag = true;
				} else {
					threadP->chooseSTAMPButton(buttonId);
					_voy._field46E = 1;
				}
			}
			break;
		}

		default:
			break;
		}

		do {
			if (flag) {
				if (_playStamp2 != -1) {
					_soundManager.stopVOCPlay();
					_playStamp2 = -1;
				}

				_eventsManager._videoComputerBut4 = -1;

				if (_voy._field47A != -1) {
					_bVoy->freeBoltGroup(_voy._field47A);
					_voy._field47A = -1;
				}

				if (_playStamp1 != -1) {
					_bVoy->freeBoltGroup(_playStamp1);
					_playStamp1 = -1;
				}

				// Break out of loop
				flag = false;

			} else if (threadP->_field40 & 2) {
				_eventsManager.getMouseInfo();
				threadP->chooseSTAMPButton(0);
				flag = true;
			} else {
				threadP->chooseSTAMPButton(0);
				flag = true;
			}
		} while (flag);
	}

	_voy._field4386 = 0;
	closeStamp();
	_stampLibPtr->freeBoltGroup(0);
	delete _stampLibPtr;
}

void VoyeurEngine::initStamp() {
	ThreadResource::_stampFlags &= ~1;
	_stackGroupPtr = _controlGroupPtr;

	if (!_controlPtr->_entries[0])
		error("No control entries");

	ThreadResource::initUseCount();
}

void VoyeurEngine::closeStamp() {
	ThreadResource::unloadAllStacks(this);
}

void VoyeurEngine::reviewTape() {
	warning("TODO: reviewTape");
}

bool VoyeurEngine::doGossip() {
	warning("TODO: doGossip");
	return false;
}

void VoyeurEngine::doTapePlaying() {
	warning("TODO");
}

bool VoyeurEngine::checkForMurder() {
	warning("TODO");
	return false;
}

void VoyeurEngine::checkForIncriminate() {
	warning("TODO");
}

void VoyeurEngine::playAVideoEvent(int eventId) {
	warning("TODO");
}

int VoyeurEngine::getChooseButton()  {
	warning("TODO");
	return 0;
}

void VoyeurEngine::makeViewFinder() {
	_graphicsManager._backgroundPage = _bVoy->boltEntry(0x103)._picResource;
	_graphicsManager.sDrawPic(_graphicsManager._backgroundPage, 
		*_graphicsManager._vPort, Common::Point(0, 0));
	CMapResource *pal = _bVoy->boltEntry(0x104)._cMapResource;

	int palOffset = 0;
	switch (_voy._transitionId) {
	case 1:
	case 2:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 17:
		palOffset = 0;
		break;
	case 3:
		palOffset = 1;
		break;
	case 4:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
		palOffset = 2;
		break;
	default:
		break;
	}

	(*_graphicsManager._vPort)->drawIfaceTime();
	doTimeBar(true);
	pal->startFade();

	(*_graphicsManager._vPort)->_flags |= 8;
	_graphicsManager.flipPage();
	_eventsManager.sWaitFlip();

	while (!shouldQuit() && (_eventsManager._fadeStatus & 1))
		_eventsManager.delay(1);

	_graphicsManager.setColor(241, 105, 105, 105);
	_graphicsManager.setColor(242, 105, 105, 105);
	_graphicsManager.setColor(243, 105, 105, 105);
	_graphicsManager.setColor(palOffset + 241, 219, 235, 235);

	_eventsManager._intPtr.field38 = 1;
	_eventsManager._intPtr._hasPalette = true;
}

void VoyeurEngine::makeViewFinderP() {
	_graphicsManager.screenReset();
}

void VoyeurEngine::initIFace(){
	int playStamp1 = _playStamp1;
	switch (_voy._transitionId) {
	case 0:
		break;
	case 1:
	case 2:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
		_playStamp1 = 0xB00;
		break;
	case 3:
		_playStamp1 = 0xC00;
		break;
	default:
		_playStamp1 = 0xD00;
		break;
	}
	if (playStamp1 != -1)
		_bVoy->freeBoltGroup(playStamp1, true);

	_bVoy->getBoltGroup(_playStamp1);
	CMapResource *pal = _bVoy->boltEntry(_playStamp1 + 2)._cMapResource;
	pal->startFade();

	_graphicsManager.doScroll(_eventsManager.getMousePos());
	
	_voy._field4386 = _bVoy->memberAddr(_playStamp1);

	// Note: the original did two loops to preload members here, which is
	// redundant for ScummVM, since computers are faster these days, and
	// getting resources as needed will be fast enough.
}

void VoyeurEngine::checkTransition(){
	Common::String time, day;

	if (_voy._transitionId != _checkTransitionId) {
		switch (_voy._transitionId) {
		case 0:
			break;
		case 1:
		case 2:
		case 3:
		case 4:
			day = SATURDAY;
			break;
		case 17: 
			day = MONDAY;
			break;
		default:
			day = SUNDAY;
			break;
		}

		if (!day.empty()) {
			_graphicsManager.fadeDownICF(6);

			if (_voy._transitionId != 17) {
				const char *amPm = _voy._isAM ? AM : PM;
				time = Common::String::format("%d:%02d%s",
					_gameHour, _gameMinute, amPm);
			}

			doTransitionCard(day, time);
			_eventsManager.delay(180);
		}

		_checkTransitionId = _voy._transitionId;
	}
}

bool VoyeurEngine::doComputerText(int v) {
	error("TODO: doComputerText");
}

void VoyeurEngine::getComputerBrush() {
	error("TODO: getComputerBrush");
}

void VoyeurEngine::doTimeBar(bool force) {
	flashTimeBar();

	if ((force || _timeBarVal != _voy._RTVNum) && _voy._field476 > 0) {
		if (_voy._RTVNum > _voy._field476 || _voy._RTVNum < 0)
			_voy._RTVNum = _voy._field476 - 1;
		
		_timeBarVal = _voy._RTVNum;
		int height = ((_voy._field476 - _voy._RTVNum) * 59) / _voy._field476;
		int fullHeight = MAX(151 - height, 93);

		_graphicsManager._drawPtr->_penColor = 134;
		_graphicsManager._drawPtr->_pos = Common::Point(39, 92);

		(*_graphicsManager._vPort)->sFillBox(6, fullHeight - 92);
		if (height > 0) {
			_graphicsManager.setColor(215, 238, 238, 238);
			_eventsManager._intPtr.field38 = 1;
			_eventsManager._intPtr._hasPalette = true;

			_graphicsManager._drawPtr->_penColor = 215;
			_graphicsManager._drawPtr->_pos = Common::Point(39, fullHeight);
			(*_graphicsManager._vPort)->sFillBox(6, height);
		}
	}
}

void VoyeurEngine::flashTimeBar(){
	if (_voy._RTVNum >= 0 && (_voy._field476 - _voy._RTVNum) < 11 &&
		(_eventsManager._intPtr.field1A >= (_flashTimeVal + 15) ||
		_eventsManager._intPtr.field1A < _flashTimeVal)) {
		// Within time range
		_flashTimeVal = _eventsManager._intPtr.field1A;

		if (_flashTimeFlag)
			_graphicsManager.setColor(240, 220, 20, 20);
		else
			_graphicsManager.setColor(240, 220, 220, 220);
		
		_eventsManager._intPtr.field38 = 1;
		_eventsManager._intPtr._hasPalette = true;
		_flashTimeFlag = !_flashTimeFlag;
	}
}

void VoyeurEngine::checkPhoneCall() {
	if ((_voy._field476 - _voy._RTVNum) >= 36 && _voy._field4B8 < 5 && 
			_playStamp2 <= 151 && _playStamp2 > 146) {
		if ((_voy._switchBGNum < _checkPhoneVal || _checkPhoneVal > 180) &&
				!_soundManager.getVOCStatus()) {
			int soundIndex;
			do {
				soundIndex = getRandomNumber(4);
			} while (_voy._field4AE[soundIndex]);
			_playStamp2 = 154 + soundIndex;

			_soundManager.stopVOCPlay();
			_soundManager.startVOCPlay(_playStamp2);
			_checkPhoneVal = _voy._switchBGNum;
			++_voy._field4AE[soundIndex];
			++_voy._field4B8;
		}
	}
}

void VoyeurEngine::doEvidDisplay(int v1, int v2) {
	error("TODO: doEvidDisplay");
}

} // End of namespace Voyeur
