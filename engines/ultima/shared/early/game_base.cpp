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

#include "common/config-manager.h"
#include "ultima/shared/early/ultima_early.h"
#include "ultima/shared/early/game_base.h"
#include "ultima/shared/engine/debugger.h"
#include "ultima/shared/engine/messages.h"
#include "ultima/shared/core/mouse_cursor.h"
#include "ultima/shared/gfx/visual_container.h"

namespace Ultima {
namespace Shared {

GameBase::GameBase() : _currentView(nullptr),
_priorLeftDownTime(0), _priorMiddleDownTime(0), _priorRightDownTime(0),
_inputHandler(this), _inputTranslator(&_inputHandler) {
}

void GameBase::starting() {
	// Enable the mouse cursor
	g_vm->_mouseCursor->show();

	// Generate starting message for showing the view
	assert(_currentView);
	CShowMsg showMsg;
	showMsg.execute(_currentView, nullptr, MSGFLAG_SCAN);
}

int GameBase::getSavegameSlot() {
	return 0;
}

void GameBase::draw() {
	if (_currentView)
		_currentView->draw();
}

void GameBase::mouseChanged() {

}

void GameBase::onIdle() {
	// Handle any drawing updates
	draw();
}

#define HANDLE_MESSAGE(METHOD) \
	_inputTranslator.METHOD(g_vm->_events->getSpecialButtons(), mousePos); \
	mouseChanged()


void GameBase::mouseMove(const Common::Point &mousePos) {
	if (!isMouseControlEnabled())
		return;

	HANDLE_MESSAGE(mouseMove);
}

void GameBase::leftButtonDown(const Common::Point &mousePos) {
	if (!isMouseControlEnabled())
		return;

	if ((g_vm->_events->getTicksCount() - _priorLeftDownTime) < DOUBLE_CLICK_TIME) {
		_priorLeftDownTime = 0;
		leftButtonDoubleClick(mousePos);
	} else {
		_priorLeftDownTime = g_vm->_events->getTicksCount();
		HANDLE_MESSAGE(leftButtonDown);
	}
}

void GameBase::leftButtonUp(const Common::Point &mousePos) {
	if (!isMouseControlEnabled())
		return;

	HANDLE_MESSAGE(leftButtonUp);
}

void GameBase::leftButtonDoubleClick(const Common::Point &mousePos) {
	if (!isMouseControlEnabled())
		return;

	HANDLE_MESSAGE(leftButtonDoubleClick);
}

void GameBase::middleButtonDown(const Common::Point &mousePos) {
	if (!isMouseControlEnabled())
		return;

	if ((g_vm->_events->getTicksCount() - _priorMiddleDownTime) < DOUBLE_CLICK_TIME) {
		_priorMiddleDownTime = 0;
		middleButtonDoubleClick(mousePos);
	} else {
		_priorMiddleDownTime = g_vm->_events->getTicksCount();
		HANDLE_MESSAGE(middleButtonDown);
	}
}

void GameBase::middleButtonUp(const Common::Point &mousePos) {
	if (!isMouseControlEnabled())
		return;

	HANDLE_MESSAGE(middleButtonUp);
}

void GameBase::middleButtonDoubleClick(const Common::Point &mousePos) {
	if (!isMouseControlEnabled())
		return;

	HANDLE_MESSAGE(middleButtonDoubleClick);
}

void GameBase::rightButtonDown(const Common::Point &mousePos) {
	if (!isMouseControlEnabled())
		return;

	if ((g_vm->_events->getTicksCount() - _priorRightDownTime) < DOUBLE_CLICK_TIME) {
		_priorRightDownTime = 0;
		rightButtonDoubleClick(mousePos);
	} else {
		_priorRightDownTime = g_vm->_events->getTicksCount();
		HANDLE_MESSAGE(rightButtonDown);
	}
}

void GameBase::rightButtonUp(const Common::Point &mousePos) {
	if (!isMouseControlEnabled())
		return;

	HANDLE_MESSAGE(rightButtonUp);
}

void GameBase::mouseWheel(const Common::Point &mousePos, bool wheelUp) {
	if (!isMouseControlEnabled())
		return;

	_inputTranslator.mouseWheel(wheelUp, mousePos);
	mouseChanged();
}

void GameBase::rightButtonDoubleClick(const Common::Point &mousePos) {
	if (!isMouseControlEnabled())
		return;

	HANDLE_MESSAGE(rightButtonDoubleClick);
}

void GameBase::keyDown(Common::KeyState keyState) {
	_inputTranslator.keyDown(keyState);
}

void GameBase::setView(Gfx::VisualItem *view) {
	_currentView = view;
}

void GameBase::setView(const Common::String &viewName) {
	_currentView = dynamic_cast<Gfx::VisualItem *>(findByName(viewName));
	assert(_currentView);
}

void GameBase::update() {
	if (_currentView) {
		// Signal the next frame
		CFrameMsg frameMsg(g_vm->_events->getTicksCount());
		frameMsg.execute(_currentView, nullptr, MSGFLAG_SCAN);

		_currentView->draw();
	}
}

void GameBase::changeView(const Common::String &name) {
	Gfx::VisualItem *newView = dynamic_cast<Gfx::VisualItem *>(findByName(name));
	assert(newView);

	// Hide the current view
	CHideMsg hideMsg(_currentView, true);
	hideMsg.execute(_currentView, nullptr, MSGFLAG_SCAN);

	if (hideMsg._fadeOut) {
		// TODO: Fade out
	}

	// Show the new view
	_currentView = newView;
	CShowMsg showMsg(_currentView, true);
	showMsg.execute(_currentView, nullptr, MSGFLAG_SCAN);

	_currentView->draw();

	if (showMsg._fadeIn) {
		// TODO: Fade in
	}
}

} // End of namespace Shared
} // End of namespace Ultima
