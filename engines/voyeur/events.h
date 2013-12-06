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

#ifndef VOYEUR_EVENTS_H
#define VOYEUR_EVENTS_H

#include "common/scummsys.h"
#include "common/list.h"
#include "graphics/surface.h"
#include "voyeur/files.h"

namespace Voyeur {

class VoyeurEngine;
class EventsManager;
class CMapResource;

#define GAME_FRAME_RATE 50
#define GAME_FRAME_TIME (1000 / GAME_FRAME_RATE)

typedef void (EventsManager::*EventMethodPtr)();
 
class IntNode {
public:
	EventMethodPtr _intFunc;
	uint16 _curTime;
	uint16 _timeReset;
	uint32 _flags;
public:
	IntNode();
	IntNode(uint16 curTime, uint16 timeReset, uint16 flags);
};

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

class EventsManager {
private:
	VoyeurEngine *_vm;
	uint32 _priorFrameTime;
	uint32 _gameCounter;
	bool _keyState[256];
	int _mouseButton;
	Common::List<IntNode *> _intNodes;
	Common::Point _mousePos;

	void mainVoyeurIntFunc();
private:
	void checkForNextFrameCounter();
	void voyeurTimer();
	void videoTimer();
	void vDoFadeInt();
	void vDoCycleInt();
	void fadeIntFunc();
public:
	IntData _gameData;
	IntData &_intPtr;
	IntNode _fadeIntNode;
	IntNode _fade2IntNode;
	IntNode _cycleIntNode;
	IntNode _evintnode;
	IntNode _mainIntNode;
	int _cycleStatus;
	int _fadeFirstCol, _fadeLastCol;
	int _fadeCount;
	int _fadeStatus;
public:
	EventsManager();
	void setVm(VoyeurEngine *vm) { _vm = vm; }

	void resetMouse();
	void startMainClockInt();
	void vStopCycle();
	void sWaitFlip();
	void vInitColor();

	void delay(int cycles);
	void pollEvents();
	void startFade(CMapResource *cMap);
	void addIntNode(IntNode *node);
	void addFadeInt();

	void setCursor(PictureResource *pic);
	void setCursor(byte *cursorData, int width, int height);
	void setCursorColor(int idx, int mode);
	void mouseOn();
	void mouseOff();
	Common::Point getMousePos() { return _mousePos; }
};

} // End of namespace Voyeur

#endif /* VOYEUR_EVENTS_H */
