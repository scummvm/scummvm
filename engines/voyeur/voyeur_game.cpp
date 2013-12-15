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
	e._computerNum = _eventsManager._videoComputerNum;
	e._computerBut[0] = _eventsManager._videoComputerBut1;
	e._computerBut[1] = _voy._delaySecs;
	e._computerBut[2] = 1;
	e._computerBut[3] = _eventsManager._videoComputerBut4;
	e._dead = _eventsManager._videoDead;
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

	_voy._delaySecs = 0;
	_eventsManager._videoComputerNum = 9;
	_eventsManager._videoComputerBut1 = 0;
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
			_voy._field474 = 17;
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
			if (!checkForMurder() && _voy._field474 <= 15)
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
	error("TODO:makeViewFinder");
}

void VoyeurEngine::initIFace(){
	error("TODO: initIFace");
}

void VoyeurEngine::checkTransition(){
	error("TODO: checkTransition");
}

void VoyeurEngine::doTimeBar(int v) {
	error("TODO: doTimeBar");
}

void VoyeurEngine::flashTimeBar(){
	error("TODO: flashTimeBar");
}

void VoyeurEngine::checkPhoneCall() {
	error("TODO: checkPhoneCall");
}

} // End of namespace Voyeur
