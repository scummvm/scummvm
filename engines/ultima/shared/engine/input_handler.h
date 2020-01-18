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

#ifndef ULTIMA_SHARED_ENGINE_INPUT_HANDLER_H
#define ULTIMA_SHARED_ENGINE_INPUT_HANDLER_H

#include "ultima/shared/engine/input_translator.h"
#include "ultima/shared/core/tree_item.h"

namespace Ultima {
namespace Shared {

class GameBase;

class InputHandler {
private:
	/**
	 * Process and dispatch a passed message
	 */
	void processMessage(CMessage *msg);

	/**
	 * Dispatches a message to the project
	 */
	void dispatchMessage(CMessage *msg);
public:
	GameBase *_game;
	InputTranslator *_inputTranslator;
	bool _dragging;
	bool _buttonDown;
	Point _mousePos;
	Point _dragStartPos;
	int _lockCount;
	bool _abortMessage;
public:
	InputHandler(GameBase *game);
	~InputHandler();

	void setTranslator(InputTranslator *translator);

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
	void handleMessage(CMessage &msg, bool respectLock = true);
};

} // End of namespace Shared
} // End of namespace Ultima

#endif
