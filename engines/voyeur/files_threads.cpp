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

int ThreadResource::_stampFlags = 0;
int ThreadResource::_useCount[8];
byte *ThreadResource::_threadDataPtr = NULL;
CMapResource *ThreadResource::_cmd14Pal = NULL;

ThreadResource::ThreadResource(BoltFilesState &state, const byte *src):
		_vm(state._vm) {
	_flags = src[8];
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
		_field28E += READ_LE_UINT32(baseP + 6) / 2;

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

			_field4E[idx] = *p++;
			_field18E[idx] = *p++;

			if (_field4E[idx] & 0x80)
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
		if (*p * 0x80)
			p += 4;
		return p + 2;
	default:
		return p;
	}
}

void ThreadResource::cardAction(const byte *p) {
	warning("TODO: cardAction");
}

bool ThreadResource::chooseSTAMPButton(int idx) {
	warning("TODO: chooseSTAMPButton");
	return false;
}

void ThreadResource::parsePlayCommands() {
	Common::fill(&_vm->_voy._arr1[0][0], &_vm->_voy._arr1[8][20], 0);
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

					_vm->_bVoy->freeBoltGroup(0x17F00);
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
						_vm->_bVoy->freeBoltGroup(0x10E00);
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

				_vm->_voy._arr1[idx][v3] = READ_LE_UINT16(dataP + 4) + READ_LE_UINT16(dataP + 6);
				_vm->_voy._arr2[idx][v3] = v3;
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

				_vm->_voy._arr3[idx][v3] = READ_LE_UINT16(dataP + 4) + READ_LE_UINT16(dataP + 6);
				_vm->_voy._arr4[idx][v3] = v3;
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

				_vm->_voy._arr5[idx][v3] = READ_LE_UINT16(dataP + 4) + READ_LE_UINT16(dataP + 6);
				_vm->_voy._arr6[idx][v3] = v3;
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
			_vm->loadTheApt();
			_vm->_voy._field472 = 141;
			_vm->freeTheApt();
			break;

		case 20:
			_vm->_voy._field472 = -1;
			_vm->loadTheApt();
			_vm->_voy._field472 = 141;
			_vm->freeTheApt();
			break;

		case 21:
			_vm->_voy._field472 = -1;
			_vm->loadTheApt();
			_vm->_voy._field472 = 140;
			_vm->freeTheApt();
			break;

		case 23:
			_vm->_voy._field474 = 17;
			_vm->_voy._field472 = -1;
			_vm->loadTheApt();
			_vm->_voy._field472 = 144;
			_vm->freeTheApt();
			break;

		default:
			break;
		}
	}
}

int ThreadResource::doApt() {
	warning("TODO: doApt");
	return 0;
}

void ThreadResource::doRoom() {
	warning("TODO: doRoom");
}

int ThreadResource::doInterface() {
	warning("TODO: doInterface");
	return 0;
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

} // End of namespace Voyeur
