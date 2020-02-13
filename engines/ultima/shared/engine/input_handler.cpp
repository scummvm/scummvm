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

#include "ultima/shared/engine/input_handler.h"
#include "ultima/shared/engine/ultima.h"
#include "ultima/shared/engine/events.h"
#include "ultima/shared/early/game_base.h"
#include "ultima/shared/engine/messages.h"
#include "ultima/shared/gfx/visual_item.h"

namespace Ultima {
namespace Shared {

InputHandler::InputHandler(GameBase *game) : _game(game), _inputTranslator(nullptr), _dragging(false),
		_buttonDown(false), _lockCount(0), _abortMessage(false) {
}

InputHandler::~InputHandler() {
}

void InputHandler::setTranslator(InputTranslator *translator) {
	_inputTranslator = translator;
}

void InputHandler::incLockCount() {
	++_lockCount;
}

void InputHandler::decLockCount() {
	--_lockCount;
	assert(_lockCount >= 0);

	if (_lockCount == 0 && _inputTranslator) {
		if (_dragging && !_inputTranslator->isMousePressed()) {
			CMouseButtonUpMsg upMsg(_mousePos, MK_LBUTTON);
			handleMessage(upMsg);
		}

		_buttonDown = _inputTranslator->isMousePressed();
		_abortMessage = true;
	}
}

void InputHandler::handleMessage(CMessage &msg, bool respectLock) {
	if (!respectLock || _lockCount <= 0) {
		processMessage(&msg);
	}
}

void InputHandler::processMessage(CMessage *msg) {
	const CMouseMsg *mouseMsg = dynamic_cast<const CMouseMsg *>(msg);
	_abortMessage = false;
	dispatchMessage(msg);

	if (_abortMessage) {
		_abortMessage = false;
	} else if (mouseMsg) {
		// Keep the game state mouse position up to date
		if (_mousePos != mouseMsg->_mousePos) {
			_mousePos = mouseMsg->_mousePos;
		}

		// Set flag for whether a mouse button is currently being pressed
		if (mouseMsg->isButtonDownMsg())
			_buttonDown = true;
		else if (mouseMsg->isButtonUpMsg())
			_buttonDown = false;

		// Drag events generation
		if (_dragging) {
			if (mouseMsg->isMouseMoveMsg()) {
				CMouseDragMsg moveMsg(_mousePos, mouseMsg->_buttons);
				dispatchMessage(&moveMsg);
			} else if (mouseMsg->isButtonUpMsg()) {
				_dragging = false;
			}
		} else if (_buttonDown) {
			if (!mouseMsg->isMouseMoveMsg()) {
				// Save where the drag movement started from
				_dragStartPos = _mousePos;
			} else {
				Point delta = _mousePos - _dragStartPos;
				int distance = (int)sqrt(double(delta.x * delta.x + delta.y * delta.y));

				if (distance > 4) {
					// A drag has started
					_dragging = true;
				}
			}
		}
	}
}

void InputHandler::dispatchMessage(CMessage *msg) {
	Gfx::VisualItem *view = _game->getView();
	if (view)
		msg->execute(view);
}

} // End of namespace Shared
} // End of namespace Ultima
