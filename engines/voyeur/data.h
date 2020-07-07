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

#ifndef VOYEUR_DATA_H
#define VOYEUR_DATA_H

#include "common/scummsys.h"
#include "common/serializer.h"
#include "voyeur/files.h"

namespace Voyeur {

#define TOTAL_EVENTS 1000

enum VoyeurEventType {
	EVTYPE_NONE = 0,
	EVTYPE_VIDEO = 1,
	EVTYPE_AUDIO = 2,
	EVTYPE_EVID = 3,
	EVTYPE_COMPUTER = 4
};

enum EventFlag { EVTFLAG_TIME_DISABLED = 1, EVTFLAG_2 = 2, EVTFLAG_8 = 8, EVTFLAG_RECORDING = 0x10,
	EVTFLAG_40 = 0x40, EVTFLAG_VICTIM_PRESET = 0x80, EVTFLAG_100 = 0x100 };

struct VoyeurEvent {
	int _hour;
	int _minute;
	bool _isAM;
	VoyeurEventType _type;
	int _audioVideoId;
	int _computerOn;
	int _computerOff;
	int _dead;

	void synchronize(Common::Serializer &s);
};

class VoyeurEngne;

/**
 * Encapsulates a list of the time expired ranges that hotspots in the mansion
 * view are enabled for in a given time period.
 */
template<int SLOTS>
class HotspotTimes {
public:
	int _min[SLOTS][20];	// Min time expired
	int _max[SLOTS][20];	// Max time expired

	HotspotTimes() {
		reset();
	}

	/**
	 * Resets the data to an initial state
	 */
	void reset() {
		for (int hotspotIdx = 0; hotspotIdx < 20; ++hotspotIdx) {
			for (int slotIdx = 0; slotIdx < SLOTS; ++slotIdx) {
				_min[slotIdx][hotspotIdx] = 9999;
				_max[slotIdx][hotspotIdx] = 0;
			}
		}
	}

	/**
	 * Synchronise the data
	 */
	void synchronize(Common::Serializer &s) {
		for (int slotIndex = 0; slotIndex < SLOTS; ++slotIndex) {
			for (int hotspotIndex = 0; hotspotIndex < 20; ++hotspotIndex) {
				s.syncAsSint16LE(_min[slotIndex][hotspotIndex]);
				s.syncAsSint16LE(_max[slotIndex][hotspotIndex]);
			}
		}
	}

	/**
	 * Returns true if the given value is in the range specified by the
	 * min and max at the given hotspot and slot indexes
	 */
	bool isInRange(int slotIndex, int hotspotIndex, int v) const {
		return _min[slotIndex][hotspotIndex] <= v &&
			v < _max[slotIndex][hotspotIndex];
	}
};

class SVoy {
private:
	VoyeurEngine *_vm;

public:
	bool _abortInterface;
	bool _isAM;
	bool _phoneCallsReceived[5];
	bool _roomHotspotsEnabled[32];
	bool _victimMurdered;

	int _aptLoadMode;
	int _audioVisualStartTime;
	int _audioVisualDuration;
	int _boltGroupId2;
	int _computerTextId;
	int _computerTimeMin;
	int _computerTimeMax;
	int _eventCount;
	int _eventFlags;
	int _fadingAmount1;
	int _fadingAmount2;
	int _fadingStep1;
	int _fadingStep2;
	int _fadingType;
	int _incriminatedVictimNumber;
	int _murderThreshold;
	int _musicStartTime;
	int _playStampMode;
	int _switchBGNum;
	int _totalPhoneCalls;
	int _transitionId;
	int _videoEventId;
	int _vocSecondsOffset;
	int _RTANum;
	int _RTVLimit;
	int _RTVNum;

	HotspotTimes<3> _audioHotspotTimes;
	HotspotTimes<3> _evidenceHotspotTimes;
	HotspotTimes<8> _videoHotspotTimes;

	Common::Rect _computerScreenRect;
	RectResource *_viewBounds;
	PictureResource *_evPicPtrs[6];
	CMapResource *_evCmPtrs[6];
	VoyeurEvent _events[TOTAL_EVENTS];

	SVoy(VoyeurEngine *vm);

	/**
	 * Synchronize the data
	 */
	void synchronize(Common::Serializer &s);

	/**
	 * Add an event to the list of game events that have occurred
	 */
	void addEvent(int hour, int minute, VoyeurEventType type, int audioVideoId,
		int on, int off, int dead);

	/**
	 * Adds the start of a video event happening
	 */
	void addVideoEventStart();

	/**
	 * Adds the finish of a video event happening
	 */
	void addVideoEventEnd();

	/**
	 * Adds the start of an audio event happening
	 */
	void addAudioEventStart();

	/**
	 * Adsd the finish of an audio event happening
	 */
	void addAudioEventEnd();

	/**
	 * Adds the start of an evidence event happening
	 */
	void addEvidEventStart(int v);

	/**
	 * Adds the finish of an evidence event happening
	 */
	void addEvidEventEnd(int totalPages);

	/**
	 * Adds the start of a computer event happening
	 */
	void addComputerEventStart();

	/**
	 * Adds the finish of a computer event happening
	 */
	void addComputerEventEnd(int v);

	/**
	 * Review a previously recorded evidence event
	 */
	void reviewAnEvidEvent(int eventIndex);

	/**
	 * Review a previously recorded computer event
	 */
	void reviewComputerEvent(int eventIndex);

	/**
	 * Checks for key information in determining what kind of murder
	 * should take place
	 */
	bool checkForKey();

private:
	int _policeEvent;
};

} // End of namespace Voyeur

#endif /* VOYEUR_DATA_H */
