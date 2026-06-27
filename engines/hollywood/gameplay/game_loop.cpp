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

#include "hollywood/gameplay/game_loop.h"

#include "common/events.h"
#include "common/system.h"

#include "hollywood/hollywood.h"

namespace Hollywood {

const byte kGameplayDefaultStrip = 1;
const byte kGameplayFirstStrip = 1;
const byte kGameplayLastStrip = 8;
const uint32 kGameplayLoopTickMillis = 10;
const uint32 kGameplayMaxFrameDeltaMillis = 250;

GameplayLoopDelegate::~GameplayLoopDelegate() {
}

uint16 GameplayLoopDelegate::viewportYOffset() const {
	return 0;
}

void GameplayLoopDelegate::prepareGameplayLoop() {
}

bool GameplayLoopDelegate::shouldExitGameplayLoop() const {
	return false;
}

void GameplayLoopDelegate::handleLeftClick(const GameplayLoopCursorState &state) {
	(void)state;
}

void GameplayLoopDelegate::handleRightClick(const GameplayLoopCursorState &state) {
	(void)state;
}

GameplayLoop::GameplayLoop(HollywoodEngine *vm, GameplayLoopDelegate *delegate) :
		_vm(vm),
		_delegate(delegate),
		_currentStrip(kGameplayDefaultStrip),
		_leftButtonDown(false),
		_rightButtonDown(false),
		_keyboardStripMode(false) {
}

bool GameplayLoop::run() {
	if (!_vm || !_delegate)
		return false;

	_delegate->prepareGameplayLoop();
	_currentStrip = kGameplayDefaultStrip;
	_leftButtonDown = false;
	_rightButtonDown = false;
	_keyboardStripMode = false;
	_hoverCaption.reset();
	_hoverCaption.setCurrentStrip(_currentStrip);

	_vm->cursor()->enterInteractiveMode();
	_vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());
	refreshHoverCaption();

	_delegate->drawGameplayFrame();
	_delegate->presentGameplayFrame(_hoverCaption);

	uint32 lastMillis = g_system->getMillis();
	while (!Engine::shouldQuit() && !_delegate->shouldExitGameplayLoop()) {
		if (pollEvents())
			break;

		g_system->delayMillis(kGameplayLoopTickMillis);

		const uint32 now = g_system->getMillis();
		uint32 delta = now - lastMillis;
		lastMillis = now;
		if (delta > kGameplayMaxFrameDeltaMillis)
			delta = kGameplayMaxFrameDeltaMillis;

		_delegate->advanceGameplayLoop(delta);
		_vm->cursor()->advance(delta);
		_hoverCaption.setCurrentStrip(_currentStrip);
		_hoverCaption.advance(delta, _delegate->hotspots(), _delegate->savedFramebuffer(),
			_vm->cursor()->surfaceX(), _vm->cursor()->surfaceY(),
			_delegate->viewportXOffset(), _delegate->viewportYOffset());

		_delegate->drawGameplayFrame();
		_delegate->presentGameplayFrame(_hoverCaption);
	}

	_vm->cursor()->leaveInteractiveMode();
	return true;
}

void GameplayLoop::setCurrentStrip(byte stripIndex) {
	_currentStrip = stripIndex;
	_hoverCaption.setCurrentStrip(_currentStrip);
	refreshHoverCaption();
}

bool GameplayLoop::pollEvents() {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			Engine::quitGame();
			return true;
		case Common::EVENT_MOUSEMOVE:
			_vm->cursor()->updatePosition(event.mouse);
			break;
		case Common::EVENT_KEYDOWN:
			handleKeyDown(event.kbd);
			break;
		case Common::EVENT_KEYUP:
			handleKeyUp(event.kbd);
			break;
		case Common::EVENT_LBUTTONDOWN:
			if (!_leftButtonDown) {
				_leftButtonDown = true;
				handleLeftClick();
			}
			break;
		case Common::EVENT_LBUTTONUP:
			_leftButtonDown = false;
			break;
		case Common::EVENT_RBUTTONDOWN:
			if (!_rightButtonDown) {
				_rightButtonDown = true;
				handleRightClick();
			}
			break;
		case Common::EVENT_RBUTTONUP:
			_rightButtonDown = false;
			break;
		default:
			break;
		}
	}

	return false;
}

