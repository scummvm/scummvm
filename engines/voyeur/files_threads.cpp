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

#include "voyeur/files.h"
#include "voyeur/graphics.h"
#include "voyeur/voyeur.h"
#include "voyeur/staticres.h"

namespace Voyeur {

int ThreadResource::_stampFlags;
int ThreadResource::_useCount[8];
byte *ThreadResource::_threadDataPtr;
CMapResource *ThreadResource::_cmd14Pal;
int ThreadResource::_currentMouseX;
int ThreadResource::_currentMouseY;
int ThreadResource::_doAptPosX;
int ThreadResource::_doAptPosY;

void ThreadResource::init() {
	_stampFlags = 0;
	Common::fill(&_useCount[0], &_useCount[8], 0);
	_threadDataPtr = nullptr;
	_cmd14Pal = nullptr;
	_currentMouseX = 392;
	_currentMouseY = 57;
	_doAptPosX = -1;
	_doAptPosY = -1;
}

ThreadResource::ThreadResource(BoltFilesState &state, const byte *src):
		_vm(state._vm) {
	_flags = src[8];
	_ctlPtr = nullptr;
}

void ThreadResource::initThreadStruct(int idx, int id) {
	_controlIndex = -1;
	if (loadAStack(idx)) {
		_field4 = _field6 = -1;
		_threadId = id;
		_field3A = -1;
		_field3E = -1;

		doState();
	}
}

bool ThreadResource::loadAStack(int idx) {
	if (_stampFlags & 1) {
		unloadAStack(_controlIndex);
		if  (!_useCount[idx]) {
			BoltEntry &boltEntry = _vm->_stampLibPtr->boltEntry(_vm->_controlPtr->_memberIds[idx]);
			if (!boltEntry._data)
				return false;

			_vm->_controlPtr->_entries[idx] = boltEntry._data;
		}

		++_useCount[idx];
	}

	_ctlPtr = _vm->_controlPtr->_entries[idx];
	return true;
}

void ThreadResource::unloadAStack(int idx) {
	if ((_stampFlags & 1) && _useCount[idx]) {
		if (--_useCount[idx] == 0) {
			_vm->_stampLibPtr->freeBoltMember(_vm->_controlPtr->_memberIds[idx]);
		}
	}
}

bool ThreadResource::doState() {
	_flags |= 1;

	if (!getStateInfo()) 
		return false;

	getButtonsFlags();
	getField1CE();

	_vm->_glGoScene = -1;
	_vm->_glGoStack = -1;

	performOpenCard();
	if (_field40 & 1) {
		return chooseSTAMPButton(_vm->getRandomNumber(_field42 - 1));
	} else {
		return true;
	}
}

bool ThreadResource::getStateInfo() {
	_field9 = 0;
	int id = READ_LE_UINT16(_ctlPtr);

	if (id <= _threadId) {
		_field9 |= 0x80;
		return false;
	} else {
		uint32 fld = READ_LE_UINT32(_ctlPtr + 2);
		fld += _threadId << 3;
		_field46 = READ_LE_UINT32(_ctlPtr + fld + 4);
		
		fld = READ_LE_UINT32(_ctlPtr + fld);
		byte *baseP = _ctlPtr + fld;
		_field42 = READ_LE_UINT16(baseP);
		_field40 = READ_LE_UINT16(baseP + 2);
		_parseCount = READ_LE_UINT16(baseP + 4);

		_field28E = getDataOffset();
		_field28E += (READ_LE_UINT32(baseP + 6) / 2) << 1;

		_field4A = baseP + 10;
		
		getButtonsText();
		return true;
	}
}

byte *ThreadResource::getDataOffset() {
	uint32 offset = READ_LE_UINT32(_ctlPtr + 10);
	_threadDataPtr = _ctlPtr + offset;
	return _threadDataPtr;
}

void ThreadResource::getButtonsText() {
	int idx = 0;
	
	for (const byte *p = _field4A; *p != 0x49; p = getNextRecord(p)) {
		if (*p == 0xC0) {
			++p;
			if (*p++ & 0x80) {
				assert(idx < 63);
				_field8E[idx] = getRecordOffset(p);
				p += 4;
			}

			++idx;
			_field8E[idx] = NULL;
		}
	}
}

void ThreadResource::getButtonsFlags() {
	int idx = 0;
	
	for (const byte *p = _field4A; *p != 0x49; p = getNextRecord(p)) {
		if (*p == 0xC0) {
			if (*++p & 0x20)
				_field40 |= 2;

			_buttonFlags[idx] = *p++;
			_field18E[idx] = *p++;

			if (_buttonFlags[idx] & 0x80)
				p += 4;

			++idx;
		}
	}
}

void ThreadResource::getField1CE() {
	int idx = 0;
	
	for (const byte *p = _field4A; *p++ != 0x49; p = getNextRecord(p)) {
		assert(idx < 47);
		_field1CE[idx++] = getRecordOffset(p);
		_field1CE[idx] = NULL;
		p += 4;
	}
}

void ThreadResource::unloadAllStacks(VoyeurEngine *vm) {
	if (_stampFlags & 1) {
		for (int i = 0; i < 8; ++i) {
			if (_useCount[i])
				vm->_stampLibPtr->freeBoltMember(vm->_controlPtr->_memberIds[i]);
		}
	}
}

void ThreadResource::performOpenCard() {
	for (const byte *p = _field4A; *p != 0x49; p = getNextRecord(p)) {
		if (*p == 0x47) {
			cardAction(p + 1);
			return;
		}
	}
}

void ThreadResource::initUseCount() {
	Common::fill(&_useCount[0], &_useCount[8], 0);
}

const byte *ThreadResource::getRecordOffset(const byte *p) {
	uint32 recSize = READ_LE_UINT32(p) + READ_LE_UINT32(_ctlPtr + 6);
	return _ctlPtr + recSize;
}

const byte *ThreadResource::getNextRecord(const byte *p) {
	byte v = *p++;

	switch (v) {
	case 2:
	case 4:
	case 6:
	case 8:
	case 10:
		return p + 8;
	case 1:
	case 3:
	case 5:
	case 7:
	case 9:
	case 11:
	case 21:
	case 22:
	case 25:
	case 26:
		return p + 5;
	case 17:
	case 23:
	case 24:
	case 27:
	case 28:
		return p + 2;
	case 19:
	case 41:
		return p + 6;
	case 18:
	case 51:
	case 52:
		return p + 1;
	case 74:
		return p + 4;
	case 192:
		if (*p & 0x80)
			p += 4;
		return p + 2;
	default:
		return p;
	}
}

const byte *ThreadResource::getSTAMPCard(int cardId) {
	const byte *p;
	int count = 0;

	for (p = _field4A; count <= cardId && *p != 0x49; p = getNextRecord(p)) {
		if (*p == 0xC0)
			++count;
	}

	return p;
}

int ThreadResource::getStateFromID(uint32 id) {
	int count = READ_LE_UINT16(_ctlPtr);

	for (int i = 0; i < count; ++i) {
		uint32 sid = getSID(i);
		if (sid == id)
			return i;
	}

	return -1;
}

uint32 ThreadResource::getSID(int sid) {
	uint32 offset = READ_LE_UINT32(_ctlPtr + 2) + (sid << 3) + 4;
	return READ_LE_UINT32(_ctlPtr + offset);
}

void ThreadResource::doSTAMPCardAction() {
	for (const byte *p = _field4A; *p != 0x49; p = getNextRecord(p)) {
		if (*p == 0x48) {
			cardAction(p + 1);
			return;
		}
	}
}

void ThreadResource::cardAction(const byte *card) {
	_vm->_glGoScene = -1;
	_vm->_glGoStack = -1;

	// Loop to perform card commands
	while (!_vm->shouldQuit() && *card < 70 && _vm->_glGoScene == -1) {
		card = cardPerform(card);
	}
}

bool ThreadResource::chooseSTAMPButton(int buttonId) {
	_flags &= ~1;

	for (int idx = 0; idx < _field42; ++idx) {
		if (_field18E[idx] == buttonId) {
			const byte *card = getSTAMPCard(idx);
			cardAction(card);

			bool flag = true;
			while (!_vm->shouldQuit() && _vm->_glGoStack != -1 && flag) {
				doSTAMPCardAction();
				flag = goToStateID(_vm->_glGoStack, _vm->_glGoScene);
			}

			while (!_vm->shouldQuit() && _vm->_glGoScene != -1 && flag) {
				doSTAMPCardAction();
				flag = goToState(-1, _vm->_glGoScene);
			}

			return flag;
		}
	}

	return false;
}

void ThreadResource::parsePlayCommands() {
	_vm->_voy._field470 = -1;
	_vm->_voy._field468 = 0;
	_vm->_voy._field46A = 0;
	_vm->_voy._field47A = -1;
	_vm->_voy._field4E2 = -1;
	_vm->_voy._field478 &= ~8;
	_vm->_eventsManager._videoDead = -1;

	Common::fill(&_vm->_voy._arr1[0][0], &_vm->_voy._arr1[8][20], 9999);
	Common::fill(&_vm->_voy._arr2[0][0], &_vm->_voy._arr2[8][20], 0);
	Common::fill(&_vm->_voy._arr3[0][0], &_vm->_voy._arr3[3][20], 9999);
	Common::fill(&_vm->_voy._arr4[0][0], &_vm->_voy._arr4[3][20], 0);
	Common::fill(&_vm->_voy._arr5[0][0], &_vm->_voy._arr5[3][20], 9999);
	Common::fill(&_vm->_voy._arr6[0][0], &_vm->_voy._arr6[3][20], 0);
	Common::fill(&_vm->_voy._arr7[0], &_vm->_voy._arr7[20], 0);

	byte *dataP = _field28E;
	int v2, v3;
	PictureResource *pic;
	CMapResource *pal;

	for (int parseIndex = 0; parseIndex < _parseCount; ++parseIndex) {
		uint16 id = READ_LE_UINT16(dataP);
		debugC(DEBUG_BASIC, kDebugScripts, "parsePlayCommands (%d of %d) - cmd #%d",
			parseIndex + 1, _parseCount, id);
		dataP += 2;

		switch (id) {
		case 1:
			_vm->_playStamp2 = READ_LE_UINT16(dataP);
			dataP += 2;
			break;

		case 2:
			v2 = READ_LE_UINT16(dataP);

			if (v2 == 0 || READ_LE_UINT16(_vm->_controlPtr->_ptr + 4) == 0) {
				_vm->_eventsManager._videoComputerBut4 = READ_LE_UINT16(dataP + 2);
				_vm->_voy._field468 = READ_LE_UINT16(dataP + 4);
				_vm->_voy._field46A = READ_LE_UINT16(dataP + 6);

				if (_vm->_voy._RTVNum < _vm->_voy._field468 ||
						(_vm->_voy._field468 + _vm->_voy._field46A)  < _vm->_voy._RTVNum) {
					_vm->_eventsManager._videoComputerBut4 = -1;
				} else {
					_vm->_voy._vocSecondsOffset = _vm->_voy._RTVNum - _vm->_voy._field468;
					addAudioEventStart();

					assert(_vm->_eventsManager._videoComputerBut4 < 38);
					_vm->_graphicsManager._backgroundPage = _vm->_bVoy->boltEntry(
						0x7F00 + BLIND_TABLE[_vm->_eventsManager._videoComputerBut4])._picResource;
					_vm->_graphicsManager._backColors = _vm->_bVoy->boltEntry(0x7F01 + 
						BLIND_TABLE[_vm->_eventsManager._videoComputerBut4])._cMapResource;

					(*_vm->_graphicsManager._vPort)->setupViewPort();
					_vm->_graphicsManager._backColors->startFade();
					(*_vm->_graphicsManager._vPort)->_flags |= 8;
					_vm->_graphicsManager.flipPage();
					_vm->_eventsManager.sWaitFlip();

					while (!_vm->shouldQuit() && (_vm->_eventsManager._fadeStatus & 1))
						_vm->_eventsManager.delay(1);

					_vm->_voy._field478 = -2;
					_vm->_soundManager.setVOCOffset(_vm->_voy._vocSecondsOffset * 11025);
					Common::String filename = _vm->_soundManager.getVOCFileName(
						_vm->_eventsManager._videoComputerBut4 + 159);
					_vm->_soundManager.startVOCPlay(filename);
					_vm->_voy._field478 |= 16;
					_vm->_eventsManager.startCursorBlink();

					while (!_vm->shouldQuit() && !_vm->_voy._incriminate && 
							_vm->_soundManager.getVOCStatus())
							_vm->_eventsManager.delay(1);

					_vm->_voy._field478 |= 1;
					_vm->_soundManager.stopVOCPlay();
					addAudioEventEnd();
					_vm->_eventsManager.incrementTime(1);
					_vm->_eventsManager.incrementTime(1);

					_vm->_bVoy->freeBoltGroup(0x7F00);
					_vm->_voy._field478 = -17;
					_vm->_eventsManager._videoComputerBut4 = -1;
					_vm->_voy._field470 = 129;
					parseIndex = 999;
				}				
			} 
			
			dataP += 8;
			break;

		case 3:
			v2 = READ_LE_UINT16(dataP);

			if (v2 == 0 || READ_LE_UINT16(_vm->_controlPtr->_ptr + 4) == 0) {
				_vm->_eventsManager._videoComputerBut4 = READ_LE_UINT16(dataP + 2);
				_vm->_voy._field468 = READ_LE_UINT16(dataP + 4);
				_vm->_voy._field46A = READ_LE_UINT16(dataP + 6);

				if (_vm->_voy._RTVNum < _vm->_voy._field468 ||
						(_vm->_voy._field468 + _vm->_voy._field46A)  < _vm->_voy._RTVNum) {
					_vm->_eventsManager._videoComputerBut4 = -1;
				} else {
					_vm->_voy._vocSecondsOffset = _vm->_voy._RTVNum - _vm->_voy._field468;
					addAudioEventStart();
					_vm->_voy._field478 &= ~1;
					_vm->_voy._field478 |= 0x10;
					_vm->playAVideo(_vm->_eventsManager._videoComputerBut4);

					_vm->_voy._field478 &= ~0x10;
					_vm->_voy._field478 |= 1;
					addVideoEventEnd();
					_vm->_eventsManager.incrementTime(1);
				
					_vm->_eventsManager._videoComputerBut4 = -1;
					_vm->_playStamp1 = -1;

					if (_vm->_eventsManager._videoDead != -1) {
						_vm->_bVoy->freeBoltGroup(0xE00);
						_vm->_eventsManager._videoDead = -1;
						(*_vm->_graphicsManager._vPort)->_flags |= 8;

						_vm->_graphicsManager.flipPage();
						_vm->_eventsManager.sWaitFlip();
					}

					_vm->_eventsManager._videoDead = -1;
					if (_field42 == 2 && _vm->_voy._incriminate == 0) {
						_vm->_voy._field470 = 132;
						parseIndex = 999;
					} else {
						_vm->_voy._field470 = 129;
					}
				}
			}

			dataP += 8;
			break;

		case 4:
		case 22:
			_vm->_eventsManager._videoComputerBut4 = READ_LE_UINT16(dataP) - 1;
			dataP += 2;

			if (id == 22) {
				int resolveIndex = READ_LE_UINT16(dataP);
				dataP += 2;
				_vm->_playStamp1 = _vm->_resolvePtr[resolveIndex];
			}

			_vm->_voy._vocSecondsOffset = 0;
			_vm->_voy._field468 = _vm->_voy._RTVNum;
			_vm->_voy._field478 &= ~0x11;
			_vm->playAVideo(_vm->_eventsManager._videoComputerBut4);
			_vm->_voy._field478 |= 1;

			if (id != 2) {
				_vm->_eventsManager._videoComputerBut4 = -1;
				parseIndex = 999;
			} else {
				// TODO: Double-check this
				int count = _vm->_bVoy->getBoltGroup(_vm->_playStamp1)->_entries.size();
				_vm->_soundManager.stopVOCPlay();
				_vm->_eventsManager.getMouseInfo();

				for (int i = 0; i < count; ++i) {
					pic = _vm->_bVoy->boltEntry(_vm->_playStamp1 + i * 2)._picResource;
					pal = _vm->_bVoy->boltEntry(_vm->_playStamp1 + i * 2 + 1)._cMapResource;

					(*_vm->_graphicsManager._vPort)->setupViewPort(pic);
					pal->startFade();

					(*_vm->_graphicsManager._vPort)->_flags |= 8;
					_vm->_graphicsManager.flipPage();
					_vm->_eventsManager.sWaitFlip();

					while (!_vm->shouldQuit() && (_vm->_eventsManager._fadeStatus & 1))
						_vm->_eventsManager.delay(1);

					if (i > 0) {
						_vm->_bVoy->freeBoltMember(_vm->_playStamp1 + i * 2);
						_vm->_bVoy->freeBoltMember(_vm->_playStamp1 + i * 2 + 1);
					}

					Common::String file = Common::String::format("news%d.voc", i + 1);
					_vm->_soundManager.startVOCPlay(file);

					while (!_vm->shouldQuit() && !_vm->_voy._incriminate &&
							_vm->_soundManager.getVOCStatus()) {
						_vm->_eventsManager.delay(1);
						_vm->_eventsManager.getMouseInfo();
					}

					_vm->_soundManager.stopVOCPlay();

					if (i == (count - 1))
						_vm->_eventsManager.delay(480);

					if (_vm->shouldQuit() || _vm->_voy._incriminate)
						break;
				}

				_vm->_bVoy->freeBoltGroup(_vm->_playStamp1);
				_vm->_playStamp1 = -1;
				_vm->_eventsManager._videoComputerBut4 = -1;
				parseIndex = 999;
			}
			break;			

		case 5:
			v2 = READ_LE_UINT16(dataP);
			if (v2 == 0 || READ_LE_UINT16(_vm->_controlPtr->_ptr + 4) == 0) {
				_vm->_voy._field470 = 5;
				int count = READ_LE_UINT16(dataP + 2);
				_vm->_voy._field476 = READ_LE_UINT16(dataP + 4);

				if (_vm->_voy._field474 != count) {
					if (_vm->_voy._field474 > 1)
						_vm->_voy._field478 &= ~0x100;

					_vm->_voy._field474 = count;
					_vm->_eventsManager._videoComputerBut1 = LEVEL_M[count - 1];
					_vm->_eventsManager._videoComputerNum = LEVEL_H[count - 1];
					//_vm->_v2A0A2 = 0;
					_vm->_voy._RTVNum = 0;
					_vm->_voy._RTANum = 255;
				}

				_vm->_voy._delaySecs = (_vm->_voy._field474 == 6) ? 1 : 0;
			}

			dataP += 6;
			break;

		case 6:
			_vm->_voy._field470 = 6;
			v2 = READ_LE_UINT16(dataP);
			_vm->_playStamp1 = _vm->_resolvePtr[v2];
			dataP += 2;
			break;

		case 7:
			v2 = READ_LE_UINT16(dataP);
			v3 = READ_LE_UINT16(dataP + 2) - 1;

			if (v2 == 0 || READ_LE_UINT16(_vm->_controlPtr->_ptr + 4) == 0) {
				int idx = 0;
				while (_vm->_voy._arr1[idx][v3] != 9999)
					++idx;

				v2 = READ_LE_UINT16(dataP + 4);
				_vm->_voy._arr1[idx][v3] = v2;
				_vm->_voy._arr2[idx][v3] = v2 + READ_LE_UINT16(dataP + 6) - 2;
			}

			dataP += 8;
			break;

		case 8:
 			v2 = READ_LE_UINT16(dataP);
			v3 = READ_LE_UINT16(dataP + 2) - 1;

			if (v2 == 0 || READ_LE_UINT16(_vm->_controlPtr->_ptr + 4) == 0) {
				int idx = 0;
				while (_vm->_voy._arr3[idx][v3] != 9999)
					++idx;

				v2 = READ_LE_UINT16(dataP + 4);
				_vm->_voy._arr3[idx][v3] = v2;
				_vm->_voy._arr4[idx][v3] = v2 + READ_LE_UINT16(dataP + 6) - 2;
			}

			dataP += 8;
			break;

		case 9:
			v2 = READ_LE_UINT16(dataP);
			v3 = READ_LE_UINT16(dataP + 2) - 1;

			if (v2 == 0 || READ_LE_UINT16(_vm->_controlPtr->_ptr + 4) == 0) {
				int idx = 0;
				while (_vm->_voy._arr5[idx][v3] != 9999)
					++idx;

				v2 = READ_LE_UINT16(dataP + 4);
				_vm->_voy._arr5[idx][v3] = v2;
				_vm->_voy._arr6[idx][v3] = v2 + READ_LE_UINT16(dataP + 6) - 2;
			}

			dataP += 8;
			break;

		case 10:
			if (_vm->_iForceDeath == -1) {
				int randomVal;
				do {
					randomVal = _vm->getRandomNumber(3) + 1;
				} while (randomVal == _vm->_voy._field4380);

				_vm->_voy._field4380 = randomVal;
				WRITE_LE_UINT16(_vm->_controlPtr->_ptr + 4, randomVal);
			} else {
				_vm->_voy._field4380 = _vm->_iForceDeath;
				WRITE_LE_UINT16(_vm->_controlPtr->_ptr + 4, _vm->_iForceDeath);
			}

			_vm->saveLastInplay();
			break;

		case 11:
			_vm->_voy._field478 = 2;
			break;

		case 12:
			v2 = READ_LE_UINT16(dataP);

			if (v2 == 0 || READ_LE_UINT16(_vm->_controlPtr->_ptr + 4) == 0) {
				_vm->_voy._field47A = _vm->_resolvePtr[READ_LE_UINT16(dataP + 2)];
				_vm->_voy._arr7[READ_LE_UINT16(dataP + 4) - 1] = 1;
			}

			dataP += 6;
			break;

		case 13:
			v2 = READ_LE_UINT16(dataP);

			if (v2 == 0 || READ_LE_UINT16(_vm->_controlPtr->_ptr + 4) == 0) {
				_vm->_voy._field4E2 = READ_LE_UINT16(dataP + 2);
				_vm->_voy._field4EC = READ_LE_UINT16(dataP + 4);
				_vm->_voy._field4EE = READ_LE_UINT16(dataP + 6);

				_vm->_voy._rect4E4.left = COMP_BUT_TABLE[_vm->_voy._field4E2 * 4];
				_vm->_voy._rect4E4.top = COMP_BUT_TABLE[_vm->_voy._field4E2 * 4 + 1];
				_vm->_voy._rect4E4.right = COMP_BUT_TABLE[_vm->_voy._field4E2 * 4 + 2];
				_vm->_voy._rect4E4.bottom = COMP_BUT_TABLE[_vm->_voy._field4E2 * 4 + 3];
			}

			dataP += 8;
			break;

		case 14:
			_vm->_playStamp1 = 2048;
			_vm->_voy._field470 = 130;
			break;

		case 15:
			_vm->_playStamp1 = (_vm->_voy._field4382 - 1) * 8 + 0x7700;
			_vm->_voy._field47A = ((READ_LE_UINT16(_vm->_controlPtr->_ptr + 4) 
				- 1) << 8) + 0x7B00;

			pic = _vm->_bVoy->boltEntry(_vm->_playStamp1)._picResource;
			_cmd14Pal = _vm->_bVoy->boltEntry(_vm->_playStamp1 + 1)._cMapResource;

			(*_vm->_graphicsManager._vPort)->setupViewPort(pic);
			_cmd14Pal->startFade();

			(*_vm->_graphicsManager._vPort)->_flags |= 8;
			_vm->_graphicsManager.flipPage();
			_vm->_eventsManager.sWaitFlip();

			while (!_vm->shouldQuit() && (_vm->_eventsManager._fadeStatus & 1))
				_vm->_eventsManager.delay(1);
			_vm->_eventsManager.getMouseInfo();

			for (int idx = 1; idx < 4; ++idx) {
				if (idx == 3) {
					pic = _vm->_bVoy->boltEntry(_vm->_voy._field47A)._picResource;
					_cmd14Pal = _vm->_bVoy->boltEntry(_vm->_voy._field47A + 1)._cMapResource;
				} else {
					pic = _vm->_bVoy->boltEntry(_vm->_playStamp1 + idx * 2)._picResource;
					_cmd14Pal = _vm->_bVoy->boltEntry(_vm->_playStamp1 + idx * 2 + 1)._cMapResource;
				}

				(*_vm->_graphicsManager._vPort)->setupViewPort(pic);
				_cmd14Pal->startFade();

				(*_vm->_graphicsManager._vPort)->_flags |= 8;
				_vm->_graphicsManager.flipPage();
				_vm->_eventsManager.sWaitFlip();

				while (!_vm->shouldQuit() && (_vm->_eventsManager._fadeStatus & 1))
					_vm->_eventsManager.delay(1);

				_vm->_bVoy->freeBoltMember(_vm->_playStamp1 + (idx - 1) * 2);
				_vm->_bVoy->freeBoltMember(_vm->_playStamp1 + (idx - 1) * 2 + 1);

				Common::String fname = Common::String::format("news%d.voc", idx);

				while (!_vm->shouldQuit() && !_vm->_voy._incriminate && 
						_vm->_soundManager.getVOCStatus())
					_vm->_eventsManager.delay(1);

				_vm->_soundManager.stopVOCPlay();
				if (idx == 3)
					_vm->_eventsManager.delay(3);

				if (_vm->shouldQuit() || _vm->_voy._incriminate)
					break;
			}

			_vm->_bVoy->freeBoltGroup(_vm->_playStamp1);
			_vm->_bVoy->freeBoltGroup(_vm->_voy._field47A);
			_vm->_playStamp1 = -1;
			_vm->_voy._field47A = -1;
			break;

		case 16:
			_vm->_voy._field470 = 16;
			break;

		case 17:
			_vm->_voy._field470 = 17;
			break;

		case 18:
			v2 = READ_LE_UINT16(dataP);
			v3 = READ_LE_UINT16(dataP + 2);

			if (v2 == 0 || READ_LE_UINT16(_vm->_controlPtr->_ptr + 4) == 0)
				_vm->_voy._field4F2 = v3;
			
			dataP += 4;
			break;

		case 19:
			_vm->_voy._field472 = 140;
			loadTheApt();
			_vm->_voy._field472 = 141;
			freeTheApt();
			break;

		case 20:
			_vm->_voy._field472 = -1;
			loadTheApt();
			_vm->_voy._field472 = 141;
			freeTheApt();
			break;

		case 21:
			_vm->_voy._field472 = -1;
			loadTheApt();
			_vm->_voy._field472 = 140;
			freeTheApt();
			break;

		case 23:
			_vm->_voy._field474 = 17;
			_vm->_voy._field472 = -1;
			loadTheApt();
			_vm->_voy._field472 = 144;
			freeTheApt();
			break;

		default:
			break;
		}
	}
}

const byte *ThreadResource::cardPerform(const byte *card) {
	const byte *p2;
	byte *pDest;

	uint16 id = *card++;
	int varD = 5;
	uint32 v2;
	int v3;
	byte bVal;
	uint32 idx1, idx2;

	switch (id) {
	case 1:
		v2 = READ_LE_UINT32(card);
		card += 4;
		WRITE_LE_UINT32(_vm->_controlPtr->_ptr + (*card << 2), v2);
		++card;
		break;

	case 2:
		v2 = READ_LE_UINT32(_vm->_controlPtr->_ptr + (*card++ << 2)),
		WRITE_LE_UINT32(_vm->_controlPtr->_ptr + (*card++ << 2), v2);
		break;

	case 3:
		v2 = READ_LE_UINT32(card);
		card += 4;
		WRITE_LE_UINT32(_vm->_controlPtr->_ptr + (*card++ << 2), v2);
		break;

	case 4:
		v2 = READ_LE_UINT32(_vm->_controlPtr->_ptr + (*card++ << 2));
		WRITE_LE_UINT32(_vm->_controlPtr->_ptr + (*card++ << 2), v2);
		break;

	case 5:
		v2 = READ_LE_UINT32(card);
		card += 4;
		pDest = _vm->_controlPtr->_ptr + (*card++ << 2);
		WRITE_LE_UINT32(pDest, READ_LE_UINT32(pDest) - v2);
		break;

	case 6:
		idx1 = *card++;
		idx2 = *card++;

		v2 = READ_LE_UINT32(_vm->_controlPtr->_ptr + idx2);
		pDest = _vm->_controlPtr->_ptr + idx1;
		WRITE_LE_UINT32(pDest, READ_LE_UINT32(pDest) - v2);
		break;

	case 7:
		v3 = *card++;
		v2 = READ_LE_UINT32(card);
		card += 4;
		pDest = _vm->_controlPtr->_ptr + (v3 << 2);
		WRITE_LE_UINT32(pDest, READ_LE_UINT32(pDest) * v2);
		break;

	case 8:
		idx1 = *card++;
		idx2 = *card++;

		pDest = _vm->_controlPtr->_ptr + (idx1 << 2);
		p2 = _vm->_controlPtr->_ptr + (idx2 << 2);
		WRITE_LE_UINT32(pDest, READ_LE_UINT32(pDest) * READ_LE_UINT32(p2));
		break;

	case 9:
		idx1 = *card++;
		v2 = READ_LE_UINT32(card);
		card += 4;

		pDest = _vm->_controlPtr->_ptr + (idx1 << 2);
		WRITE_LE_UINT32(pDest, READ_LE_UINT32(pDest) / v2);
		break;

	case 10:
		idx1 = *card++;
		idx2 = *card++;

		pDest = _vm->_controlPtr->_ptr + (idx1 << 2);
		p2 = _vm->_controlPtr->_ptr + (idx2 << 2);
		WRITE_LE_UINT32(pDest, READ_LE_UINT32(pDest) / READ_LE_UINT32(p2));
		break;
	
	case 11:
		v2 = READ_LE_UINT32(card);
		card += 4;
		v2 = _vm->getRandomNumber(v2 - 1) + 1;
		WRITE_LE_UINT32(_vm->_controlPtr->_ptr + (*card++ << 2), v2);
		break;

	case 17:
		_vm->_glGoScene = READ_LE_UINT16(card);
		card += 2;
		_vm->_glGoStack = -1;
		break;

	case 18:
		v2 = READ_LE_UINT32(_vm->_controlPtr->_ptr + (*card++ << 2));
		_vm->_glGoScene = getStateFromID(v2);
		break;

	case 19:
		_vm->_glGoScene = READ_LE_UINT32(card);
		card += 4;
		_vm->_glGoStack = READ_LE_UINT16(card);
		card += 2;
		break;

	case 22:
	case 23:
	case 26:
	case 27:
		varD -= 3;	
		// Deliberate fall-through

	case 20:
	case 21:
	case 24:
	case 25:
		bVal = card[varD];
		if (bVal == 61) {
			if (cardPerform2(card, id)) {
				card += varD;
				while (*card != 30 && *card != 29)
					card = cardPerform(card);

				if (*card == 29) {
					int count = 1;
					while (count > 0) {
						card = getNextRecord(card);
						if (*card == 30)
							--count;
						if (*card >= 21 && *card <= 28)
							++count;
					}
				}
			} else {
				card += varD;
				int count = 1;
				while (count > 0) {
					card = getNextRecord(card);
					if (*card == 29 || *card == 30)
						--count;
					if (*card < 21 || *card > 28)
						continue;

					const byte *nextP = getNextRecord(card + 2);
					if (*nextP == 61)
						++count;
				}
			}
			
			++card;
		} else {
			if (cardPerform2(card, id)) {
				card += varD;
				card = cardPerform(card);
				while (*card++ != 61) ;		
			} else {
				card += varD;
				while (*card != 61 && *card != 29)
					++card;
			}
		}
		break;

	case 41:
		bVal = *card++;
		assert(bVal < 8);
		_fieldA[bVal] = READ_LE_UINT32(card);
		card += 4;

		_field2A[bVal] = READ_LE_UINT16(card);
		card += 2;
	
	case 45:
		_field3A = _field46;
		_field3E = _controlIndex;
		break;

	case 46:
		_vm->_glGoScene = _field3A;
		_vm->_glGoStack = _field3E;
		_field3A = -1;
		_field3E = -1;
		break;

	case 51:
		setButtonFlag(READ_LE_UINT16(card), 64);
		break;

	case 52:
		clearButtonFlag(READ_LE_UINT16(card), 64);
		break;

	default:
		break;
	}

	return card;
}

bool ThreadResource::cardPerform2(const byte *pSrc, int cardCmdId) {
	uint32 vLong, vLong2;

	switch (cardCmdId) {
	case 21:
		vLong = READ_LE_UINT32(pSrc + 1);
		return READ_LE_UINT32(_vm->_controlPtr->_ptr + (*pSrc << 2)) == vLong;

	case 22:
		vLong = READ_LE_UINT32(pSrc + 1);
		return READ_LE_UINT32(_vm->_controlPtr->_ptr + (*pSrc << 2)) != vLong;

	case 23:
		vLong = READ_LE_UINT32(_vm->_controlPtr->_ptr + (*pSrc << 2));
		vLong2 = READ_LE_UINT32(_vm->_controlPtr->_ptr + (*(pSrc + 1) << 2));
		return vLong == vLong2;

	case 24:
		vLong = READ_LE_UINT32(_vm->_controlPtr->_ptr + (*pSrc << 2));
		vLong2 = READ_LE_UINT32(_vm->_controlPtr->_ptr + (*(pSrc + 1) << 2));
		return vLong != vLong2;

	case 25:
		vLong = READ_LE_UINT32(_vm->_controlPtr->_ptr + (*pSrc << 2)); 
		vLong2 = READ_LE_UINT32(pSrc + 1);
		return vLong < vLong2;

	case 26:
		vLong = READ_LE_UINT32(_vm->_controlPtr->_ptr + (*pSrc << 2)); 
		vLong2 = READ_LE_UINT32(pSrc + 1);
		return vLong > vLong2;

	case 27:
		vLong = READ_LE_UINT32(_vm->_controlPtr->_ptr + (*pSrc << 2));
		vLong2 = READ_LE_UINT32(_vm->_controlPtr->_ptr + (*(pSrc + 1) << 2));
		return vLong < vLong2;

	case 28:
		vLong = READ_LE_UINT32(_vm->_controlPtr->_ptr + (*pSrc << 2));
		vLong2 = READ_LE_UINT32(_vm->_controlPtr->_ptr + (*(pSrc + 1) << 2));
		return vLong > vLong2;

	default:
		return false;
	}
}	

int ThreadResource::doApt() {
	int varC = -1;
	loadTheApt();

	_vm->_playStamp2 = 151;
	byte *dataP = _vm->_bVoy->memberAddr(_vm->_playStamp1);
	PictureResource *srcPic = _vm->_bVoy->boltEntry(_vm->_playStamp1 + 3)._picResource;
	_vm->_eventsManager.getMouseInfo();

	if (_doAptPosX == -1) {
		_doAptPosX = READ_LE_UINT16(dataP + 18) + 16;
		_doAptPosY = READ_LE_UINT16(dataP + 20) + 16;
		_vm->_playStamp2 = 153;
	}

	if (_vm->_voy._field470 == 16) {
		WRITE_LE_UINT16(dataP + 2, 999);
		WRITE_LE_UINT16(dataP + 26, 999);
		_doAptPosX = READ_LE_UINT16(dataP + 34) + 28;
		_doAptPosY = READ_LE_UINT16(dataP + 36) + 28;
	}

	_vm->_eventsManager.setMousePos(Common::Point(_doAptPosX, _doAptPosY));
	_vm->_soundManager.startVOCPlay(_vm->_soundManager.getVOCFileName(_vm->_playStamp2));
	_vm->_playStamp2 = 151;

	_vm->_graphicsManager.setColor(129, 82, 82, 82);
	_vm->_graphicsManager.setColor(130, 112, 112, 112);
	_vm->_graphicsManager.setColor(131, 215, 215, 215);
	_vm->_graphicsManager.setColor(132, 235, 235, 235);

	_vm->_eventsManager._intPtr.field38 = true;
	_vm->_eventsManager._intPtr._hasPalette = true;

	int result;
	Common::Point pt;
	PictureResource *pic;
	do {
		_vm->_eventsManager.getMouseInfo();
		if (!_vm->_soundManager.getVOCStatus()) {
			_vm->_playStamp2 = _vm->getRandomNumber(4) + 151;
			_vm->_soundManager.startVOCPlay(_vm->_soundManager.getVOCFileName(_vm->_playStamp2));
		}

		result = -1;
		pt = _vm->_eventsManager.getMousePos();
		for (int idx = 0; idx < READ_LE_UINT16(dataP); ++idx) {
			if (READ_LE_UINT16(dataP + idx * 8 + 2) <= pt.x &&
				READ_LE_UINT16(dataP + idx * 8 + 6) >= pt.x &&
				READ_LE_UINT16(dataP + idx * 8 + 4) <= pt.y &&
				READ_LE_UINT16(dataP + idx * 8 + 8) >= pt.y) {
				// Found entry
				result = idx;

				if (idx != varC) {
					if ((_vm->_voy._field478 & 0x100) && (result == 2))
						result = 5;

					pic = _vm->_bVoy->boltEntry(_vm->_playStamp1 + 
						result + 6)._picResource;
					_vm->_graphicsManager.sDrawPic(pic, *_vm->_graphicsManager._vPort,
						Common::Point(106, 200));
				}

				break;
			}
		}

		pic = _vm->_bVoy->boltEntry((result == -1) ? _vm->_playStamp1 + 2 :
			_vm->_playStamp1 + 3)._picResource;
		_vm->_graphicsManager.sDrawPic(pic, *_vm->_graphicsManager._vPort, pt);

		(*_vm->_graphicsManager._vPort)->_flags |= 8;
		_vm->_graphicsManager.flipPage();
		_vm->_eventsManager.sWaitFlip();

	} while (!_vm->shouldQuit() && !_vm->_voy._lastInplay && result == -1);

	pt = _vm->_eventsManager.getMousePos();
	_doAptPosX = pt.x;
	_doAptPosY = pt.y;

	switch (result) {
	case 0:
		_vm->_voy._field472 = 140;
		break;
	case 1:
		_vm->_voy._field472 = 143;
		break;
	case 2:
		_vm->_voy._field472 = 142;
	case 5:
		_vm->_voy._field472 = 141;
		break;
	default:
		_vm->_voy._field472 = -1;
		break;
	}

	freeTheApt();
	if (_vm->_voy._field474 == 1 && result == 0)
		_vm->checkTransition();

	if (!result)
		_vm->makeViewFinder();

	return result;
}

void ThreadResource::doRoom() {
	warning("TODO: doRoom");
}

int ThreadResource::doInterface() {
	int varA = -1;
	int var8 = 0;
	PictureResource *pic;
	Common::Point pt;

	_vm->_voy._field478 |= 1;
	if (_vm->_voy._field46E) {
		_vm->_voy._field46E = 0;
		return -2;
	}

	_vm->_voy._field478 &= ~0x100;
	_vm->_playStamp1 = -1;
	_vm->_eventsManager._intPtr.field1E = 1;
	_vm->_eventsManager._intPtr.field1A = 0;

	if (_vm->_voy._RTVNum >= _vm->_voy._field476 || _vm->_voy._RTVNum < 0)
		_vm->_voy._RTVNum = _vm->_voy._field476 - 1;

	if (_vm->_voy._field474 < 15 && (_vm->_voy._field476 - 3) < _vm->_voy._RTVNum) {
		_vm->_voy._RTVNum = _vm->_voy._field476;
		_vm->makeViewFinder();

		_vm->_eventsManager.setMousePos(Common::Point(_currentMouseX, _currentMouseY));
		_vm->initIFace();
		_vm->_voy._RTVNum = _vm->_voy._field476 - 4;
		_vm->_voy._field478 &= ~1;

		while (!_vm->shouldQuit() && _vm->_voy._RTVNum < _vm->_voy._field476) {
			_vm->flashTimeBar();
		}

		_vm->_voy._field478 = 1;
		chooseSTAMPButton(20);
		parsePlayCommands();
	}

	_vm->checkTransition();
	_vm->makeViewFinder();
	_vm->_eventsManager.getMouseInfo();
	_vm->_eventsManager.setMousePos(Common::Point(_currentMouseX, _currentMouseY));
	_vm->initIFace();

	byte *dataP = _vm->_bVoy->memberAddr(_vm->_playStamp1);
	_vm->_playStamp2 = 151 - _vm->getRandomNumber(5);
	_vm->_voy._vocSecondsOffset = _vm->getRandomNumber(29);

	Common::String fname = _vm->_soundManager.getVOCFileName(_vm->_playStamp2);
	_vm->_soundManager.startVOCPlay(fname);
	_vm->_eventsManager.getMouseInfo();
	_vm->_eventsManager.setMousePos(Common::Point(_currentMouseX, _currentMouseY));
	
	_vm->_graphicsManager.setColor(240, 220, 220, 220);
	_vm->_eventsManager._intPtr.field38 = true;
	_vm->_eventsManager._intPtr._hasPalette = true;
	_vm->_voy._field478 &= ~1;

	do {
		_vm->doTimeBar(1);
		_vm->_eventsManager.getMouseInfo();

		pt = _vm->_eventsManager.getMousePos();
		if (pt.x != _currentMouseX || pt.y != _currentMouseY || var8 != varA) {
			varA = var8;
			_vm->_graphicsManager.doScroll(pt);

			_currentMouseX = pt.x;
			_currentMouseY = pt.y;
		}

		_vm->checkPhoneCall();
		if (!_vm->_soundManager.getVOCStatus()) {
			_vm->_playStamp2 = 151 - _vm->getRandomNumber(5);
			_vm->_soundManager.startVOCPlay(_vm->_soundManager.getVOCFileName(_vm->_playStamp2));
		}

		pt = _vm->_eventsManager.getMousePos() + Common::Point(120, 75);

		for (int idx = 0; idx < READ_LE_UINT16(dataP); ++idx) {
			if (READ_LE_UINT16(dataP + (idx * 8 + 2)) <= pt.x &&
					READ_LE_UINT16(dataP + (idx * 8 + 6)) >= pt.x &&
					READ_LE_UINT16(dataP + (idx * 8 + 4)) <= pt.y &&
					READ_LE_UINT16(dataP + (idx * 8 + 8)) >= pt.y) {
				// Rect check done
				for (int arrIndex = 0; arrIndex < 3; ++arrIndex) {
					if (_vm->_voy._arr3[arrIndex][idx] <= _vm->_voy._RTVNum &&
							_vm->_voy._arr4[arrIndex][idx] > _vm->_voy._RTVNum) {
						// Draw the picture
						pic = _vm->_bVoy->boltEntry(276)._picResource;
						_vm->_graphicsManager.sDrawPic(pic, *_vm->_graphicsManager._vPort, 
							Common::Point(178, 108));
						var8 = idx;
					}

					if (_vm->_voy._arr5[arrIndex][idx] <= _vm->_voy._RTVNum &&
							_vm->_voy._arr6[idx][idx] > _vm->_voy._RTVNum) {
						// Draw the picture
						pic = _vm->_bVoy->boltEntry(277)._picResource;
						_vm->_graphicsManager.sDrawPic(pic, *_vm->_graphicsManager._vPort, 
							Common::Point(178, 108));
						var8 = idx;
					}
				}

				for (int arrIndex = 0; arrIndex < 3; ++arrIndex) {
					if (_vm->_voy._arr1[arrIndex][idx] <= _vm->_voy._RTVNum &&
							_vm->_voy._arr2[arrIndex][idx] > _vm->_voy._RTVNum) {
						// Draw the picture
						pic = _vm->_bVoy->boltEntry(375)._picResource;
						_vm->_graphicsManager.sDrawPic(pic, *_vm->_graphicsManager._vPort, 
							Common::Point(178, 108));
						var8 = idx;
					}
				}
			}
		}

		if (var8 == -1) {
			// Draw the default picture
			pic = _vm->_bVoy->boltEntry(274)._picResource;
			_vm->_graphicsManager.sDrawPic(pic, *_vm->_graphicsManager._vPort, 
				Common::Point(178, 108));
		}

		if (_vm->_voy._RTVNum & 2) {
			_vm->_graphicsManager.drawANumber(*_vm->_graphicsManager._vPort, 
				10 / _vm->_eventsManager._videoComputerBut1, 0x1900BE);
			_vm->_graphicsManager.drawANumber(*_vm->_graphicsManager._vPort, 
				10 % _vm->_eventsManager._videoComputerBut1, 0x1900BE);

			if (_vm->_voy._RTANum & 4) {
				int v = 10 / _vm->_eventsManager._videoComputerNum;
				_vm->_graphicsManager.drawANumber(*_vm->_graphicsManager._vPort, 
					v == 0 ? 10 : v, 0x1900BE);
				_vm->_graphicsManager.drawANumber(*_vm->_graphicsManager._vPort, 
					_vm->_eventsManager._videoComputerNum % 10, 0x1900AC);

				pic = _vm->_bVoy->boltEntry(274)._picResource;
				_vm->_graphicsManager.sDrawPic(pic, *_vm->_graphicsManager._vPort, 
					Common::Point(215, 27));
			}
		}

		_vm->_voy._RTANum = 0;
		(*_vm->_graphicsManager._vPort)->_flags |= 8;
		_vm->_graphicsManager.flipPage();
		_vm->_eventsManager.sWaitFlip();

		pt = _vm->_eventsManager.getMousePos();
		if ((_vm->_voy._field476 <= _vm->_voy._RTVNum) || ((_vm->_voy._field478 & 0x80) &&
				(_vm->_voy._fadeFunc != NULL) && (pt.x == 0))) {
			_vm->_eventsManager.getMouseInfo();

			if (_vm->_voy._field474 == 15) {
				var8 = 20;
				_vm->_voy._field474 = 17;
				_vm->_soundManager.stopVOCPlay();
				_vm->checkTransition();
				_vm->_voy._lastInplay = true;
			} else {
				_vm->_voy._field478 = 1;
				_currentMouseX = pt.x;
				_currentMouseY = pt.y;

				chooseSTAMPButton(20);
				parsePlayCommands();
				_vm->checkTransition();
				_vm->makeViewFinder();

				_vm->_eventsManager.setMousePos(Common::Point(_currentMouseX, _currentMouseY));
				_vm->initIFace();
				
				dataP = _vm->_bVoy->memberAddr(_vm->_playStamp1 + 1);
				_vm->_eventsManager.getMouseInfo();
				_vm->_eventsManager.setMousePos(Common::Point(_currentMouseX, _currentMouseY));

				_vm->_voy._field478 &= ~2;
				_vm->_eventsManager._intPtr.field1E = 1;
				_vm->_eventsManager._intPtr.field1A = 0;
			}
		}
	} while (!_vm->_voy._fadeFunc && !_vm->shouldQuit() && 
		(!_vm->_voy._lastInplay || var8 == -1));

	_vm->_voy._field478 |= 1;
	_vm->_bVoy->freeBoltGroup(_vm->_playStamp1);
	if (_vm->_playStamp2 != -1)
		_vm->_soundManager.stopVOCPlay();

	return !_vm->_voy._fadeFunc ? var8 : -2;
}

void ThreadResource::addAudioEventStart() {
	_vm->_voy._events.push_back(VoyeurEvent(_vm->_eventsManager._videoComputerNum,
		_vm->_eventsManager._videoComputerBut1, _vm->_voy._delaySecs, 2, 
		_vm->_eventsManager._videoComputerBut4, _vm->_voy._vocSecondsOffset, 
		_vm->_eventsManager._videoDead));
}

void ThreadResource::addAudioEventEnd() {
	error("TODO: addAudioEventEnd");
}

void ThreadResource::addVideoEventEnd() {
	error("TODO: addVideoEventEnd");
}

bool ThreadResource::goToStateID(int stackId, int sceneId) {
	debugC(DEBUG_BASIC, kDebugScripts, "goToStateID - %d, %d", stackId, sceneId);

	// Save current stack
	savePrevious();

	if (_controlIndex == stackId || stackId == -1 || loadAStack(stackId)) {
		// Now in the correct state
		_threadId = getStateFromID(sceneId);

		if (_threadId != -1) {
			return doState();
		} else {
			_threadId = _field4;
			_controlIndex = _field6;
		}
	}

	return false;
}

bool ThreadResource::goToState(int stackId, int sceneId) {
	debugC(DEBUG_BASIC, kDebugScripts, "goToState - %d, %d", stackId, sceneId);

	savePrevious();
	if (stackId == -1 || loadAStack(stackId)) {
		if (sceneId != -1)
			_threadId = sceneId;

		return doState();
	} else {
		return false;
	}
}

void ThreadResource::savePrevious() {
	if (_field4 == _threadId && _controlIndex == _field6) {
		_flags &= ~1;
	} else {
		_flags |= 1;
		_field4 = _threadId;
		_field6 = _controlIndex;
	}
}

void ThreadResource::setButtonFlag(int idx, byte bits) {
	_buttonFlags[idx] |= bits;
}

void ThreadResource::clearButtonFlag(int idx, byte bits) {
	_buttonFlags[idx] &= ~bits;
}

void ThreadResource::loadTheApt() {
	switch (_vm->_voy._field474) {
	case 1:
	case 2:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 17:
		_vm->_playStamp1 = 0x5700;
		break;
	case 3:
		_vm->_playStamp1 = 0x5800;
		break;
	case 4:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
		_vm->_playStamp1 = 0x5900;
		break;
	default:
		break;
	}

	if (_vm->_voy._field472 == 143)
		_vm->_voy._field472 = -1;

	if (_vm->_voy._field472  != -1) {
		doAptAnim(1);
		_vm->_bVoy->getBoltGroup(_vm->_playStamp1);
		_vm->_voy._field472 = -1;
		_vm->_graphicsManager._backgroundPage = _vm->_bVoy->boltEntry(
			_vm->_playStamp1 + 5)._picResource;
		(*_vm->_graphicsManager._vPort)->setupViewPort(
			_vm->_graphicsManager._backgroundPage);
	} else {
		_vm->_bVoy->getBoltGroup(_vm->_playStamp1);
		_vm->_graphicsManager._backgroundPage = _vm->_bVoy->boltEntry(
			_vm->_playStamp1 + 5)._picResource;
		(*_vm->_graphicsManager._vPort)->setupViewPort(
			_vm->_graphicsManager._backgroundPage);
	}

	CMapResource *pal = _vm->_bVoy->boltEntry(_vm->_playStamp1 + 4)._cMapResource;
	pal->_steps = 1; 
	pal->startFade();

	(*_vm->_graphicsManager._vPort)->_flags |= 8;
	_vm->_graphicsManager.flipPage();
	_vm->_eventsManager.sWaitFlip();

	while (!_vm->shouldQuit() && (_vm->_eventsManager._fadeStatus & 1))
		_vm->_eventsManager.delay(1);
}

void ThreadResource::freeTheApt() {
	_vm->_graphicsManager.fadeDownICF1(5);
	(*_vm->_graphicsManager._vPort)->_flags |= 8;
	_vm->_graphicsManager.flipPage();
	_vm->_eventsManager.sWaitFlip();

	while (!_vm->shouldQuit() && (_vm->_eventsManager._fadeStatus & 1))
		_vm->_eventsManager.delay(1);

	_vm->_graphicsManager.fadeUpICF1(0);

	if (_vm->_playStamp2 != -1) {
		_vm->_soundManager.stopVOCPlay();
		_vm->_playStamp2 = -1;
	}

	if (_vm->_voy._field472 == -1) {
		_vm->_graphicsManager.fadeDownICF(6);
	} else {
		doAptAnim(2);
	}

	if (_vm->_voy._field472 == 140) {
		_vm->_graphicsManager.screenReset();
		_vm->_graphicsManager.resetPalette();
	}

	(*_vm->_graphicsManager._vPort)->setupViewPort(nullptr);
	_vm->_bVoy->freeBoltGroup(_vm->_playStamp1);
	_vm->_playStamp1 = -1;
	_vm->_voy._field4386 = 0;
}

void ThreadResource::doAptAnim(int mode) {
	_vm->_bVoy->freeBoltGroup(0x100, true);

	// Figure out the resource to use
	int id = 0;
	switch (_vm->_voy._field472) {
	case 140:
		id = 0x5A00;
		break;
	case 141:
		id = 0x6000;
		break;
	case 142:
		id = 0x6600;
		break;
	case 143:
		id = 0x6C00;
		break;
	case 144:
		id = 0x6F00;
		break;
	default:
		break;
	}

	int id2 = (id == 0x6C00 || id == 0x6F00) ? 1 : 2;
	switch (_vm->_voy._field474) {
	case 3:
		id += id2 << 8;
		break;
	case 4:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
		id += id2 << 9;
		break;
	default:
		break;
	}

	if (mode)
		id += 0x100;

	// Do the display
	if (_vm->_bVoy->getBoltGroup(id)) {
		CMapResource *pal = _vm->_bVoy->boltEntry(id)._cMapResource;
		pal->_steps = 1;

		for (int idx = 0; (idx < 6) && !_vm->shouldQuit(); ++idx) {
			PictureResource *pic = _vm->_bVoy->boltEntry(id + idx + 1)._picResource;
			(*_vm->_graphicsManager._vPort)->setupViewPort(pic);
			pal->startFade();

			(*_vm->_graphicsManager._vPort)->_flags |= 8;
			_vm->_graphicsManager.flipPage();
			_vm->_eventsManager.sWaitFlip();

			_vm->_eventsManager.delay(5);
		}

		_vm->_bVoy->freeBoltGroup(id);
	}

	_vm->_bVoy->getBoltGroup(0x100);
}

} // End of namespace Voyeur
