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
const byte kGameplayFirstPanelStrip = 2;
const byte kGameplayLastStrip = 8;
const uint32 kGameplayLoopTickMillis = 10;
const uint32 kGameplayMaxFrameDeltaMillis = 250;
const uint32 kGameplayPanelHoverMillis = 75;
const uint16 kGameplayVerbPanelTopY = 0x19f;
const uint16 kGameplayInventoryOpenY = 0x1df;
const uint16 kGameplayInventoryCloseY = 0x122;
const uint16 kGameplayVerbPanelStripWidth = 0x58;
const uint16 kGameplayVerbPanelStripHeight = 0x1b;
const uint16 kGameplayVerbPanelStripTopY = 0x1bd;
const uint16 kGameplayVerbPanelStripXOffsets[9] = {
	0xff, 0, 8, 97, 186, 276, 366, 456, 545
};

GameplayPanelState::GameplayPanelState() :
		verbPanelVisible(false),
		inventoryPanelVisible(false),
		currentStrip(kGameplayDefaultStrip),
		requestedStrip(0),
		resolvedItem(0) {
}

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
		_keyboardStripMode(false),
		_inventoryPanelOpenedFromDefault(false),
		_panelHoverTimer(0) {
}

bool GameplayLoop::run() {
	if (!_vm || !_delegate)
		return false;

	_delegate->prepareGameplayLoop();
	_currentStrip = kGameplayDefaultStrip;
	_leftButtonDown = false;
	_rightButtonDown = false;
	_keyboardStripMode = false;
	_inventoryPanelOpenedFromDefault = false;
	_panelState = GameplayPanelState();
	_panelHoverTimer = 0;
	_hoverCaption.reset();
	_hoverCaption.setCurrentStrip(_currentStrip);

	_vm->cursor()->enterInteractiveMode();
	_vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());
	refreshHoverCaption();
	syncPanelState();

	_delegate->drawGameplayFrame();
	_delegate->presentGameplayFrame(_hoverCaption, _panelState);

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
		if (_panelState.visible())
			updatePanelHover(delta);
		else
			_hoverCaption.advance(delta, _delegate->hotspots(), _delegate->savedFramebuffer(),
				_vm->cursor()->surfaceX(), _vm->cursor()->surfaceY(),
				_delegate->viewportXOffset(), _delegate->viewportYOffset());
		syncPanelState();

		_delegate->drawGameplayFrame();
		_delegate->presentGameplayFrame(_hoverCaption, _panelState);
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
			updatePanelFromMousePosition();
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
				openVerbPanel();
			}
			break;
		case Common::EVENT_RBUTTONUP:
			_rightButtonDown = false;
			if (!_keyboardStripMode)
				closeVerbPanel();
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
	if (_panelState.visible())
		return;

	_delegate->handleLeftClick(makeCursorState());
	_currentStrip = kGameplayDefaultStrip;
	_hoverCaption.setCurrentStrip(_currentStrip);
	refreshHoverCaption();
}