void GameplayLoop::handleKeyDown(const Common::KeyState &keyState) {
	switch (keyState.keycode) {
	case Common::KEYCODE_LSHIFT:
	case Common::KEYCODE_RSHIFT:
		enterKeyboardStripMode();
		break;
	case Common::KEYCODE_LEFT:
		if (_keyboardStripMode || (keyState.flags & Common::KBD_SHIFT)) {
			enterKeyboardStripMode();
			selectPreviousKeyboardStrip();
		}
		break;
	case Common::KEYCODE_RIGHT:
		if (_keyboardStripMode || (keyState.flags & Common::KBD_SHIFT)) {
			enterKeyboardStripMode();
			selectNextKeyboardStrip();
		}
		break;
	default:
		break;
	}
}

void GameplayLoop::handleKeyUp(const Common::KeyState &keyState) {
	switch (keyState.keycode) {
	case Common::KEYCODE_LSHIFT:
	case Common::KEYCODE_RSHIFT:
		leaveKeyboardStripMode();
		break;
	default:
		break;
	}
}

void GameplayLoop::handleLeftClick() {
	_delegate->handleLeftClick(makeCursorState());
	refreshHoverCaption();
}

void GameplayLoop::handleRightClick() {
	if (_hoverCaption.requestedStrip() != 0 && _hoverCaption.requestedStrip() != _currentStrip) {
		_currentStrip = _hoverCaption.requestedStrip();
		_hoverCaption.setCurrentStrip(_currentStrip);
	} else {
		selectNextStrip();
	}
	_delegate->handleRightClick(makeCursorState());
	refreshHoverCaption();
}

void GameplayLoop::enterKeyboardStripMode() {
	if (_keyboardStripMode)
		return;

	_keyboardStripMode = true;
	byte requestedStrip = _hoverCaption.requestedStrip();
	if (requestedStrip < 2 && _currentStrip == kGameplayDefaultStrip)
		requestedStrip = 5;
	if (requestedStrip != 0)
		_currentStrip = requestedStrip;

	_hoverCaption.setCurrentStrip(_currentStrip);
	refreshHoverCaption();
}

void GameplayLoop::leaveKeyboardStripMode() {
	_keyboardStripMode = false;
	refreshHoverCaption();
}

void GameplayLoop::selectPreviousKeyboardStrip() {
	if (_currentStrip > 2)
		--_currentStrip;

	_hoverCaption.setCurrentStrip(_currentStrip);
	refreshHoverCaption();
}

void GameplayLoop::selectNextKeyboardStrip() {
	if (_currentStrip < kGameplayLastStrip)
		++_currentStrip;

	_hoverCaption.setCurrentStrip(_currentStrip);
	refreshHoverCaption();
}

void GameplayLoop::selectNextStrip() {
	if (_currentStrip < kGameplayFirstStrip || _currentStrip >= kGameplayLastStrip)
		_currentStrip = kGameplayFirstStrip;
	else
		++_currentStrip;

	_hoverCaption.setCurrentStrip(_currentStrip);
}

GameplayLoopCursorState GameplayLoop::makeCursorState() const {
	GameplayLoopCursorState state;
	state.cursorX = _vm->cursor()->surfaceX();
	state.cursorY = _vm->cursor()->surfaceY();
	state.sceneX = state.cursorX + _delegate->viewportXOffset();
	state.sceneY = state.cursorY + _delegate->viewportYOffset();
	state.currentStrip = _currentStrip;
	state.requestedStrip = _hoverCaption.requestedStrip();
	state.resolvedItem = _hoverCaption.resolvedItem();
	return state;
}

void GameplayLoop::refreshHoverCaption() {
	_hoverCaption.refreshNow(_delegate->hotspots(), _delegate->savedFramebuffer(),
		_vm->cursor()->surfaceX(), _vm->cursor()->surfaceY(),
		_delegate->viewportXOffset(), _delegate->viewportYOffset());
}

} // End of namespace Hollywood
