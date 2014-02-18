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

#ifndef HOPKINS_EVENTS_H
#define HOPKINS_EVENTS_H

#include "common/scummsys.h"
#include "common/events.h"
#include "common/str.h"

namespace Hopkins {

#define GAME_FRAME_RATE 50
#define GAME_FRAME_TIME (1000 / GAME_FRAME_RATE)

class HopkinsEngine;

enum DIALOG_KEY { KEY_NONE = 0, KEY_INVENTORY = 1, KEY_OPTIONS = 2, KEY_SAVE = 3, KEY_LOAD = 4 };

class EventsManager {
private:
	int _oldIconId;
	uint32 _priorCounterTime;
	uint32 _priorFrameTime;
	bool _keyState[256];
	bool _mouseLinuxFl;
	int _mouseSizeX, _mouseSizeY;

	HopkinsEngine *_vm;

	void pollEvents();
	void handleKey(const Common::Event &event);
	void checkForNextFrameCounter();
	void updateCursor();

public:
	DIALOG_KEY _gameKey;
	uint32 _rateCounter;
	uint32 _gameCounter;
	bool _escKeyFl;
	bool _mouseFl;
	bool _breakoutFl;
	Common::Point _startPos;
	Common::Point _mousePos;
	Common::Point _mouseOffset;
	int _mouseSpriteId;
	int _curMouseButton;
	int _mouseButton;
	int _mouseCursorId;
	byte *_objectBuf;
	byte *_mouseCursor;

	EventsManager(HopkinsEngine *vm);
	~EventsManager();
	void clearAll();
	void initMouseData();

	void delay(int totalMilli);
	void changeMouseCursor(int id);
	void refreshEvents();
	int waitKeyPress();
	int getMouseX();
	int getMouseY();
	int getMouseButton();
	void setMouseXY(Common::Point pos);
	void setMouseXY(int xp, int yp);
	void mouseOn();
	void mouseOff();
	void setMouseOn();
	void refreshScreenAndEvents();
};

} // End of namespace Hopkins

#endif /* HOPKINS_EVENTS_H */
