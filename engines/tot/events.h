/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef TOT_EVENTS_H
#define TOT_EVENTS_H

#include "common/events.h"
#include "common/scummsys.h"

namespace Tot {

enum GAME_KEY {
	KEY_TALK = 0,
	KEY_PICKUP = 1,
	KEY_LOOKAT = 2,
	KEY_USE = 3,
	KEY_OPEN = 4,
	KEY_CLOSE = 5,
	KEY_YES = 6,
	KEY_NO = 7,
	KEY_SAVELOAD = 8,
	KEY_VOLUME = 9,
	KEY_ESCAPE = 10,
	KEY_NONE = -1
};

class TotEventManager {
private:
	Common::Event _event;

	void handleKey(const Common::Event &event);
public:
	bool _escKeyFl = false;
	bool _keyPressed = false;
	GAME_KEY _gameKey = KEY_NONE;
	bool _leftMouseButton = 0;
	bool _rightMouseButton = 0;
	int16 _mouseX = 0;
	int16 _mouseY = 0;
	uint16 _lastChar = '\0';

	TotEventManager();

	void pollEvent(bool allowDrag = false);
	void zeroEvents(bool allowDrag = false);
	void waitForPress();

};

} // End of namespace Tot
#endif
