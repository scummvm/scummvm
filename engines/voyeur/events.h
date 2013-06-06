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
#include "voyeur/game.h"

namespace Voyeur {

class VoyeurEngine;
class CMapResource;

#define GAME_FRAME_RATE 50
#define GAME_FRAME_TIME (1000 / GAME_FRAME_RATE)

class EventsManager {
private:
	VoyeurEngine *_vm;
	uint32 _priorFrameTime;
	uint32 _gameCounter;
	bool _keyState[256];
	int _mouseButton;

	static void mainVoyeurIntFunc();
private:
	void checkForNextFrameCounter();
	void videoTimer();
public:
	IntData _audioStruc;
	IntData &_intPtr;
	IntNode _fadeIntNode;
	IntNode _cycleIntNode;
	IntNode _evintnode;
	IntNode _mainIntNode;
	int _cycleStatus;
	int _fadeFirstCol, _fadeLastCol;
	int _fadeCount;
public:
	EventsManager();
	void setVm(VoyeurEngine *vm) { _vm = vm; }

	void resetMouse();
	void startMainClockInt();
	void vStopCycle();
	void sWaitFlip();

	void delay(int cycles);
	void pollEvents();
	void startFade(CMapResource *cMap);
};

} // End of namespace Voyeur

#endif /* VOYEUR_EVENTS_H */
