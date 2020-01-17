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

#ifndef TITANIC_INPUT_HANDLER_H
#define TITANIC_INPUT_HANDLER_H

#include "titanic/support/rect.h"

namespace Titanic {

class CGameObject;
class CGameManager;
class CInputTranslator;
class CMessage;
class CTreeItem;

class CInputHandler {
private:
	/**
	 * Process and dispatch a passed message
	 * @returns		True if message was handled
	 */
	bool processMessage(CMessage *msg);

	/**
	 * Dispatches a message to the project
	 * @returns		True if message was handled
	 */
	bool dispatchMessage(CMessage *msg);

	/**
	 * Called when a drag operation has ended
	 */
	CGameObject *dragEnd(const Point &pt, CTreeItem *dragItem);
public:
	CGameManager *_gameManager;
	CInputTranslator *_inputTranslator;
	bool _dragging;
	bool _buttonDown;
	CTreeItem *_dragItem;
	Point _dragStartPos;
	Point _mousePos;
	int _lockCount;
	bool _abortMessage;
public:
	CInputHandler(CGameManager *owner);
	~CInputHandler();

	void setTranslator(CInputTranslator *translator);

	/**
	 * Increment the lock count
	 */
	void incLockCount();

	/**
	 * Decrement the lock count on the input handler
	 */
	void decLockCount();

	/**
	 * Handles a genereated mouse message
	 */
	bool handleMessage(CMessage &msg, bool respectLock = true);

	/**
	 * Returns true if input is locked
	 */
	bool isLocked() const { return _lockCount > 0; }
};

} // End of namespace Titanic

#endif /* TITANIC_INPUT_HANDLER_H */
