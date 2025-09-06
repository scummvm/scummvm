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

#ifndef BAGEL_MFC_LIBS_EVENTS_H
#define BAGEL_MFC_LIBS_EVENTS_H

#include "common/events.h"
#include "bagel/mfc/minwindef.h"

namespace Bagel {
namespace MFC {
namespace Libs {

class Event : public Common::Event {
private:
	static int _mouseX, _mouseY;
	static int _flags;

public:
	static void init();

	operator MSG() const;
};

class EventQueue {
private:
	Common::Array<MSG> _queue;

public:
	/**
	 * Returns true if the queue is empty
	 * @return  true if empty
	 */
	bool empty() const {
		return _queue.empty();
	}

	/**
	 * Adds a message to the queue
	 * @param msg   Message to add
	*/
	void push(const MSG &msg) {
		_queue.insert_at(0, msg);
	}

	/**
	 * Removes a message from the queue.
	 * @return
	*/
	MSG pop();

	/**
	 * Checks the queue for a given message
	 */
	bool peekMessage(MSG *lpMsg, HWND hWnd,
		unsigned int wMsgFilterMin, unsigned int wMsgFilterMax,
		unsigned int wRemoveMsg);
};

} // namespace Libs
} // namespace MFC
} // namespace Bagel

#endif
