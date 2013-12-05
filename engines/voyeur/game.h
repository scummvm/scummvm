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

#ifndef VOYEUR_GAME_H
#define VOYEUR_GAME_H

#include "common/scummsys.h"
#include "common/events.h"
#include "common/str.h"

namespace Voyeur {

class VoyeurEngine;

class Event {
public:
	int _hours;
	int _minutes;
	int _seconds;
	int _type;
	int _data1;
	int _data2;
	byte *_data;
};

class SVoy {
public:
	int _delaySecs;
	int _RTANum;
	int _RTVNum;
	int _switchBGNum;
	int _group;
	int _resolvePtr;
	int _seconds;
	int _minutes;
	int _hours;
	int _morning;
	int _timeChangeFlag;
	int _totalSeconds;
	int _gameSeconds;
	int _vCursorOn[160];
	int _vCursorOff[160];
	int _aCursorOn[60];
	int _aCursorOff[60];
	int _eCursorOn[60];
	int _eCursorOff[60];
	int _timeStart;
	int _duration;
	int _vidStart;
	int _doApt;
	int _function;
	int _anim;
	int _level;
	int _levelDone;
	int _flags;
	int _evGroup;
	byte *_evPicPtrs[6];
	byte *_evCmPtrs[6];
	int _audioTime;
	int _phones[5];
	int _numPhonesUsed;
	int _evidence[20];
	int _computerNum;
	int _computerBut;
	int _computerOn;
	int _computerOff;
	int _dead;
	int _deadTime;
	int _eventCnt;
	Event _eventTable[1000];
	int _curICF0;
	int _curICF1;
	int _fadeICF0;
	int _fadeICF1;
	int _fadeFunc;
	int _lastInplay;
	int _incriminate;
	int _policeEvent;
};

class IntData {
public:
	bool _field9;
	bool _flipWait;
	int field16;
	int field1A;
	int field1E;
	int field22;
	int field24;
	int field26;
	int field2A;
	bool _hasPalette;
	int field38;
	int field3B;
	int field3D;
	int _palStartIndex;
	int _palEndIndex;
	byte *_palette;
public:
	IntData();

	void audioInit();
};

class Game {
private:
	VoyeurEngine *_vm;
public:
	int _v2A098;
	int _v2A0A6;
	int _v2A0A4;
	int _v2A09A;
	int _iForceDeath;
public:
	Game();
	void setVm(VoyeurEngine *vm) { _vm = vm; }

	void doTransitionCard(const Common::String &time, const Common::String &location);
	void addVideoEventStart();
	void playStamp();
};

} // End of namespace Voyeur

#endif /* VOYEUR_GAME_H */