void GameplayLoop::handleRightClick() {
	const byte requestedStrip = _hoverCaption.requestedStrip();
	if (requestedStrip >= kGameplayFirstPanelStrip && requestedStrip <= kGameplayLastStrip &&
			requestedStrip != _currentStrip) {
		_currentStrip = requestedStrip;
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
	openVerbPanel();
}

void GameplayLoop::leaveKeyboardStripMode() {
	_keyboardStripMode = false;
	if (!_rightButtonDown)
		closeVerbPanel();
}

void GameplayLoop::selectPreviousKeyboardStrip() {
	selectPanelStrip(_currentStrip > kGameplayFirstPanelStrip ? (byte)(_currentStrip - 1) : _currentStrip);
}

void GameplayLoop::selectNextKeyboardStrip() {
	if (_currentStrip < kGameplayLastStrip)
		selectPanelStrip((byte)(_currentStrip + 1));
}

void GameplayLoop::selectNextStrip() {
	if (_currentStrip < kGameplayFirstPanelStrip || _currentStrip >= kGameplayLastStrip)
		_currentStrip = kGameplayFirstPanelStrip;
	else
		++_currentStrip;

	_hoverCaption.setCurrentStrip(_currentStrip);
}

void GameplayLoop::openVerbPanel() {
	closeInventoryPanel();
	refreshHoverCaption();

	byte requestedStrip = _hoverCaption.requestedStrip();
	if (requestedStrip < kGameplayFirstPanelStrip) {
		if (_currentStrip == kGameplayDefaultStrip)
			requestedStrip = 5;
		else
			requestedStrip = 0;
	}
	if (requestedStrip >= kGameplayFirstPanelStrip)
		_currentStrip = requestedStrip;
	if (_currentStrip > kGameplayLastStrip)
		_currentStrip = kGameplayLastStrip;

	_panelState.verbPanelVisible = true;
	_panelState.inventoryPanelVisible = false;
	_panelState.requestedStrip = requestedStrip;
	_panelState.resolvedItem = _hoverCaption.resolvedItem();
	_panelState.itemName = _delegate->hotspots().itemName(_panelState.resolvedItem);
	_panelHoverTimer = 0;
	_hoverCaption.setCurrentStrip(_currentStrip);
	updatePanelCaption();
}

void GameplayLoop::closeVerbPanel() {
	if (!_panelState.verbPanelVisible)
		return;

	_panelState.verbPanelVisible = false;
	_panelState.captionText.clear();
	_panelState.itemName.clear();
	_panelState.resolvedItem = 0;
	refreshHoverCaption();
}

void GameplayLoop::openInventoryPanel() {
	if (_panelState.inventoryPanelVisible || _panelState.verbPanelVisible)
		return;

	_inventoryPanelOpenedFromDefault = _currentStrip == kGameplayDefaultStrip;
	if (_inventoryPanelOpenedFromDefault) {
		_currentStrip = 5;
		_hoverCaption.setCurrentStrip(_currentStrip);
	}
	_panelState.inventoryPanelVisible = true;
	_panelState.captionText = "Inventario";
	_panelState.resolvedItem = 0;
	_panelState.itemName.clear();
}

void GameplayLoop::closeInventoryPanel() {
	if (!_panelState.inventoryPanelVisible)
		return;

	_panelState.inventoryPanelVisible = false;
	_panelState.captionText.clear();
	if (_inventoryPanelOpenedFromDefault && _currentStrip == 5) {
		_currentStrip = kGameplayDefaultStrip;
		_hoverCaption.setCurrentStrip(_currentStrip);
	}
	_inventoryPanelOpenedFromDefault = false;
	refreshHoverCaption();
}

void GameplayLoop::updatePanelHover(uint32 deltaMillis) {
	if (!_panelState.verbPanelVisible || _keyboardStripMode)
		return;

	_panelHoverTimer += deltaMillis;
	if (_panelHoverTimer < kGameplayPanelHoverMillis)
		return;

	while (_panelHoverTimer >= kGameplayPanelHoverMillis)
		_panelHoverTimer -= kGameplayPanelHoverMillis;
	updatePanelFromMousePosition();
}

void GameplayLoop::updatePanelFromMousePosition() {
	const uint16 cursorX = _vm->cursor()->surfaceX();
	const uint16 cursorY = _vm->cursor()->surfaceY();

	if (_panelState.verbPanelVisible) {
		const byte stripIndex = panelStripAt(cursorX, cursorY);
		if (stripIndex != 0)
			selectPanelStrip(stripIndex);
		return;
	}

	if (_panelState.inventoryPanelVisible) {
		if (cursorY <= kGameplayInventoryCloseY)
			closeInventoryPanel();
		return;
	}

	if (cursorY >= kGameplayInventoryOpenY)
		openInventoryPanel();
}

void GameplayLoop::selectPanelStrip(byte stripIndex) {
	if (stripIndex < kGameplayFirstPanelStrip || stripIndex > kGameplayLastStrip)
		return;
	if (_currentStrip == stripIndex)
		return;

	_currentStrip = stripIndex;
	_hoverCaption.setCurrentStrip(_currentStrip);
	updatePanelCaption();
}

byte GameplayLoop::panelStripAt(uint16 cursorX, uint16 cursorY) const {
	if (cursorY < kGameplayVerbPanelTopY ||
			cursorY < kGameplayVerbPanelStripTopY ||
			cursorY >= kGameplayVerbPanelStripTopY + kGameplayVerbPanelStripHeight)
		return 0;

	for (byte stripIndex = kGameplayFirstPanelStrip; stripIndex <= kGameplayLastStrip; ++stripIndex) {
		const uint16 stripX = kGameplayVerbPanelStripXOffsets[stripIndex];
		if (cursorX >= stripX && cursorX < stripX + kGameplayVerbPanelStripWidth)
			return stripIndex;
	}

	return 0;
}

void GameplayLoop::updatePanelCaption() {
	_panelState.currentStrip = _currentStrip;
	_panelState.captionText = inventoryActionCaption(_currentStrip);
	if (_panelState.resolvedItem != 0 &&
			_delegate->hotspots().hasVerbAction(_panelState.resolvedItem, _currentStrip))
		_panelState.captionText += _panelState.itemName;
}

void GameplayLoop::syncPanelState() {
	_panelState.currentStrip = _currentStrip;
	_panelState.requestedStrip = _hoverCaption.requestedStrip();
	if (!_panelState.verbPanelVisible && !_panelState.inventoryPanelVisible)
		_panelState.resolvedItem = _hoverCaption.resolvedItem();
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
