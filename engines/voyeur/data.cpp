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

#include "voyeur/data.h"
#include "voyeur/voyeur.h"

namespace Voyeur {

void VoyeurEvent::synchronize(Common::Serializer &s) {
	s.syncAsByte(_hour);
	s.syncAsByte(_minute);
	s.syncAsByte(_isAM);
	s.syncAsByte(_type);
	s.syncAsSint16LE(_videoId);
	s.syncAsSint16LE(_computerOn);
	s.syncAsSint16LE(_computerOff);
	s.syncAsSint16LE(_dead);
}

/*------------------------------------------------------------------------*/

void SVoy::setVm(VoyeurEngine *vm) {
	_vm = vm;
}

void SVoy::addEvent(int hour, int minute, VoyeurEventType type, int videoId, 
		int on, int off, int dead) {
	VoyeurEvent &e = _events[_eventCount++];

	e._hour = hour;
	e._minute = minute;
	e._isAM = hour < 12;
	e._videoId = videoId;
	e._computerOn = on;
	e._computerOff = off;
	e._dead = dead;
}

void SVoy::synchronize(Common::Serializer &s) {
	s.syncAsByte(_isAM);
	s.syncAsSint16LE(_RTANum);
	s.syncAsSint16LE(_RTVNum);
	s.syncAsSint16LE(_switchBGNum);

	for (int v1 = 0; v1 < 8; ++v1) {
		for (int v2 = 0; v2 < 20; ++v2) {
			s.syncAsSint16LE(_arr1[v1][v2]);
			s.syncAsSint16LE(_arr2[v1][v2]);
		}
	}
	for (int v1 = 0; v1 < 4; ++v1) {
		for (int v2 = 0; v2 < 20; ++v2) {
			s.syncAsSint16LE(_arr3[v1][v2]);
			s.syncAsSint16LE(_arr4[v1][v2]);
		}
	}
	for (int v1 = 0; v1 < 4; ++v1) {
		for (int v2 = 0; v2 < 20; ++v2) {
			s.syncAsSint16LE(_arr5[v1][v2]);
			s.syncAsSint16LE(_arr6[v1][v2]);
		}
	}
	for (int v1 = 0; v1 < 20; ++v1) {
		s.syncAsSint16LE(_arr7[20]);
	}

	s.syncAsSint16LE(_field468);
	s.syncAsSint16LE(_field46A);
	s.syncAsSint16LE(_vocSecondsOffset);
	s.syncAsSint16LE(_field46E);
	s.syncAsSint16LE(_field470);
	s.syncAsSint16LE(_aptLoadMode);
	s.syncAsSint16LE(_transitionId);
	s.syncAsSint16LE(_RTVLimit);
	s.syncAsSint16LE(_field478);
	s.syncAsSint16LE(_field47A);

	s.syncAsSint16LE(_field4AC);
	s.syncAsSint16LE(_field4B8);
	s.syncAsSint16LE(_computerTextId);
	s.syncAsSint16LE(_field4EC);
	s.syncAsSint16LE(_field4EE);
	s.syncAsSint16LE(_field4F0);
	s.syncAsSint16LE(_field4F2);

	// Events
	s.syncAsUint16LE(_eventCount);
	for (int idx = 0; idx < _eventCount; ++idx)
		_events[idx].synchronize(s);

	s.syncAsSint16LE(_field4376);
	s.syncAsSint16LE(_field4378);
	s.syncAsSint16LE(_field437A);
	s.syncAsSint16LE(_field437C);
	s.syncAsSint16LE(_field437E);
	s.syncAsSint16LE(_field4380);
	s.syncAsSint16LE(_field4382);
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
	e._videoId = _vm->_videoId;
	e._computerOn = _vocSecondsOffset;
	e._dead = _vm->_eventsManager._videoDead;
}

void SVoy::addVideoEventEnd() {
	VoyeurEvent &e = _events[_eventCount];
	e._computerOff = _RTVNum - _field468 - _vocSecondsOffset;
	if (_eventCount < (TOTAL_EVENTS - 1))
		++_eventCount;
}

void SVoy::addAudioEventStart() {
	VoyeurEvent &e = _events[_eventCount];
	e._hour = _vm->_gameHour;
	e._minute = _vm->_gameMinute;
	e._isAM = _isAM;
	e._type = EVTYPE_AUDIO;
	e._videoId = _vm->_videoId;
	e._computerOn = _field47A;
	e._dead = _vm->_eventsManager._videoDead;
}

void SVoy::addAudioEventEnd() {
	VoyeurEvent &e = _events[_eventCount];
	e._computerOff = _RTVNum - _field468 - _vocSecondsOffset;
	if (_eventCount < (TOTAL_EVENTS - 1))
		++_eventCount;
}

void SVoy::addEvidEventStart(int v) {
	VoyeurEvent &e = _events[_eventCount];
	e._hour = _vm->_gameHour;
	e._minute = _vm->_gameMinute;
	e._isAM = _isAM;
	e._type = EVTYPE_EVID;
	e._videoId = _vm->_videoId;
	e._computerOn = _vocSecondsOffset;
	e._dead = _vm->_eventsManager._videoDead;

}

void SVoy::addEvidEventEnd(int dead) {
	VoyeurEvent &e = _events[_eventCount];
	e._dead = dead;
	if (_eventCount < (TOTAL_EVENTS - 1))
		++_eventCount;
}

void SVoy::addComputerEventStart() {
	VoyeurEvent &e = _events[_eventCount];
	e._hour = _vm->_gameHour;
	e._minute = _vm->_gameMinute;
	e._isAM = _isAM;
	e._type = EVTYPE_COMPUTER;
	e._videoId = _vm->_playStampGroupId;
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
	_vm->_playStampGroupId = e._videoId;
	_field47A = e._computerOn;
	int frameOff = e._computerOff;

	if (_vm->_bVoy->getBoltGroup(_vm->_playStampGroupId)) {
		_vm->_graphicsManager._backColors = _vm->_bVoy->boltEntry(_vm->_playStampGroupId + 1)._cMapResource;
		_vm->_graphicsManager._backgroundPage = _vm->_bVoy->boltEntry(_vm->_playStampGroupId)._picResource;
		(*_vm->_graphicsManager._vPort)->setupViewPort(_vm->_graphicsManager._backgroundPage);
		_vm->_graphicsManager._backColors->startFade();

		_vm->doEvidDisplay(frameOff, e._dead);
		_vm->_bVoy->freeBoltGroup(_vm->_playStampGroupId);
		_vm->_playStampGroupId = -1;

		if (_field47A != -1) {
			_vm->_bVoy->freeBoltGroup(_field47A);
			_field47A = -1;
		}
	}
}

void SVoy::reviewComputerEvent(int eventIndex) {
	VoyeurEvent &e = _events[eventIndex];
	_vm->_playStampGroupId = e._videoId;
	_computerTextId = e._computerOn;

	if (_vm->_bVoy->getBoltGroup(_vm->_playStampGroupId)) {
		_vm->_graphicsManager._backColors = _vm->_bVoy->boltEntry(_vm->_playStampGroupId + 1)._cMapResource;
		_vm->_graphicsManager._backgroundPage = _vm->_bVoy->boltEntry(_vm->_playStampGroupId)._picResource;
		(*_vm->_graphicsManager._vPort)->setupViewPort(_vm->_graphicsManager._backgroundPage);
		_vm->_graphicsManager._backColors->startFade();
		_vm->flipPageAndWaitForFade();

		_vm->getComputerBrush();
		_vm->flipPageAndWait();
		_vm->doComputerText(e._computerOff);

		_vm->_bVoy->freeBoltGroup(0x4900);
		_vm->_bVoy->freeBoltGroup(_vm->_playStampGroupId);
		_vm->_playStampGroupId = -1;
	}
}

} // End of namespace Voyeur
