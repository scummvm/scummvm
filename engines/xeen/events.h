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

#ifndef XEEN_EVENTS_H
#define XEEN_EVENTS_H

#include "common/scummsys.h"
#include "common/events.h"

namespace Xeen {

#define GAME_FRAME_RATE (1000 / 18.2)

class XeenEngine;

class EventsManager {
private:
	XeenEngine *_vm;
	uint32 _frameCounter;
	uint32 _priorFrameCounterTime;
	uint32 _gameCounter;
	uint32 _priorGameCounterTime;
	Common::KeyCode _keyCode;
	bool _leftButton, _rightButton;

	void nextFrame();
public:
	EventsManager(XeenEngine *vm);

	~EventsManager();

	uint32 getFrameCounter() { return _frameCounter; }

	void showCursor();

	void hideCursor();

	bool isCursorVisible();

	void pollEvents();

	void pollEventsAndWait();

	void clearEvents();

	void debounceMouse();

	bool getKey(Common::KeyState &key);

	bool isKeyPending() const;

	bool isKeyMousePressed();

	void updateGameCounter();

	uint32 timeElapsed();
};

} // End of namespace Xeen

#endif /* XEEN_EVENTS_H */
