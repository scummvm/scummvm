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

#ifndef VOYEUR_DATA_H
#define VOYEUR_DATA_H

#include "common/scummsys.h"
#include "common/serializer.h"
#include "voyeur/files.h"

namespace Voyeur {

#define TOTAL_EVENTS 1000

enum VoyeurEventType { EVTYPE_VIDEO = 1, EVTYPE_AUDIO = 2, EVTYPE_EVID = 3,
	EVTYPE_COMPUTER = 4 };

struct VoyeurEvent {
	int _hour;
	int _minute;
	bool _isAM;
	VoyeurEventType _type;
	int _videoId;
	int _computerOn;
	int _computerOff;
	int _dead;

	void synchronize(Common::Serializer &s);
};

class VoyeurEngne;

class SVoy {
private:
	VoyeurEngine *_vm;
public:
	bool _isAM;
	int _RTANum;
	int _RTVNum;
	int _switchBGNum;
	int _arr1[8][20];
	int _arr2[8][20];
	int _arr3[3][20];
	int _arr4[3][20];
	int _arr5[3][20];
	int _arr6[3][20];
	int _arr7[20];

	int _field468;
	int _field46A;
	int _vocSecondsOffset;
	bool _field46E;
	int _field470;
	int _aptLoadMode;
	int _transitionId;
	int _RTVLimit;
	int _field478;
	int _field47A;
	PictureResource *_evPicPtrs[6];
	CMapResource *_evCmPtrs[6];
	int _field4AC;
	int _field4AE[5];
	int _field4B8;

	int _computerTextId;
	Common::Rect _rect4E4;
	int _field4EC;
	int _field4EE;
	int _field4F0;
	int _field4F2;

	/**
	 * Total number of game events that have occurred
	 */
	int _eventCount;

	/**
	 * List of game events that have occurred
	 */
	VoyeurEvent _events[TOTAL_EVENTS];

	int _field4376;
	int _field4378;
	int _field437A;
	int _field437C;
	int _field437E;
	int _field4380;
	int _field4382;
	int _videoEventId;
	RectResource *_viewBounds;
	int _curICF0;
	int _curICF1;
	int _fadeICF0;
	int _policeEvent;
public:
	void setVm(VoyeurEngine *vm);

	/**
	 * Synchronise the data
	 */
	void synchronize(Common::Serializer &s);

	/**
	 * Add an event to the list of game events that have occurred
	 */
	void addEvent(int hour, int minute, VoyeurEventType type, int videoId, int on,
		int off, int dead);

	void addVideoEventStart();
	void addVideoEventEnd();
	void addAudioEventStart();
	void addAudioEventEnd();
	void addEvidEventStart(int v);
	void addEvidEventEnd(int dead);
	void addComputerEventStart();
	void addComputerEventEnd(int v);
	void reviewAnEvidEvent(int eventIndex);
	void reviewComputerEvent(int eventIndex);
};

} // End of namespace Voyeur

#endif /* VOYEUR_DATA_H */
