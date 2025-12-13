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
#ifndef PELROCK_EVENTS_H
#define PELROCK_EVENTS_H

#include "common/events.h"
#include "common/scummsys.h"

namespace Pelrock {
static const int kDoubleClickDelay = 300; // in milliseconds
class PelrockEventManager {
private:
	Common::Event _event;
	bool _leftMouseButton = 0;
	bool _rightMouseButton = 0;
	uint32 _clickTime = 0;
public:
	int16 _mouseX = 0;
	int16 _mouseY = 0;
	int16 _mouseClickX = 0;
	int16 _mouseClickY = 0;
	bool _leftMouseClicked = false;
	bool _longClicked = false;
	bool _rightMouseClicked = false;
	Common::Event _lastKeyEvent;
	PelrockEventManager();
	void pollEvent();
	void waitForKey();
};

} // End of namespace Pelrock
#endif
