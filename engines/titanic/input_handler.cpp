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

#include "titanic/input_handler.h"
#include "titanic/events.h"
#include "titanic/game_manager.h"
#include "titanic/core/project_item.h"
#include "titanic/messages/mouse_messages.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/support/files_manager.h"
#include "titanic/support/screen_manager.h"
#include "titanic/titanic.h"

namespace Titanic {

CInputHandler::CInputHandler(CGameManager *owner) :
		_gameManager(owner), _inputTranslator(nullptr), _dragging(false),
		_buttonDown(false), _dragItem(nullptr),  _lockCount(0),
		_abortMessage(false) {
	CScreenManager::_screenManagerPtr->_inputHandler = this;
}

CInputHandler::~CInputHandler() {
	CScreenManager::_screenManagerPtr->_inputHandler = nullptr;
}

void CInputHandler::setTranslator(CInputTranslator *translator) {
	_inputTranslator = translator;
}

void CInputHandler::incLockCount() {
	++_lockCount;
}

void CInputHandler::decLockCount() {
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

bool CInputHandler::handleMessage(CMessage &msg, bool respectLock) {
	if (!respectLock || _lockCount <= 0) {
		if (_gameManager->_gameState._mode == GSMODE_INTERACTIVE) {
			return processMessage(&msg);
		} else if (!msg.isMouseMsg()) {
			g_vm->_filesManager->loadDrive();
		}
	}

	return false;
}

bool CInputHandler::processMessage(CMessage *msg) {
	const CMouseMsg *mouseMsg = dynamic_cast<const CMouseMsg *>(msg);
	_abortMessage = false;
	bool handled = dispatchMessage(msg);

	if (_abortMessage) {
		_abortMessage = false;
	} else if (mouseMsg) {
		// Keep the game state mouse position up to date
		if (_mousePos != mouseMsg->_mousePos) {
			_mousePos = mouseMsg->_mousePos;
			_gameManager->_gameState.setMousePos(mouseMsg->_mousePos);
		}

		// Set flag for whether a mouse button is currently being pressed
		if (mouseMsg->isButtonDownMsg())
			_buttonDown = true;
		else if (mouseMsg->isButtonUpMsg())
			_buttonDown = false;

		// Drag events generation
		if (_dragging) {
			if (mouseMsg->isMouseMoveMsg()) {
				if (_dragItem) {
					CMouseDragMoveMsg moveMsg(_mousePos);
					moveMsg.execute(_dragItem);
				}
			} else if (mouseMsg->isButtonUpMsg()) {
				if (_dragItem) {
					// Mouse drag ended
					CGameObject *target = dragEnd(_mousePos, _dragItem);
					CMouseDragEndMsg endMsg(_mousePos, target);
					endMsg.execute(_dragItem);
				}

				_dragging = false;
				_dragItem = nullptr;
				_gameManager->_dragItem = nullptr;
			}
		} else if (_buttonDown) {
			if (!mouseMsg->isMouseMoveMsg()) {
				// Save where the drag movement started from
				_dragStartPos = _mousePos;
			} else {
				Point delta = mouseMsg->_mousePos - _dragStartPos;
				int distance = (int)sqrt(double(delta.x * delta.x + delta.y * delta.y));

				if (distance > 4) {
					// We've moved far enough with the mouse button held down
					// to trigger an official dragging operation
					CMouseDragStartMsg startMsg(_dragStartPos);
					dispatchMessage(&startMsg);

					// Set the drag item, if any, that a handler will have set on the message
					_dragItem = startMsg._dragItem;
					_gameManager->_dragItem = startMsg._dragItem;

					if (_dragItem) {
						CMouseDragMoveMsg moveMsg(_dragStartPos);
						moveMsg.execute(_dragItem);
					}

					_dragging = true;
				}
			}
		}
	}

	return handled;
}

bool CInputHandler::dispatchMessage(CMessage *msg) {
	CPetControl *pet = _gameManager->_project->getPetControl();
	if (!pet || !msg->execute(pet, nullptr, MSGFLAG_BREAK_IF_HANDLED)) {
		CViewItem *view = _gameManager->getView();
		return msg->execute(view);
	}

	return true;
}

CGameObject *CInputHandler::dragEnd(const Point &pt, CTreeItem *dragItem) {
	CViewItem *view = _gameManager->getView();
	if (!view)
		return nullptr;

	// Scan through the view items to find the element being dropped on
	CGameObject *target = nullptr;
	for (CTreeItem *treeItem = view->scan(view); treeItem; treeItem = treeItem->scan(view)) {
		CGameObject *gameObject = dynamic_cast<CGameObject *>(treeItem);
		if (gameObject && gameObject != dragItem) {
			if (gameObject->checkPoint(pt))
				target = gameObject;
		}
	}

	if (!target) {
		// Check if the cursor is on the PET. If so, pass to the PET
		// to see what specific element the drag ended on
		CProjectItem *project = view->getRoot();
		if (project) {
			CPetControl *petControl = project->getPetControl();
			if (petControl && petControl->contains(pt)) {
				target = petControl->dragEnd(pt);
				if (!target)
					target = petControl;
			}
		}
	}

	return target;
}

} // End of namespace Titanic
