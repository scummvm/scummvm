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

	IntNode();
	IntNode(uint16 curTime, uint16 timeReset, uint16 flags);
};

class IntData {
public:
	bool _flipWait;
	int _flashTimer;
	int _flashStep;
	bool _hasPalette;
	bool _skipFading;
	int _palStartIndex;
	int _palEndIndex;
	byte *_palette;

	IntData();
};

class EventsManager {
private:
	VoyeurEngine *_vm;
	bool _counterFlag;
	bool _cursorBlinked;
	uint32 _gameCounter;
	uint32 _priorFrameTime;
	uint32 _recordBlinkCounter;	// Original field was called _joe :)
	Common::List<IntNode *> _intNodes;
	Common::Point _mousePos;

	void mainVoyeurIntFunc();
	void checkForNextFrameCounter();
	void voyeurTimer();
	void videoTimer();
	void vDoFadeInt();
	void vDoCycleInt();
	void fadeIntFunc();
	void addIntNode(IntNode *node);
	void deleteIntNode(IntNode *node);

	/**
	 * Debugger support method to show the mouse position
	 */
	void showMousePosition();
public:
	IntData _gameData;
	IntData &_intPtr;
	IntNode _fadeIntNode;
	IntNode _fade2IntNode;
	IntNode _cycleIntNode;
	IntNode _evIntNode;
	IntNode _mainIntNode;
	int _cycleStatus;
	int _fadeFirstCol, _fadeLastCol;
	int _fadeCount;
	int _fadeStatus;

	bool _leftClick, _rightClick;
	bool _mouseClicked, _newMouseClicked;
	bool _newLeftClick, _newRightClick;

	int _videoDead;
	int _cycleTime[4];
	byte *_cycleNext[4];
	VInitCycleResource *_cyclePtr;

	EventsManager(VoyeurEngine *vm);

	void setMousePos(const Common::Point &p) { _mousePos = p; }
	void startMainClockInt();
	void sWaitFlip();
	void vInitColor();

	void delay(int cycles);
	void delayClick(int cycles);
	void pollEvents();
	void startFade(CMapResource *cMap);
	void addFadeInt();

	void setCursor(PictureResource *pic);
	void setCursor(byte *cursorData, int width, int height, int keyColor);
	void setCursorColor(int idx, int mode);
	void showCursor();
	void hideCursor();
	Common::Point getMousePos() { return _mousePos; }
	uint32 getGameCounter() const { return _gameCounter; }
	void getMouseInfo();
	void startCursorBlink();
	void incrementTime(int amt);

	void stopEvidDim();

	Common::String getEvidString(int eventIndex);
};

} // End of namespace Voyeur

#endif /* VOYEUR_EVENTS_H */
