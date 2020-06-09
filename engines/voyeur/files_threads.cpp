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

#include "voyeur/files.h"
#include "voyeur/screen.h"
#include "voyeur/voyeur.h"
#include "voyeur/staticres.h"
#include "common/config-manager.h"

namespace Voyeur {

int ThreadResource::_useCount[8];

void ThreadResource::init() {
	Common::fill(&_useCount[0], &_useCount[8], 0);
}

ThreadResource::ThreadResource(BoltFilesState &state, const byte *src):_vm(state._vm) {
	_stateId = READ_LE_UINT16(&src[0]);
	_stackId = READ_LE_UINT16(&src[0]);
	_savedStateId = READ_LE_UINT16(&src[0]);
	_savedStackId = READ_LE_UINT16(&src[0]);
	_ctlPtr = nullptr;
	_aptPos = Common::Point(-1, -1);

	_newStateId = -1;
	_newStackId = -1;
	_stateFlags = 0;
	_stateCount = 0;
	_parseCount = 0;
	_nextStateId = 0;
	_threadInfoPtr = nullptr;
	_playCommandsPtr = nullptr;
}

void ThreadResource::initThreadStruct(int idx, int id) {
	_stackId = -1;
	if (loadAStack(idx)) {
		_savedStateId = _savedStackId = -1;
		_stateId = id;
		_newStateId = -1;
		_newStackId = -1;

		doState();
	}
}

bool ThreadResource::loadAStack(int stackId) {
	if (_vm->_stampFlags & 1) {
		if (stackId < 0)
			error("loadAStack() - Invalid stackId %d", stackId);

		unloadAStack(_stackId);
		if  (!_useCount[stackId]) {
			BoltEntry &boltEntry = _vm->_stampLibPtr->boltEntry(_vm->_controlPtr->_memberIds[stackId]);
			if (!boltEntry._data)
				return false;

			_vm->_controlPtr->_entries[stackId] = boltEntry._data;
		}

		++_useCount[stackId];
	}

	_ctlPtr = _vm->_controlPtr->_entries[stackId];
	_stackId = stackId;
	return true;
}

void ThreadResource::unloadAStack(int stackId) {
	if (stackId < 0)
		return;

	if ((_vm->_stampFlags & 1) && _useCount[stackId]) {
		if (--_useCount[stackId] == 0) {
			_vm->_stampLibPtr->freeBoltMember(_vm->_controlPtr->_memberIds[stackId]);
		}
	}
}

bool ThreadResource::doState() {
	if (!getStateInfo())
		return false;

	getButtonsFlags();

	_vm->_glGoState = -1;
	_vm->_glGoStack = -1;

	performOpenCard();
	if (_stateFlags & 1) {
		return chooseSTAMPButton(_vm->getRandomNumber(_stateCount - 1));
	} else {
		return true;
	}
}

bool ThreadResource::getStateInfo() {
	int id = READ_LE_UINT16(_ctlPtr);

	if (id <= _stateId) {
		return false;
	} else {
		uint32 fld = READ_LE_UINT32(_ctlPtr + 2);
		fld += _stateId << 3;
		_nextStateId = READ_LE_UINT32(_ctlPtr + fld + 4);

		fld = READ_LE_UINT32(_ctlPtr + fld);
		byte *baseP = _ctlPtr + fld;
		_stateCount = READ_LE_UINT16(baseP);
		_stateFlags = READ_LE_UINT16(baseP + 2);
		_parseCount = READ_LE_UINT16(baseP + 4);

		_playCommandsPtr = getDataOffset();
		_playCommandsPtr += (READ_LE_UINT32(baseP + 6) / 2) << 1;

		_threadInfoPtr = baseP + 10;

		getButtonsText();
		return true;
	}
}

byte *ThreadResource::getDataOffset() {
	uint32 offset = READ_LE_UINT32(_ctlPtr + 10);
	return _ctlPtr + offset;
}

void ThreadResource::getButtonsText() {
	int idx = 0;

	for (const byte *p = _threadInfoPtr; *p != 0x49; p = getNextRecord(p)) {
		if (*p == 0xC0) {
			++p;
			if (*p++ & 0x80) {
				assert(idx < 63);
				p += 4;
			}

			++idx;
		}
	}
}

void ThreadResource::getButtonsFlags() {
	int idx = 0;

	for (const byte *p = _threadInfoPtr; *p != 0x49; p = getNextRecord(p)) {
		if (*p == 0xC0) {
			if (*++p & 0x20)
				_stateFlags |= 2;

			_buttonFlags[idx] = *p++;
			_buttonIds[idx] = *p++;

			if (_buttonFlags[idx] & 0x80)
				p += 4;

			++idx;
		}
	}
}

void ThreadResource::unloadAllStacks(VoyeurEngine *vm) {
	if (vm->_stampFlags & 1) {
		for (int i = 0; i < 8; ++i) {
			if (_useCount[i])
				vm->_stampLibPtr->freeBoltMember(vm->_controlPtr->_memberIds[i]);
		}
	}
}

void ThreadResource::performOpenCard() {
	for (const byte *p = _threadInfoPtr; *p != 0x49; p = getNextRecord(p)) {
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

	for (p = _threadInfoPtr; count <= cardId && *p != 0x49; p = getNextRecord(p)) {
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
	for (const byte *p = _threadInfoPtr; *p != 0x49; p = getNextRecord(p)) {
		if (*p == 0x48) {
			cardAction(p + 1);
			return;
		}
	}
}

void ThreadResource::cardAction(const byte *card) {
	_vm->_glGoState = -1;
	_vm->_glGoStack = -1;

	// Loop to perform card commands
	while (!_vm->shouldQuit() && *card < 70 && _vm->_glGoState == -1) {
		card = cardPerform(card);
	}
}

bool ThreadResource::chooseSTAMPButton(int buttonId) {
	for (int idx = 0; idx < _stateCount; ++idx) {
		if (_buttonIds[idx] == buttonId) {
			const byte *card = getSTAMPCard(idx);
			cardAction(card);

			bool flag = true;
			while (!_vm->shouldQuit() && _vm->_glGoStack != -1 && flag) {
				doSTAMPCardAction();
				flag = goToStateID(_vm->_glGoStack, _vm->_glGoState);
			}

			while (!_vm->shouldQuit() && _vm->_glGoState != -1 && flag) {
				doSTAMPCardAction();
				flag = goToState(-1, _vm->_glGoState);
			}

			return flag;
		}
	}

	return false;
}

void ThreadResource::parsePlayCommands() {
	_vm->_voy->_playStampMode = -1;
	_vm->_voy->_audioVisualStartTime = 0;
	_vm->_voy->_audioVisualDuration = 0;
	_vm->_voy->_boltGroupId2 = -1;
	_vm->_voy->_computerTextId = -1;
	_vm->_voy->_eventFlags &= ~EVTFLAG_8;
	_vm->_eventsManager->_videoDead = -1;

	// Reset hotspot data
	_vm->_voy->_videoHotspotTimes.reset();
	_vm->_voy->_audioHotspotTimes.reset();
	_vm->_voy->_evidenceHotspotTimes.reset();
	Common::fill(&_vm->_voy->_roomHotspotsEnabled[0], &_vm->_voy->_roomHotspotsEnabled[20], false);
	byte *dataP = _playCommandsPtr;
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
			_vm->_currentVocId = READ_LE_UINT16(dataP);
			dataP += 2;
			break;

		case 2:
			// Play an audio event
			v2 = READ_LE_UINT16(dataP);

			if (v2 == 0 || _vm->_controlPtr->_state->_victimIndex == v2) {
				_vm->_audioVideoId = READ_LE_UINT16(dataP + 2) - 1;
				_vm->_voy->_audioVisualStartTime = READ_LE_UINT16(dataP + 4);
				_vm->_voy->_audioVisualDuration = READ_LE_UINT16(dataP + 6);

				if (_vm->_voy->_RTVNum < _vm->_voy->_audioVisualStartTime ||
						(_vm->_voy->_audioVisualStartTime + _vm->_voy->_audioVisualDuration)  < _vm->_voy->_RTVNum) {
					_vm->_audioVideoId = -1;
				} else {
					_vm->_voy->_vocSecondsOffset = _vm->_voy->_RTVNum - _vm->_voy->_audioVisualStartTime;
					_vm->_voy->addAudioEventStart();

					// Play the audio
					assert(_vm->_audioVideoId < 38);
					_vm->playAudio(_vm->_audioVideoId);

					_vm->_voy->addAudioEventEnd();
					_vm->_eventsManager->incrementTime(1);
					_vm->_eventsManager->incrementTime(1);
					_vm->_audioVideoId = -1;
					parseIndex = 999;
				}
			}

			dataP += 8;
			break;

		case 3:
			// Play a video event
			v2 = READ_LE_UINT16(dataP);

			if (v2 == 0 || _vm->_controlPtr->_state->_victimIndex == v2) {
				_vm->_audioVideoId = READ_LE_UINT16(dataP + 2) - 1;
				_vm->_voy->_audioVisualStartTime = READ_LE_UINT16(dataP + 4);
				_vm->_voy->_audioVisualDuration = READ_LE_UINT16(dataP + 6);

				if (_vm->_voy->_RTVNum < _vm->_voy->_audioVisualStartTime ||
						(_vm->_voy->_audioVisualStartTime + _vm->_voy->_audioVisualDuration)  < _vm->_voy->_RTVNum) {
					_vm->_audioVideoId = -1;
				} else {
					_vm->_voy->_vocSecondsOffset = _vm->_voy->_RTVNum - _vm->_voy->_audioVisualStartTime;
					_vm->_voy->addVideoEventStart();
					_vm->_voy->_eventFlags &= ~EVTFLAG_TIME_DISABLED;
					_vm->_voy->_eventFlags |= EVTFLAG_RECORDING;
					_vm->playAVideo(_vm->_audioVideoId);

					_vm->_voy->_eventFlags &= ~EVTFLAG_RECORDING;
					_vm->_voy->_eventFlags |= EVTFLAG_TIME_DISABLED;
					_vm->_voy->addVideoEventEnd();
					_vm->_eventsManager->incrementTime(1);

					_vm->_audioVideoId = -1;
					_vm->_playStampGroupId = -1;

					if (_vm->_eventsManager->_videoDead != -1) {
						_vm->_bVoy->freeBoltGroup(0xE00);
						_vm->_eventsManager->_videoDead = -1;
						_vm->flipPageAndWait();
					}

					_vm->_eventsManager->_videoDead = -1;
					if (_stateCount == 2 && _vm->_eventsManager->_mouseClicked == 0) {
						_vm->_voy->_playStampMode = 132;
						parseIndex = 999;
					} else {
						_vm->_voy->_playStampMode = 129;
					}
				}
			}

			dataP += 8;
			break;

		case 4:
		case 22:
			// Case 22: Endgame news reports
			_vm->_audioVideoId = READ_LE_UINT16(dataP) - 1;
			dataP += 2;

			if (id == 22) {
				int resolveIndex = READ_LE_UINT16(dataP);
				dataP += 2;
				_vm->_playStampGroupId = _vm->_resolvePtr[resolveIndex];
			}

			_vm->_voy->_vocSecondsOffset = 0;
			_vm->_voy->_audioVisualStartTime = _vm->_voy->_RTVNum;
			_vm->_voy->_eventFlags &= ~(EVTFLAG_TIME_DISABLED | EVTFLAG_RECORDING);
			_vm->playAVideo(_vm->_audioVideoId);
			_vm->_voy->_eventFlags |= EVTFLAG_TIME_DISABLED;

			if (id != 22) {
				_vm->_audioVideoId = -1;
				parseIndex = 999;
			} else {
				int count = _vm->_bVoy->getBoltGroup(_vm->_playStampGroupId)->_entries.size() / 2;
				_vm->_soundManager->stopVOCPlay();
				_vm->_eventsManager->getMouseInfo();

				for (int i = 0; i < count; ++i) {
					pic = _vm->_bVoy->boltEntry(_vm->_playStampGroupId + i * 2)._picResource;
					pal = _vm->_bVoy->boltEntry(_vm->_playStampGroupId + i * 2 + 1)._cMapResource;

					_vm->_screen->_vPort->setupViewPort(pic);
					pal->startFade();

					_vm->flipPageAndWaitForFade();

					if (i > 0) {
						_vm->_bVoy->freeBoltMember(_vm->_playStampGroupId + i * 2);
						_vm->_bVoy->freeBoltMember(_vm->_playStampGroupId + i * 2 + 1);
					}

					Common::String file = Common::String::format("news%d.voc", i + 1);
					_vm->_soundManager->startVOCPlay(file);

					while (!_vm->shouldQuit() && !_vm->_eventsManager->_mouseClicked &&
							_vm->_soundManager->getVOCStatus()) {
						_vm->_eventsManager->delayClick(1);
						_vm->_eventsManager->getMouseInfo();
					}

					_vm->_soundManager->stopVOCPlay();

					if (i == (count - 1))
						_vm->_eventsManager->delayClick(480);

					if (_vm->shouldQuit() || _vm->_eventsManager->_mouseClicked)
						break;
				}

				_vm->_bVoy->freeBoltGroup(_vm->_playStampGroupId);
				_vm->_playStampGroupId = -1;
				_vm->_audioVideoId = -1;
				parseIndex = 999;
			}
			break;

		case 5:
			// Check whether transition to a given time period is allowed, and
			// if so, load the time information for the new time period
			v2 = READ_LE_UINT16(dataP);
			if (v2 == 0 || _vm->_controlPtr->_state->_victimIndex == v2) {
				_vm->_voy->_playStampMode = 5;
				int count = READ_LE_UINT16(dataP + 2);
				_vm->_voy->_RTVLimit = READ_LE_UINT16(dataP + 4);

				if (_vm->_voy->_transitionId != count) {
					if (_vm->_voy->_transitionId > 1)
						_vm->_voy->_eventFlags &= ~EVTFLAG_100;

					_vm->_voy->_transitionId = count;
					_vm->_gameMinute = LEVEL_M[count - 1];
					_vm->_gameHour = LEVEL_H[count - 1];
					//_vm->_v2A0A2 = 0;
					_vm->_voy->_RTVNum = 0;
					_vm->_voy->_RTANum = 255;
				}

				_vm->_voy->_isAM = (_vm->_voy->_transitionId == 6);
			}

			dataP += 6;
			break;

		case 6:
			_vm->_voy->_playStampMode = 6;
			v2 = READ_LE_UINT16(dataP);
			_vm->_playStampGroupId = _vm->_resolvePtr[v2];
			dataP += 2;
			break;

		case 7:
			// Load the video event scene hotspot times data
			v2 = READ_LE_UINT16(dataP);
			v3 = READ_LE_UINT16(dataP + 2) - 1;

			if (v2 == 0 || _vm->_controlPtr->_state->_victimIndex == v2) {
				int idx = 0;
				while (_vm->_voy->_videoHotspotTimes._min[idx][v3] != 9999)
					++idx;

				v2 = READ_LE_UINT16(dataP + 4);
				_vm->_voy->_videoHotspotTimes._min[idx][v3] = v2;
				_vm->_voy->_videoHotspotTimes._max[idx][v3] = v2 + READ_LE_UINT16(dataP + 6) - 2;
			}

			dataP += 8;
			break;

		case 8:
			// Load the audio event scene hotspot times data
 			v2 = READ_LE_UINT16(dataP);
			v3 = READ_LE_UINT16(dataP + 2) - 1;

			if (v2 == 0 || _vm->_controlPtr->_state->_victimIndex == v2) {
				int idx = 0;
				while (_vm->_voy->_audioHotspotTimes._min[idx][v3] != 9999)
					++idx;

				v2 = READ_LE_UINT16(dataP + 4);
				_vm->_voy->_audioHotspotTimes._min[idx][v3] = v2;
				_vm->_voy->_audioHotspotTimes._max[idx][v3] = v2 + READ_LE_UINT16(dataP + 6) - 2;
			}

			dataP += 8;
			break;

		case 9:
			// Load up evidence event scene hotspot times data
			v2 = READ_LE_UINT16(dataP);
			v3 = READ_LE_UINT16(dataP + 2) - 1;

			if (v2 == 0 || _vm->_controlPtr->_state->_victimIndex == v2) {
				int idx = 0;
				while (_vm->_voy->_evidenceHotspotTimes._min[idx][v3] != 9999)
					++idx;

				v2 = READ_LE_UINT16(dataP + 4);
				_vm->_voy->_evidenceHotspotTimes._min[idx][v3] = v2;
				_vm->_voy->_evidenceHotspotTimes._max[idx][v3] = v2 + READ_LE_UINT16(dataP + 6) - 2;
			}

			dataP += 8;
			break;

		case 10:
			// Pick the person who is to die, during startup
			if (_vm->_iForceDeath == -1) {
				// No specific person has been preset to be killed, so pick one randomly.
				// The loop below ensures that a different victim is picked.
				int lastVictim = ConfMan.hasKey("lastVictim") ? ConfMan.getInt("lastVictim") : -1;
				int randomVictim;
				do {
					randomVictim = _vm->getRandomNumber(3) + 1;
				} while (randomVictim == lastVictim);

				_vm->_controlPtr->_state->_victimIndex = randomVictim;
			} else {
				// Victim selected from command line
				_vm->_controlPtr->_state->_victimIndex = _vm->_iForceDeath;
			}
			
			ConfMan.setInt("lastVictim", _vm->_controlPtr->_state->_victimIndex);
			ConfMan.flushToDisk();
			break;

		case 11:
			_vm->_voy->_eventFlags |= EVTFLAG_2;
			break;

		case 12:
			v2 = READ_LE_UINT16(dataP);

			if (v2 == 0 || _vm->_controlPtr->_state->_victimIndex == v2) {
				_vm->_voy->_boltGroupId2 = _vm->_resolvePtr[READ_LE_UINT16(dataP + 2)];
				_vm->_voy->_roomHotspotsEnabled[READ_LE_UINT16(dataP + 4) - 1] = true;
			}

			dataP += 6;
			break;

		case 13:
			v2 = READ_LE_UINT16(dataP);

			if (v2 == 0 || _vm->_controlPtr->_state->_victimIndex == v2) {
				_vm->_voy->_computerTextId = READ_LE_UINT16(dataP + 2) - 1;
				_vm->_voy->_computerTimeMin = READ_LE_UINT16(dataP + 4);
				_vm->_voy->_computerTimeMax = READ_LE_UINT16(dataP + 6);

				_vm->_voy->_computerScreenRect.left = COMPUTER_SCREEN_TABLE[_vm->_voy->_computerTextId * 4];
				_vm->_voy->_computerScreenRect.top = COMPUTER_SCREEN_TABLE[_vm->_voy->_computerTextId * 4 + 1];
				_vm->_voy->_computerScreenRect.right = COMPUTER_SCREEN_TABLE[_vm->_voy->_computerTextId * 4 + 2];
				_vm->_voy->_computerScreenRect.bottom = COMPUTER_SCREEN_TABLE[_vm->_voy->_computerTextId * 4 + 3];
			}

			dataP += 8;
			break;

		case 14:
			_vm->_playStampGroupId = 2048;
			_vm->_voy->_playStampMode = 130;
			break;

		case 15:
			_vm->showEndingNews();
			break;

		case 16:
			_vm->_voy->_playStampMode = 16;
			break;

		case 17:
			_vm->_voy->_playStampMode = 17;
			break;

		case 18:
			// Called during the murder (Sunday 10:30PM) time period, to specify the
			// time expired point at which the murder takes place
			v2 = READ_LE_UINT16(dataP);
			v3 = READ_LE_UINT16(dataP + 2);

			if (v2 == 0 || _vm->_controlPtr->_state->_victimIndex == v2)
				_vm->_voy->_murderThreshold = v3;

			dataP += 4;
			break;

		case 19:
			_vm->_voy->_aptLoadMode = 140;
			loadTheApt();
			_vm->_voy->_aptLoadMode = 141;
			freeTheApt();
			break;

		case 20:
			_vm->_voy->_aptLoadMode = -1;
			loadTheApt();
			_vm->_voy->_aptLoadMode = 141;
			freeTheApt();
			break;

		case 21:
			_vm->_voy->_aptLoadMode = -1;
			loadTheApt();
			_vm->_voy->_aptLoadMode = 140;
			freeTheApt();
			break;

		case 23:
			_vm->_voy->_transitionId = 17;
			_vm->_voy->_aptLoadMode = -1;
			loadTheApt();
			_vm->_voy->_aptLoadMode = 144;
			freeTheApt();
			break;

		default:
			break;
		}
	}
}

const byte *ThreadResource::cardPerform(const byte *card) {
	uint16 id = *card++;
	int subId = 5;
	uint32 v2;
	byte bVal;
	uint32 idx1, idx2;
	debugC(DEBUG_BASIC, kDebugScripts, "cardPerform - %d", id);

	switch (id) {
	case 1:
		v2 = READ_LE_UINT32(card);
		card += 4;
		_vm->_controlPtr->_state->_vals[*card++] = v2;
		break;

	case 2:
		v2 = _vm->_controlPtr->_state->_vals[*card++];
		_vm->_controlPtr->_state->_vals[*card++] = v2;
		break;

	case 3:
		v2 = READ_LE_UINT32(card);
		card += 4;
		_vm->_controlPtr->_state->_vals[*card++] = v2;
		break;

	case 4:
		v2 = _vm->_controlPtr->_state->_vals[*card++];
		_vm->_controlPtr->_state->_vals[*card++] = v2;
		break;

	case 5: {
		v2 = READ_LE_UINT32(card);
		card += 4;
		int &v = _vm->_controlPtr->_state->_vals[*card++];
		v -= v2;
		break;
	}

	case 6: {
		idx1 = *card++;
		idx2 = *card++;

		v2 = _vm->_controlPtr->_state->_vals[idx1];
		int &v = _vm->_controlPtr->_state->_vals[idx2];
		v -= v2;
		break;
	}

	case 7: {
		int v3 = *card++;
		v2 = READ_LE_UINT32(card);
		card += 4;
		int &v = _vm->_controlPtr->_state->_vals[v3];
		v *= v2;
		break;
	}

	case 8: {
		idx1 = *card++;
		idx2 = *card++;

		int &v1 = _vm->_controlPtr->_state->_vals[idx1];
		v2 = _vm->_controlPtr->_state->_vals[idx2];
		v1 *= v2;
		break;
	}

	case 9: {
		idx1 = *card++;
		v2 = READ_LE_UINT32(card);
		card += 4;

		int &v = _vm->_controlPtr->_state->_vals[idx1];
		v /= v2;
		break;
	}

	case 10: {
		idx1 = *card++;
		idx2 = *card++;

		int &v1 = _vm->_controlPtr->_state->_vals[idx1];
		v2 = _vm->_controlPtr->_state->_vals[idx2];
		v1 /= v2;
		break;
	}

	case 11:
		v2 = READ_LE_UINT32(card);
		card += 4;
		v2 = _vm->getRandomNumber(v2 - 1) + 1;
		_vm->_controlPtr->_state->_vals[*card++] = v2;
		break;

	case 17:
		_vm->_glGoState = READ_LE_UINT16(card);
		card += 2;
		_vm->_glGoStack = -1;
		break;

	case 18:
		v2 = _vm->_controlPtr->_state->_vals[*card++];
		_vm->_glGoState = getStateFromID(v2);
		break;

	case 19:
		_vm->_glGoState = READ_LE_UINT32(card);
		card += 4;
		_vm->_glGoStack = READ_LE_UINT16(card);
		card += 2;
		break;

	case 23:
	case 24:
	case 27:
	case 28:
		subId -= 3;
		// fall through

	case 21:
	case 22:
	case 25:
	case 26:
		bVal = card[subId];
		if (bVal == 61) {
			if (cardPerform2(card, id)) {
				card += subId;
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
				card += subId;
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
				card += subId;
				card = cardPerform(card);
				while (*card++ != 61) {}
			} else {
				card += subId;
				while (*card != 61 && *card != 29)
					++card;
			}
		}
		break;

	case 41:
		bVal = *card++;
		assert(bVal < 8);
		card += 6;
		break;

	case 45:
		_newStateId = _nextStateId;
		_newStackId = _stackId;
		break;

	case 46:
		_vm->_glGoState = _newStateId;
		_vm->_glGoStack = _newStackId;
		_newStateId = -1;
		_newStackId = -1;
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
	int vLong, vLong2;

	switch (cardCmdId) {
	case 21:
		vLong = (int32)READ_LE_UINT32(pSrc + 1);
		return _vm->_controlPtr->_state->_vals[*pSrc] == vLong;

	case 22:
		vLong = (int32)READ_LE_UINT32(pSrc + 1);
		return _vm->_controlPtr->_state->_vals[*pSrc] != vLong;

	case 23:
		vLong = _vm->_controlPtr->_state->_vals[*pSrc];
		vLong2 = _vm->_controlPtr->_state->_vals[*(pSrc + 1)];
		return vLong == vLong2;

	case 24:
		vLong = _vm->_controlPtr->_state->_vals[*pSrc];
		vLong2 = _vm->_controlPtr->_state->_vals[*(pSrc + 1)];
		return vLong != vLong2;

	case 25:
		vLong = _vm->_controlPtr->_state->_vals[*pSrc];
		vLong2 = (int32)READ_LE_UINT32(pSrc + 1);
		return vLong < vLong2;

	case 26:
		vLong = _vm->_controlPtr->_state->_vals[*pSrc];
		vLong2 = (int32)READ_LE_UINT32(pSrc + 1);
		return vLong > vLong2;

	case 27:
		vLong = _vm->_controlPtr->_state->_vals[*pSrc];
		vLong2 = _vm->_controlPtr->_state->_vals[*(pSrc + 1)];
		return vLong < vLong2;

	case 28:
		vLong = _vm->_controlPtr->_state->_vals[*pSrc];
		vLong2 = _vm->_controlPtr->_state->_vals[*(pSrc + 1)];
		return vLong > vLong2;

	default:
		return false;
	}
}

int ThreadResource::doApt() {
	loadTheApt();

	_vm->_currentVocId = 151;
	_vm->_voy->_viewBounds = _vm->_bVoy->boltEntry(_vm->_playStampGroupId)._rectResource;
	Common::Array<RectEntry> &hotspots = _vm->_bVoy->boltEntry(
		_vm->_playStampGroupId + 1)._rectResource->_entries;
	_vm->_eventsManager->getMouseInfo();

	// Very first time apartment is shown, start the phone message
	if (_aptPos.x == -1) {
		_aptPos.x = hotspots[2].left;
		_aptPos.y = hotspots[2].top;
		_vm->_currentVocId = 153;
	}

	if (_vm->_voy->_playStampMode == 16) {
		hotspots[0].left = 999;
		hotspots[3].left = 999;
		_aptPos.x = hotspots[4].left + 28;
		_aptPos.y = hotspots[4].top + 28;
	}

	_vm->_eventsManager->setMousePos(Common::Point(_aptPos.x, _aptPos.y));
	_vm->_soundManager->startVOCPlay(_vm->_soundManager->getVOCFileName(_vm->_currentVocId));
	_vm->_currentVocId = 151;

	_vm->_screen->setColor(129, 82, 82, 82);
	_vm->_screen->setColor(130, 112, 112, 112);
	_vm->_screen->setColor(131, 215, 215, 215);
	_vm->_screen->setColor(132, 235, 235, 235);

	_vm->_eventsManager->_intPtr._hasPalette = true;

	// Set up the cursors
	PictureResource *unselectedCursor = _vm->_bVoy->boltEntry(_vm->_playStampGroupId + 2)._picResource;
	PictureResource *selectedCursor = _vm->_bVoy->boltEntry(_vm->_playStampGroupId + 3)._picResource;
	unselectedCursor->_keyColor = 0xff;
	selectedCursor->_keyColor = 0xff;
	_vm->_eventsManager->setCursor(unselectedCursor);
	_vm->_eventsManager->showCursor();

	// Main loop to allow users to move the cursor and select hotspots
	int hotspotId;
	int prevHotspotId = -1;
	Common::Point pt;
	PictureResource *pic;
	Common::Rect gmmHotspot(75, 125, 130, 140);

	do {
		_vm->_voyeurArea = AREA_APARTMENT;

		if (_vm->_loadGameSlot != -1) {
			// Load a savegame
			int slot = _vm->_loadGameSlot;
			_vm->_loadGameSlot = -1;
			_vm->loadGame(slot);

			_vm->_eventsManager->showCursor();
		}

		_vm->_eventsManager->getMouseInfo();
		if (!_vm->_soundManager->getVOCStatus()) {
			// Previous sound ended, so start up a new one
			_vm->_currentVocId = 151 - _vm->getRandomNumber(4);
			_vm->_soundManager->startVOCPlay(_vm->_soundManager->getVOCFileName(_vm->_currentVocId));
		}

		// Loop through the hotspot list
		hotspotId = -1;
		pt = _vm->_eventsManager->getMousePos() + Common::Point(16, 16);
		for (int idx = 0; idx < (int)hotspots.size(); ++idx) {
			if (hotspots[idx].contains(pt)) {
				// Cursor is within hotspot area

				// Don't allow the camera to be highlighted on Monday morning.
				if (idx == 0 && _vm->_voy->_transitionId == 17)
					continue;

				// Set the highlighted hotspot Id
				hotspotId = idx;

				if (hotspotId != prevHotspotId) {
					// Check for whether to replace hotspot Id for "Watch TV" for
					// "Review the Tape" if player has already watched the TV
					if ((_vm->_voy->_eventFlags & EVTFLAG_100) && (hotspotId == 2))
						hotspotId = 5;

					// Draw the text description for the highlighted hotspot
					pic = _vm->_bVoy->boltEntry(_vm->_playStampGroupId +
						hotspotId + 6)._picResource;
					_vm->_screen->sDrawPic(pic, _vm->_screen->_vPort,
						Common::Point(106, 200));
				}

				break;
			}
		}

		// Check for presence in ScummVM GMM
		if (gmmHotspot.contains(pt))
			hotspotId = 42;

		// Update the cursor to either standard or highlighted eye cursor
		_vm->_eventsManager->setCursor((hotspotId == -1) ? unselectedCursor : selectedCursor);
		_vm->flipPageAndWait();

		if (hotspotId == 42 && _vm->_eventsManager->_leftClick) {
			// Show the ScummVM GMM
			_vm->_eventsManager->getMouseInfo();
			_vm->openMainMenuDialog();
		}

	} while (!_vm->shouldQuit() && (!_vm->_eventsManager->_leftClick || hotspotId == -1));

	_vm->_eventsManager->hideCursor();
	pt = _vm->_eventsManager->getMousePos();
	_aptPos.x = pt.x;
	_aptPos.y = pt.y;

	switch (hotspotId) {
	case 0:
		_vm->_voy->_aptLoadMode = 140;
		break;
	case 1:
		_vm->_voy->_aptLoadMode = 143;
		break;
	case 2:
		_vm->_voy->_aptLoadMode = 142;
		break;
	case 5:
		_vm->_voy->_aptLoadMode = 141;
		break;
	default:
		_vm->_voy->_aptLoadMode = -1;
		break;
	}

	freeTheApt();

	if (_vm->_voy->_transitionId == 1 && hotspotId == 0)
		_vm->checkTransition();

	if (!hotspotId)
		_vm->makeViewFinder();

	return hotspotId;
}

void ThreadResource::doRoom() {
	VoyeurEngine &vm = *_vm;
	SVoy voy = *vm._voy;

	vm.makeViewFinderP();
	voy._fadingType = 0;

	if (!vm._bVoy->getBoltGroup(vm._playStampGroupId))
		return;

	vm._screen->_backColors = vm._bVoy->boltEntry(vm._playStampGroupId + 1)._cMapResource;
	vm._screen->_backgroundPage = vm._bVoy->boltEntry(vm._playStampGroupId)._picResource;
	vm._screen->_vPort->setupViewPort(vm._screen->_backgroundPage);
	vm._screen->_backColors->startFade();

	voy._fadingStep1 = 2;
	voy._fadingStep2 = 0;
	voy._fadingType = 1;

	Common::Array<RectEntry> &hotspots = vm._bVoy->boltEntry(vm._playStampGroupId + 4)._rectResource->_entries;
	int hotspotId = -1;

	PictureResource *crosshairsCursor = vm._bVoy->boltEntry(vm._playStampGroupId + 2)._picResource;
	PictureResource *magnifierCursor = vm._bVoy->boltEntry(vm._playStampGroupId + 3)._picResource;
	vm._eventsManager->showCursor();

	RectResource viewBounds(48, 38, 336, 202);
	voy._viewBounds = &viewBounds;

	vm._eventsManager->getMouseInfo();
	vm._eventsManager->setMousePos(Common::Point(192, 120));
	voy._fadingType = 0;
	vm._currentVocId = 146;
	voy._musicStartTime = voy._RTVNum;

	voy._vocSecondsOffset = 0;
	vm._soundManager->startVOCPlay(vm._currentVocId);
	voy._eventFlags &= ~EVTFLAG_TIME_DISABLED;

	bool breakFlag = false;
	while (!vm.shouldQuit() && !breakFlag) {
		_vm->_voyeurArea = AREA_ROOM;
		vm._screen->setColor(128, 0, 255, 0);
		vm._eventsManager->_intPtr._hasPalette = true;

		do {
			if (vm._currentVocId != -1 && !vm._soundManager->getVOCStatus()) {
				voy._musicStartTime = voy._RTVNum;
				voy._vocSecondsOffset = 0;
				vm._soundManager->startVOCPlay(vm._currentVocId);
			}

			vm._eventsManager->getMouseInfo();
			Common::Point pt = vm._eventsManager->getMousePos();
			pt += Common::Point(30, 15);

			hotspotId = -1;

			if (voy._computerTextId != -1 && voy._computerScreenRect.contains(pt))
				hotspotId = 999;

			for (uint idx = 0; idx < hotspots.size(); ++idx) {
				if (hotspots[idx].contains(pt)) {
					int arrIndex = hotspots[idx]._arrIndex;
					if (voy._roomHotspotsEnabled[arrIndex - 1]) {
						hotspotId = idx;
						break;
					}
				}
			}

			if (hotspotId == -1) {
				vm._eventsManager->setCursorColor(128, 0);
				vm._eventsManager->setCursor(crosshairsCursor);
			} else if (hotspotId != 999 || voy._RTVNum < voy._computerTimeMin ||
					(voy._computerTimeMax - 2) < voy._RTVNum) {
				vm._eventsManager->setCursorColor(128, 1);
				vm._eventsManager->setCursor(magnifierCursor);
			} else {
				vm._eventsManager->setCursorColor(128, 2);
				vm._eventsManager->setCursor(magnifierCursor);
			}

			vm._eventsManager->_intPtr._hasPalette = true;
			vm._screen->_vPort->_flags |= DISPFLAG_8;
			vm._screen->flipPage();
			vm._eventsManager->sWaitFlip();
		} while (!vm.shouldQuit() && !vm._eventsManager->_mouseClicked);

		if (!vm._eventsManager->_leftClick || hotspotId == -1) {
			if (vm._eventsManager->_rightClick)
				breakFlag = true;

			Common::Point pt = vm._eventsManager->getMousePos();
			vm._eventsManager->getMouseInfo();
			vm._eventsManager->setMousePos(pt);
		} else {
			voy._eventFlags |= EVTFLAG_RECORDING;
			vm._eventsManager->hideCursor();
			vm._eventsManager->startCursorBlink();

			if (hotspotId == 999) {
				_vm->flipPageAndWait();

				if (vm._currentVocId != -1) {
					voy._vocSecondsOffset = voy._RTVNum - voy._musicStartTime;
					vm._soundManager->stopVOCPlay();
				}

				vm.getComputerBrush();
				_vm->flipPageAndWait();

				vm._voy->addComputerEventStart();

				vm._eventsManager->_mouseClicked = false;
				vm._eventsManager->startCursorBlink();

				int totalChars = vm.doComputerText(9999);
				if (totalChars)
					vm._voy->addComputerEventEnd(totalChars);

				vm._bVoy->freeBoltGroup(0x4900);
			} else {
				vm.doEvidDisplay(hotspotId, 999);
			}

			voy._eventFlags &= ~EVTFLAG_RECORDING;
			if (!vm._eventsManager->_mouseClicked)
				vm._eventsManager->delayClick(18000);

			// WORKAROUND: Skipped code from the original, that freed the group,
			// reloaded it, and reloaded the cursors

			vm._screen->_backColors = vm._bVoy->boltEntry(
				vm._playStampGroupId + 1)._cMapResource;
			vm._screen->_backgroundPage = vm._bVoy->boltEntry(
				vm._playStampGroupId)._picResource;

			vm._screen->_vPort->setupViewPort();
			vm._screen->_backColors->startFade();
			_vm->flipPageAndWait();

			while (!vm.shouldQuit() && (vm._eventsManager->_fadeStatus & 1))
				vm._eventsManager->delay(1);
			vm._eventsManager->hideCursor();

			while (!vm.shouldQuit() && voy._fadingAmount2 > 0) {
				if (voy._fadingAmount1 < 63) {
					voy._fadingAmount1 += 4;
					if (voy._fadingAmount1 > 63)
						voy._fadingAmount1 = 63;
				}

				if (voy._fadingAmount2 > 0) {
					voy._fadingAmount2 -= 8;
					if (voy._fadingAmount2 < 0)
						voy._fadingAmount2 = 0;
				}

				vm._eventsManager->delay(1);
			}

			_vm->flipPageAndWait();

			vm._screen->fadeUpICF1();
			voy._eventFlags &= EVTFLAG_RECORDING;
			vm._eventsManager->showCursor();
		}
	}

	voy._eventFlags = EVTFLAG_TIME_DISABLED;
	vm._eventsManager->incrementTime(1);
	voy._viewBounds = nullptr;
	voy._fadingType = 0;
	vm.makeViewFinderP();

	if (voy._boltGroupId2 != -1) {
		vm._bVoy->freeBoltGroup(voy._boltGroupId2);
		voy._boltGroupId2 = -1;
	}

	if (vm._playStampGroupId != -1) {
		vm._bVoy->freeBoltGroup(vm._playStampGroupId);
		vm._playStampGroupId = -1;
	}

	if (vm._currentVocId != -1) {
		vm._soundManager->stopVOCPlay();
		vm._currentVocId = -1;
	}

	vm._eventsManager->hideCursor();
	chooseSTAMPButton(0);
}

int ThreadResource::doInterface() {
	PictureResource *pic;
	Common::Point pt;

	_vm->_voy->_eventFlags |= EVTFLAG_TIME_DISABLED;
	if (_vm->_voy->_abortInterface) {
		_vm->_voy->_abortInterface = false;
		return -2;
	}

	_vm->_voy->_eventFlags &= ~EVTFLAG_100;
	_vm->_playStampGroupId = -1;
	_vm->_eventsManager->_intPtr._flashStep = 1;
	_vm->_eventsManager->_intPtr._flashTimer = 0;

	if (_vm->_voy->_RTVNum >= _vm->_voy->_RTVLimit || _vm->_voy->_RTVNum < 0)
		_vm->_voy->_RTVNum = _vm->_voy->_RTVLimit - 1;

	if (_vm->_voy->_transitionId < 15 && _vm->_debugger->_isTimeActive
		&& (_vm->_voy->_RTVLimit - 3) < _vm->_voy->_RTVNum) {
		_vm->_voy->_RTVNum = _vm->_voy->_RTVLimit;
		_vm->makeViewFinder();

		_vm->initIFace();
		_vm->_eventsManager->hideCursor();
		_vm->_voy->_RTVNum = _vm->_voy->_RTVLimit - 4;
		_vm->_voy->_eventFlags &= ~EVTFLAG_TIME_DISABLED;

		while (!_vm->shouldQuit() && _vm->_voy->_RTVNum < _vm->_voy->_RTVLimit) {
			_vm->flashTimeBar();
			_vm->_eventsManager->delayClick(1);
		}

		_vm->_voy->_eventFlags |= EVTFLAG_TIME_DISABLED;
		chooseSTAMPButton(20);
		parsePlayCommands();

		_vm->_eventsManager->showCursor();
	}

	_vm->checkTransition();
	_vm->makeViewFinder();
	_vm->_eventsManager->getMouseInfo();
	_vm->initIFace();

	Common::Array<RectEntry> *hotspots = &_vm->_bVoy->boltEntry(
		_vm->_playStampGroupId + 1)._rectResource->_entries;
	_vm->_currentVocId = 151 - _vm->getRandomNumber(5);
	_vm->_voy->_vocSecondsOffset = _vm->getRandomNumber(29);

	Common::String fname = _vm->_soundManager->getVOCFileName(_vm->_currentVocId);
	_vm->_soundManager->startVOCPlay(fname);
	_vm->_eventsManager->getMouseInfo();

	_vm->_screen->setColor(240, 220, 220, 220);
	_vm->_eventsManager->_intPtr._hasPalette = true;
	_vm->_voy->_eventFlags &= ~EVTFLAG_TIME_DISABLED;

	// Set the cusor
	PictureResource *crosshairsCursor = _vm->_bVoy->boltEntry(0x112)._picResource;
	PictureResource *eyeCursor = _vm->_bVoy->boltEntry(0x113)._picResource;
	PictureResource *listenCursor = _vm->_bVoy->boltEntry(0x114)._picResource;
	PictureResource *mangifyCursor = _vm->_bVoy->boltEntry(0x115)._picResource;

	_vm->_eventsManager->setCursor(crosshairsCursor);

	// Main loop
	int regionIndex = 0;
	Common::Rect mansionViewBounds(MANSION_VIEW_X, MANSION_VIEW_Y,
		MANSION_VIEW_X + MANSION_VIEW_WIDTH, MANSION_VIEW_Y + MANSION_VIEW_HEIGHT);

	do {
		_vm->_voyeurArea = AREA_INTERFACE;
		_vm->doTimeBar();
		_vm->_eventsManager->getMouseInfo();

		if (checkMansionScroll())
			_vm->doScroll(_vm->_mansionViewPos);

		_vm->checkPhoneCall();
		if (!_vm->_soundManager->getVOCStatus()) {
			_vm->_currentVocId = 151 - _vm->getRandomNumber(5);
			_vm->_soundManager->startVOCPlay(_vm->_soundManager->getVOCFileName(_vm->_currentVocId));
		}

		// Calculate the mouse position within the entire mansion
		pt = _vm->_eventsManager->getMousePos();
		if (!mansionViewBounds.contains(pt))
			pt = Common::Point(-1, -1);
		else
			pt = _vm->_mansionViewPos +
				Common::Point(pt.x - MANSION_VIEW_X, pt.y - MANSION_VIEW_Y);
		regionIndex = -1;

		for (uint hotspotIdx = 0; hotspotIdx < hotspots->size(); ++hotspotIdx) {
			if ((*hotspots)[hotspotIdx].contains(pt)) {
				// Rect check done
				for (int arrIndex = 0; arrIndex < 3; ++arrIndex) {
					if (_vm->_voy->_audioHotspotTimes.isInRange(arrIndex, hotspotIdx, _vm->_voy->_RTVNum)) {
						// Set the ear cursor for an audio event
						_vm->_eventsManager->setCursor(listenCursor);
						regionIndex = hotspotIdx;
					}

					if (_vm->_voy->_evidenceHotspotTimes.isInRange(arrIndex, hotspotIdx, _vm->_voy->_RTVNum)) {
						// Set the magnifier cursor for an evidence event
						_vm->_eventsManager->setCursor(mangifyCursor);
						regionIndex = hotspotIdx;
					}
				}

				for (int arrIndex = 0; arrIndex < 8; ++arrIndex) {
					if (_vm->_voy->_videoHotspotTimes.isInRange(arrIndex, hotspotIdx, _vm->_voy->_RTVNum)) {
						// Set the eye cursor for a video event
						_vm->_eventsManager->setCursor(eyeCursor);
						regionIndex = hotspotIdx;
					}
				}
			}
		}

		if (regionIndex == -1) {
			// Reset back to the crosshairs cursor
			_vm->_eventsManager->setCursor(crosshairsCursor);
		}

		// Regularly update the time display
		if (_vm->_voy->_RTANum & 2) {
			_vm->_screen->drawANumber(_vm->_screen->_vPort,
				_vm->_gameMinute / 10, Common::Point(190, 25));
			_vm->_screen->drawANumber(_vm->_screen->_vPort,
				_vm->_gameMinute % 10, Common::Point(201, 25));

			if (_vm->_voy->_RTANum & 4) {
				int v = _vm->_gameHour / 10;
				_vm->_screen->drawANumber(_vm->_screen->_vPort,
					v == 0 ? 10 : v, Common::Point(161, 25));
				_vm->_screen->drawANumber(_vm->_screen->_vPort,
					_vm->_gameHour % 10, Common::Point(172, 25));

				pic = _vm->_bVoy->boltEntry(_vm->_voy->_isAM ? 272 : 273)._picResource;
				_vm->_screen->sDrawPic(pic, _vm->_screen->_vPort,
					Common::Point(215, 27));
			}
		}

		_vm->_voy->_RTANum = 0;
		_vm->flipPageAndWait();

		pt = _vm->_eventsManager->getMousePos();
		if ((_vm->_voy->_RTVNum >= _vm->_voy->_RTVLimit) || ((_vm->_voy->_eventFlags & EVTFLAG_VICTIM_PRESET) &&
				_vm->_eventsManager->_rightClick && (pt.x == 0))) {
			// Time to transition to the next time period
			_vm->_eventsManager->getMouseInfo();

			if (_vm->_voy->_transitionId == 15) {
				regionIndex = 20;
				_vm->_voy->_transitionId = 17;
				_vm->_soundManager->stopVOCPlay();
				_vm->checkTransition();
				_vm->_eventsManager->_leftClick = true;
			} else {
				_vm->_voy->_eventFlags |= EVTFLAG_TIME_DISABLED;

				chooseSTAMPButton(20);
				parsePlayCommands();
				_vm->checkTransition();
				_vm->makeViewFinder();

				_vm->initIFace();

				hotspots = &_vm->_bVoy->boltEntry(_vm->_playStampGroupId + 1)._rectResource->_entries;
				_vm->_eventsManager->getMouseInfo();

				_vm->_voy->_eventFlags &= ~EVTFLAG_TIME_DISABLED;
				_vm->_eventsManager->_intPtr._flashStep = 1;
				_vm->_eventsManager->_intPtr._flashTimer = 0;
			}
		}
	} while (!_vm->_eventsManager->_rightClick && !_vm->shouldQuit() &&
		(!_vm->_eventsManager->_leftClick || regionIndex == -1));

	_vm->_eventsManager->hideCursor();
	_vm->_voy->_eventFlags |= EVTFLAG_TIME_DISABLED;
	_vm->_bVoy->freeBoltGroup(_vm->_playStampGroupId);
	if (_vm->_currentVocId != -1)
		_vm->_soundManager->stopVOCPlay();

	return !_vm->_eventsManager->_rightClick ? regionIndex : -2;
}

bool ThreadResource::checkMansionScroll() {
	Common::Point pt = _vm->_eventsManager->getMousePos() -
		Common::Point(MANSION_VIEW_X, MANSION_VIEW_Y);
	Common::Point &viewPos = _vm->_mansionViewPos;
	bool result = false;

	// Scroll mansion view if close to any of the mansion edges
	if (pt.x >= 0 && pt.x < MANSION_SCROLL_AREA_X && viewPos.x > 0) {
		viewPos.x = MAX(viewPos.x - MANSION_SCROLL_INC_X, 0);
		result = true;
	}
	if  (pt.x >= (MANSION_VIEW_WIDTH - MANSION_SCROLL_AREA_X) &&
			pt.x < MANSION_VIEW_WIDTH && viewPos.x < MANSION_MAX_X) {
		viewPos.x = MIN(viewPos.x + MANSION_SCROLL_INC_X, MANSION_MAX_X);
		result = true;
	}
	if (pt.y >= 0 && pt.y < MANSION_SCROLL_AREA_Y && viewPos.y > 0) {
		viewPos.y = MAX(viewPos.y - MANSION_SCROLL_INC_Y, 0);
		result = true;
	}
	if  (pt.y >= (MANSION_VIEW_HEIGHT - MANSION_SCROLL_AREA_Y) &&
			pt.y < MANSION_VIEW_HEIGHT && viewPos.y < MANSION_MAX_Y) {
		viewPos.y = MIN(viewPos.y + MANSION_SCROLL_INC_Y, MANSION_MAX_Y);
		result = true;
	}

	// Return whether mansion view area has changed
	return result;
}

bool ThreadResource::goToStateID(int stackId, int id) {
	debugC(DEBUG_BASIC, kDebugScripts, "goToStateID - %d, %d", stackId, id);

	// Save current stack
	savePrevious();

	if (_stackId == stackId || stackId == -1 || loadAStack(stackId)) {
		// Now in the correct state
		_stateId = getStateFromID(id);

		if (_stateId != -1) {
			return doState();
		} else {
			_stateId = _savedStateId;
			_stackId = _savedStackId;
		}
	}

	return false;
}

bool ThreadResource::goToState(int stackId, int stateId) {
	debugC(DEBUG_BASIC, kDebugScripts, "goToState - %d, %d", stackId, stateId);

	savePrevious();
	if (stackId == -1 || loadAStack(stackId)) {
		if (stateId != -1)
			_stateId = stateId;

		return doState();
	} else {
		return false;
	}
}

void ThreadResource::savePrevious() {
	if (_savedStateId != _stateId || _stackId != _savedStackId) {
		_savedStateId = _stateId;
		_savedStackId = _stackId;
	}
}

void ThreadResource::setButtonFlag(int idx, byte bits) {
	_buttonFlags[idx] |= bits;
}

void ThreadResource::clearButtonFlag(int idx, byte bits) {
	_buttonFlags[idx] &= ~bits;
}

void ThreadResource::loadTheApt() {
	switch (_vm->_voy->_transitionId) {
	case 1:
	case 2:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 17:
		_vm->_playStampGroupId = 0x5700;
		break;
	case 3:
		_vm->_playStampGroupId = 0x5800;
		break;
	case 4:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
		_vm->_playStampGroupId = 0x5900;
		break;
	default:
		break;
	}

	if (_vm->_voy->_aptLoadMode == 143)
		_vm->_voy->_aptLoadMode = -1;

	if (_vm->_voy->_aptLoadMode  != -1) {
		if (_vm->_loadGameSlot != -1)
			doAptAnim(1);

		_vm->_bVoy->getBoltGroup(_vm->_playStampGroupId);
		_vm->_voy->_aptLoadMode = -1;
		_vm->_screen->_backgroundPage = _vm->_bVoy->boltEntry(
			_vm->_playStampGroupId + 5)._picResource;
		_vm->_screen->_vPort->setupViewPort(
			_vm->_screen->_backgroundPage);
	} else {
		_vm->_bVoy->getBoltGroup(_vm->_playStampGroupId);
		_vm->_screen->_backgroundPage = _vm->_bVoy->boltEntry(
			_vm->_playStampGroupId + 5)._picResource;
		_vm->_screen->_vPort->setupViewPort(
			_vm->_screen->_backgroundPage);
	}

	CMapResource *pal = _vm->_bVoy->boltEntry(_vm->_playStampGroupId + 4)._cMapResource;
	pal->_steps = 1;
	pal->startFade();
	_vm->flipPageAndWaitForFade();
}

void ThreadResource::freeTheApt() {
	_vm->_screen->fadeDownICF1(5);
	_vm->flipPageAndWaitForFade();

	_vm->_screen->fadeUpICF1();

	if (_vm->_currentVocId != -1) {
		_vm->_soundManager->stopVOCPlay();
		_vm->_currentVocId = -1;
	}

	if (_vm->_voy->_aptLoadMode == -1) {
		_vm->_screen->fadeDownICF(6);
	} else {
		doAptAnim(2);
	}

	if (_vm->_voy->_aptLoadMode == 140) {
		_vm->_screen->screenReset();
		_vm->_screen->resetPalette();
	}

	_vm->_screen->_vPort->setupViewPort(nullptr);
	_vm->_bVoy->freeBoltGroup(_vm->_playStampGroupId);
	_vm->_playStampGroupId = -1;
	_vm->_voy->_viewBounds = nullptr;
}

void ThreadResource::doAptAnim(int mode) {
	_vm->_bVoy->freeBoltGroup(0x100);

	// Figure out the resource to use
	int id = 0;
	switch (_vm->_voy->_aptLoadMode) {
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
	switch (_vm->_voy->_transitionId) {
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

	if (mode == 1)
		id += 0x100;

	// Do the display
	if (_vm->_bVoy->getBoltGroup(id)) {
		CMapResource *pal = _vm->_bVoy->boltEntry(id)._cMapResource;
		pal->_steps = 1;

		for (int idx = 0; (idx < 6) && !_vm->shouldQuit(); ++idx) {
			PictureResource *pic = _vm->_bVoy->boltEntry(id + idx + 1)._picResource;
			_vm->_screen->_vPort->setupViewPort(pic);
			pal->startFade();

			_vm->flipPageAndWait();
			_vm->_eventsManager->delayClick(5);
		}

		_vm->_bVoy->freeBoltGroup(id);
	}

	_vm->_bVoy->getBoltGroup(0x100);
}

void ThreadResource::synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(_aptPos.x);
	s.syncAsSint16LE(_aptPos.y);

	int stateId = _stateId;
	int stackId = _stackId;
	s.syncAsSint16LE(stateId);
	s.syncAsSint16LE(stackId);

	if (s.isLoading() && (stateId != _stateId || stackId != _stackId))
		goToState(stackId, stateId);

	s.syncAsSint16LE(_savedStateId);
	s.syncAsSint16LE(_savedStackId);
}

} // End of namespace Voyeur
