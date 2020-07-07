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

#include "voyeur/data.h"
#include "voyeur/voyeur.h"

namespace Voyeur {

void VoyeurEvent::synchronize(Common::Serializer &s) {
	s.syncAsByte(_hour);
	s.syncAsByte(_minute);
	s.syncAsByte(_isAM);
	s.syncAsByte(_type);
	s.syncAsSint16LE(_audioVideoId);
	s.syncAsSint16LE(_computerOn);
	s.syncAsSint16LE(_computerOff);
	s.syncAsSint16LE(_dead);
}

/*------------------------------------------------------------------------*/

SVoy::SVoy(VoyeurEngine *vm):_vm(vm) {
	// Initialize all the data fields
	_abortInterface = false;
	_isAM = false;
	Common::fill(&_phoneCallsReceived[0], &_phoneCallsReceived[5], false);
	Common::fill(&_roomHotspotsEnabled[0], &_roomHotspotsEnabled[32], false);
	_victimMurdered = false;

	_audioVisualStartTime = 0;
	_audioVisualDuration = 0;
	_boltGroupId2 = 0;
	_computerTextId = -1;
	_computerTimeMin = _computerTimeMax = 0;
	_eventCount = 0;
	_fadingStep1 = 0;
	_fadingStep2 = 0;
	_fadingType = 0;
	_incriminatedVictimNumber = 0;
	_musicStartTime = 0;
	_playStampMode = 0;
	_switchBGNum = 0;
	_transitionId = 0;
	_videoEventId = 0;
	_vocSecondsOffset = 0;
	_RTANum = 0;
	_RTVLimit = 0;
	_RTVNum = 0;
	_viewBounds = nullptr;
	Common::fill(&_evPicPtrs[0], &_evPicPtrs[6], (PictureResource *)nullptr);
	Common::fill(&_evCmPtrs[0], &_evCmPtrs[6], (CMapResource *)nullptr);
	_policeEvent = 0;

	_eventFlags = EVTFLAG_TIME_DISABLED;
	_fadingAmount1 = _fadingAmount2 = 127;
	_murderThreshold = 9999;
	_aptLoadMode = -1;
	_eventFlags |= EVTFLAG_100;
	_totalPhoneCalls = 0;

	for (int i = 0; i < 6; i++)
		_evPicPtrs[i] = nullptr;
	for (int i = 0; i < 1000; i++) {
		_events[i]._hour = 0;
		_events[i]._minute = 0;
		_events[i]._isAM = true;
		_events[i]._type = EVTYPE_NONE;
		_events[i]._audioVideoId = -1;
		_events[i]._computerOn = 0;
		_events[i]._computerOff = 0;
		_events[i]._dead = 0;
	}
	for (int i = 0; i < 6; i++)
		_evCmPtrs[i] = nullptr;
}

void SVoy::addEvent(int hour, int minute, VoyeurEventType type, int audioVideoId,
		int on, int off, int dead) {
	VoyeurEvent &e = _events[_eventCount++];

	e._type = type;
	e._hour = hour;
	e._minute = minute;
	e._isAM = hour < 12;
	e._audioVideoId = audioVideoId;
	e._computerOn = on;
	e._computerOff = off;
	e._dead = dead;
}

void SVoy::synchronize(Common::Serializer &s) {
	s.syncAsByte(_isAM);
	s.syncAsSint16LE(_RTANum);
	s.syncAsSint16LE(_RTVNum);
	s.syncAsSint16LE(_switchBGNum);

	_videoHotspotTimes.synchronize(s);
	_audioHotspotTimes.synchronize(s);
	_evidenceHotspotTimes.synchronize(s);

	int count = s.getVersion() == 1 ? 20 : 32;
	for (int idx = 0; idx < count; ++idx) {
		s.syncAsByte(_roomHotspotsEnabled[idx]);
	}

	s.syncAsSint16LE(_audioVisualStartTime);
	s.syncAsSint16LE(_audioVisualDuration);
	s.syncAsSint16LE(_vocSecondsOffset);
	s.syncAsSint16LE(_abortInterface);
	s.syncAsSint16LE(_playStampMode);
	s.syncAsSint16LE(_aptLoadMode);
	s.syncAsSint16LE(_transitionId);
	s.syncAsSint16LE(_RTVLimit);
	s.syncAsSint16LE(_eventFlags);
	s.syncAsSint16LE(_boltGroupId2);

	s.syncAsSint16LE(_musicStartTime);
	s.syncAsSint16LE(_totalPhoneCalls);
	s.syncAsSint16LE(_computerTextId);
	s.syncAsSint16LE(_computerTimeMin);
	s.syncAsSint16LE(_computerTimeMax);
	s.syncAsSint16LE(_victimMurdered);
	s.syncAsSint16LE(_murderThreshold);

	// Events
	s.syncAsUint16LE(_eventCount);
	for (int idx = 0; idx < _eventCount; ++idx)
		_events[idx].synchronize(s);

	s.syncAsSint16LE(_fadingAmount1);
	s.syncAsSint16LE(_fadingAmount2);
	s.syncAsSint16LE(_fadingStep1);
	s.syncAsSint16LE(_fadingStep2);
	s.syncAsSint16LE(_fadingType);
	s.skip(sizeof(int16), 0, 2);
	s.syncAsSint16LE(_incriminatedVictimNumber);
	s.syncAsSint16LE(_videoEventId);

	if (s.isLoading()) {
		// Reset apartment loading mode to initial game value
		_aptLoadMode = 140;
		_viewBounds = nullptr;
	}
}

void SVoy::addVideoEventStart() {
	VoyeurEvent &e = _events[_eventCount];
	e._hour = _vm->_gameHour;
	e._minute = _vm->_gameMinute;
	e._isAM = _isAM;
	e._type = EVTYPE_VIDEO;
	e._audioVideoId = _vm->_audioVideoId;
	e._computerOn = _vocSecondsOffset;
	e._dead = _vm->_eventsManager->_videoDead;
}

void SVoy::addVideoEventEnd() {
	VoyeurEvent &e = _events[_eventCount];
	e._computerOff = _RTVNum - _audioVisualStartTime - _vocSecondsOffset;
	if (_eventCount < (TOTAL_EVENTS - 1))
		++_eventCount;
}

void SVoy::addAudioEventStart() {
	VoyeurEvent &e = _events[_eventCount];
	e._hour = _vm->_gameHour;
	e._minute = _vm->_gameMinute;
	e._isAM = _isAM;
	e._type = EVTYPE_AUDIO;
	e._audioVideoId = _vm->_audioVideoId;
	e._computerOn = _vocSecondsOffset;
	e._dead = _vm->_eventsManager->_videoDead;
}

void SVoy::addAudioEventEnd() {
	VoyeurEvent &e = _events[_eventCount];
	e._computerOff = _RTVNum - _audioVisualStartTime - _vocSecondsOffset;
	if (_eventCount < (TOTAL_EVENTS - 1))
		++_eventCount;
}

void SVoy::addEvidEventStart(int v) {
	VoyeurEvent &e = _events[_eventCount];
	e._hour = _vm->_gameHour;
	e._minute = _vm->_gameMinute;
	e._isAM = _isAM;
	e._type = EVTYPE_EVID;
	e._audioVideoId = _vm->_playStampGroupId;
	e._computerOn = _boltGroupId2;
	e._computerOff = v;
}

void SVoy::addEvidEventEnd(int totalPages) {
	VoyeurEvent &e = _events[_eventCount];
	e._dead = totalPages;
	if (_eventCount < (TOTAL_EVENTS - 1))
		++_eventCount;
}

void SVoy::addComputerEventStart() {
	VoyeurEvent &e = _events[_eventCount];
	e._hour = _vm->_gameHour;
	e._minute = _vm->_gameMinute;
	e._isAM = _isAM;
	e._type = EVTYPE_COMPUTER;
	e._audioVideoId = _vm->_playStampGroupId;
	e._computerOn = _computerTextId;
}

void SVoy::addComputerEventEnd(int v) {
	VoyeurEvent &e = _events[_eventCount];
	e._computerOff = v;
	if (_eventCount < (TOTAL_EVENTS - 1))
		++_eventCount;
}

void SVoy::reviewAnEvidEvent(int eventIndex) {
	VoyeurEvent &e = _events[eventIndex];
	_vm->_playStampGroupId = e._audioVideoId;
	_boltGroupId2 = e._computerOn;
	int frameOff = e._computerOff;

	if (_vm->_bVoy->getBoltGroup(_vm->_playStampGroupId)) {
		_vm->_screen->_backColors = _vm->_bVoy->boltEntry(_vm->_playStampGroupId + 1)._cMapResource;
		_vm->_screen->_backgroundPage = _vm->_bVoy->boltEntry(_vm->_playStampGroupId)._picResource;
		_vm->_screen->_vPort->setupViewPort(_vm->_screen->_backgroundPage);
		_vm->_screen->_backColors->startFade();

		_vm->doEvidDisplay(frameOff, e._dead);
		_vm->_bVoy->freeBoltGroup(_vm->_playStampGroupId);
		_vm->_playStampGroupId = -1;

		if (_boltGroupId2 != -1) {
			_vm->_bVoy->freeBoltGroup(_boltGroupId2);
			_boltGroupId2 = -1;
		}
	}
}

void SVoy::reviewComputerEvent(int eventIndex) {
	VoyeurEvent &e = _events[eventIndex];
	_vm->_playStampGroupId = e._audioVideoId;
	_computerTextId = e._computerOn;

	if (_vm->_bVoy->getBoltGroup(_vm->_playStampGroupId)) {
		_vm->_screen->_backColors = _vm->_bVoy->boltEntry(_vm->_playStampGroupId + 1)._cMapResource;
		_vm->_screen->_backgroundPage = _vm->_bVoy->boltEntry(_vm->_playStampGroupId)._picResource;
		_vm->_screen->_vPort->setupViewPort(_vm->_screen->_backgroundPage);
		_vm->_screen->_backColors->startFade();
		_vm->flipPageAndWaitForFade();

		_vm->getComputerBrush();
		_vm->flipPageAndWait();
		_vm->doComputerText(e._computerOff);

		_vm->_bVoy->freeBoltGroup(0x4900);
		_vm->_bVoy->freeBoltGroup(_vm->_playStampGroupId);
		_vm->_playStampGroupId = -1;
	}
}

bool SVoy::checkForKey() {
	StateResource *state = _vm->_controlPtr->_state;
	state->_victimEvidenceIndex = 0;
	if (_vm->_voy->_victimMurdered)
		return false;

	for (int eventIdx = 0; eventIdx < _eventCount; ++eventIdx) {
		VoyeurEvent &e = _events[eventIdx];

		switch (e._type) {
		case EVTYPE_VIDEO:
			switch (state->_victimIndex) {
			case 1:
				if (e._audioVideoId == 33 && e._computerOn < 2 && e._computerOff >= 38)
					state->_victimEvidenceIndex = 1;
				break;

			case 2:
				if (e._audioVideoId == 47 && e._computerOn < 2 && e._computerOff >= 9)
					state->_victimEvidenceIndex = 2;
				break;

			case 3:
				if (e._audioVideoId == 46 && e._computerOn < 2 && e._computerOff > 2)
					state->_victimEvidenceIndex = 3;
				break;

			case 4:
				if (e._audioVideoId == 40 && e._computerOn < 2 && e._computerOff > 6)
					state->_victimEvidenceIndex = 4;
				break;

			default:
				break;
			}
			break;

		case EVTYPE_AUDIO:
			switch (state->_victimIndex) {
			case 1:
				if (e._audioVideoId == 8 && e._computerOn < 2 && e._computerOff > 26)
					state->_victimEvidenceIndex = 1;
				break;

			case 3:
				if (e._audioVideoId == 20 && e._computerOn < 2 && e._computerOff > 28)
					state->_victimEvidenceIndex = 3;
				if (e._audioVideoId == 35 && e._computerOn < 2 && e._computerOff > 18)
					state->_victimEvidenceIndex = 3;
				break;

			default:
				break;
			}
			break;

		case EVTYPE_EVID:
			switch (state->_victimIndex) {
			case 4:
				if (e._audioVideoId == 0x2400 && e._computerOn == 0x4f00 && e._computerOff == 17)
					state->_victimEvidenceIndex = 4;

			default:
				break;
			}
			break;

		case EVTYPE_COMPUTER:
			switch (state->_victimIndex) {
			case 2:
				if (e._computerOn == 13 && e._computerOff > 76)
					state->_victimEvidenceIndex = 2;
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}

		if (state->_victimEvidenceIndex == state->_victimIndex)
			return true;
	}

	return false;
}

} // End of namespace Voyeur
