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
#include "common/system.h"
#include "ultima/shared/early/ultima_early.h"
#include "ultima/shared/early/game_base.h"
#include "ultima/shared/engine/debugger.h"
#include "ultima/shared/engine/messages.h"
#include "ultima/shared/core/mouse_cursor.h"
#include "ultima/shared/gfx/font.h"
#include "ultima/shared/gfx/text_cursor.h"
#include "ultima/shared/gfx/text_input.h"
#include "ultima/shared/gfx/visual_container.h"

namespace Ultima {
namespace Shared {

GameBase::GameBase(): _currentView(nullptr), _pendingPopup(nullptr), _font(nullptr), _priorLeftDownTime(0),
		_priorMiddleDownTime(0), _priorRightDownTime(0), _inputHandler(this), _inputTranslator(&_inputHandler),
		_videoMode(0), _textCursor(nullptr) {
}

GameBase::~GameBase() {
	delete _font;
	delete _textCursor;
}

int GameBase::getSavegameSlot() {
	return 0;
}

void GameBase::mouseChanged() {

}

void GameBase::onIdle() {
	// Handle any drawing updates
	update();
}

#define HANDLE_MESSAGE(METHOD) \
	_inputTranslator.METHOD(g_vm->_events->getSpecialButtons(), mousePos); \
	mouseChanged()


void GameBase::mouseMove(const Point &mousePos) {
	if (!isMouseControlEnabled())
		return;

	HANDLE_MESSAGE(mouseMove);
}

void GameBase::leftButtonDown(const Point &mousePos) {
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

void GameBase::leftButtonUp(const Point &mousePos) {
	if (!isMouseControlEnabled())
		return;

	HANDLE_MESSAGE(leftButtonUp);
}

void GameBase::leftButtonDoubleClick(const Point &mousePos) {
	if (!isMouseControlEnabled())
		return;

	HANDLE_MESSAGE(leftButtonDoubleClick);
}

void GameBase::middleButtonDown(const Point &mousePos) {
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

void GameBase::middleButtonUp(const Point &mousePos) {
	if (!isMouseControlEnabled())
		return;

	HANDLE_MESSAGE(middleButtonUp);
}

void GameBase::middleButtonDoubleClick(const Point &mousePos) {
	if (!isMouseControlEnabled())
		return;

	HANDLE_MESSAGE(middleButtonDoubleClick);
}

void GameBase::rightButtonDown(const Point &mousePos) {
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

void GameBase::rightButtonUp(const Point &mousePos) {
	if (!isMouseControlEnabled())
		return;

	HANDLE_MESSAGE(rightButtonUp);
}

void GameBase::mouseWheel(const Point &mousePos, bool wheelUp) {
	if (!isMouseControlEnabled())
		return;

	_inputTranslator.mouseWheel(wheelUp, mousePos);
	mouseChanged();
}

void GameBase::rightButtonDoubleClick(const Point &mousePos) {
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
	Gfx::VisualItem *view = dynamic_cast<Gfx::VisualItem *>(findByName(viewName));

	if (view != _currentView) {
		_currentView = view;
		assert(_currentView);

		// Signal the view that it's now active
		CShowMsg showMsg;
		showMsg.execute(_currentView);
	}
}

void GameBase::setPopup(Gfx::Popup *popup) {
	assert(!_pendingPopup);
	_pendingPopup = popup;
}

void GameBase::update() {
	if (_currentView) {
		// Signal the next frame
		CFrameMsg frameMsg(g_vm->_events->getTicksCount());
		frameMsg.execute(_currentView);

		// Draw the view
		if (_currentView->isDirty()) {
			_currentView->draw();
			_currentView->clearDirty();
		} else if (_pendingPopup) {
			// There's a pending popup to display, so make it active
			_currentView = _pendingPopup;
			_pendingPopup = nullptr;

			CShowMsg showMsg;
			showMsg.execute(_currentView);
		}

		 // Allow the text cursor to update
		_textCursor->update();
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

void GameBase::setFont(Gfx::Font *font) {
	delete _font;
	_font = font;
}

uint GameBase::getRandomNumber(uint max) {
	return g_vm->getRandomNumber(max);
}

uint GameBase::getRandomNumber(uint min, uint max) {
	return g_vm->getRandomNumber(min, max);
}

void GameBase::sleep(uint time) {
	g_vm->_events->sleep(time);
}

uint32 GameBase::getMillis() const {
	return g_system->getMillis();
}

void GameBase::synchronize(Common::Serializer &s) {
	_priorLeftDownTime = 0;
	_priorMiddleDownTime = 0;
	_priorRightDownTime = 0;
}

} // End of namespace Shared
} // End of namespace Ultima
