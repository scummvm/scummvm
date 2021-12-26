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

#ifndef BACKEND_EVENTS_DS_H
#define BACKEND_EVENTS_DS_H

#include "common/events.h"

/**
 * The Nintendo DS event source.
 */
class DSEventSource : public Common::EventSource {
public:
	DSEventSource() : _firstPoll(true), _handleTouch(true) {}

	/**
	 * Gets and processes events.
	 */
	virtual bool pollEvent(Common::Event &event);

	virtual void handleTouch(bool enabled) { _handleTouch = enabled; }

protected:
	Common::Queue<Common::Event> _eventQueue;
	Common::Point _lastTouch;
	bool _firstPoll;
	bool _handleTouch;

	void addEventsToQueue();
	void addJoyButtonEvent(u32 keysPressed, u32 keysReleased, u32 ndsKey, uint8 svmButton);
};

#endif
