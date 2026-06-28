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

#include "common/debug.h"
#include "common/events.h"
#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/gameplay/options_menu.h"
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
const uint16 kGameplayInventoryPanelStripTopY = 0x12b;
const uint16 kGameplayInventoryGridLeft = 0x32;
const uint16 kGameplayInventoryGridTop = 0x152;
const uint16 kGameplayInventoryTileSize = 0x40;
const uint16 kGameplayInventoryTileStride = 0x44;
const uint16 kGameplayInventoryNextVisualOverlapX = 0x36;
const uint16 kGameplayInventoryPageArrowLeft = 0x25a;
const uint16 kGameplayInventoryPageArrowRight = 0x277;
const uint16 kGameplayInventoryOptionsButtonLeft = 7;
const uint16 kGameplayInventoryOptionsButtonRight = 0x26;
const uint16 kGameplayInventoryOptionsButtonTop = 0x178;
const uint16 kGameplayInventoryOptionsButtonBottom = 0x1b0;
const uint16 kGameplayInventoryPreviousPageArrowTop = 0x178;
const uint16 kGameplayInventoryPreviousPageArrowBottom = 0x192;
const uint16 kGameplayInventoryNextPageArrowTop = 0x195;
const uint16 kGameplayInventoryNextPageArrowBottom = 0x1af;
const byte kGameplayUseStrip = 5;
const byte kGameplayGiveStrip = 8;
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

void GameplayLoopDelegate::prepareOptionsMenuPalette(Common::Array<byte> &palette) const {
	palette.clear();
}

bool GameplayLoopDelegate::shouldExitGameplayLoop() const {
	return false;
}

Common::String GameplayLoopDelegate::inventoryItemName(byte owner, byte itemId) const {
	(void)owner;
	(void)itemId;
	return Common::String();
}

void GameplayLoopDelegate::beginSharedInventorySpeechLine(uint16 rowIndex, byte frameIndex) {
	(void)rowIndex;
	(void)frameIndex;
}

byte GameplayLoopDelegate::randomSharedInventorySpeechFrame(byte maxFrameIndex) {
	(void)maxFrameIndex;
	return 0;
}

void GameplayLoopDelegate::playSharedInventorySound(byte sampleId) {
	(void)sampleId;
}

void GameplayLoopDelegate::handleLeftClick(const GameplayLoopCursorState &state) {
	(void)state;
}

void GameplayLoopDelegate::handleInventoryItemClick(const GameplayLoopCursorState &state) {
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
		_relationMode(0),
		_primaryInventoryItem(0),
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
	_relationMode = 0;
	_primaryInventoryItem = 0;
	_panelState = GameplayPanelState();
	_panelHoverTimer = 0;
	_hoverCaption.reset();
	_hoverCaption.setCurrentStrip(_currentStrip);

	_vm->cursor()->enterInteractiveMode();
	_vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());
	syncHoverCaptionRelationContext();
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
		syncHoverCaptionRelationContext();
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
	case Common::KEYCODE_ESCAPE:
		openOptionsMenu();
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
	if (_panelState.inventoryPanelVisible) {
		const uint16 cursorX = _vm->cursor()->surfaceX();
		const uint16 cursorY = _vm->cursor()->surfaceY();
		if (isInventoryPanelOptionsButton(cursorX, cursorY)) {
			openOptionsMenu();
			return;
		}

		const byte stripIndex = inventoryPanelStripAt(cursorX, cursorY);
		if (stripIndex != 0) {
			selectPanelStrip(stripIndex);
			updateInventoryPanelCaption();
			syncPanelState();
			return;
		}
		if (isInventoryPanelPreviousPageArrow(cursorX, cursorY)) {
			if (scrollInventoryPanelPreviousPage())
				updateInventoryPanelCaption();
			syncPanelState();
			return;
		}
		if (isInventoryPanelNextPageArrow(cursorX, cursorY)) {
			if (scrollInventoryPanelNextPage())
				updateInventoryPanelCaption();
			syncPanelState();
			return;
		}

		const byte itemId = inventoryItemAtPanelPosition(cursorX, cursorY);
		const uint inventorySlot = inventoryPanelSlotAtPosition(cursorX, cursorY);
		if (itemId == 0) {
			debugC(2, kDebugScene,
				"Inventory click ignored: no item at cursor (%u,%u), slot=%u, strip=%u, relationMode=%u, primary=0x%02x",
				cursorX, cursorY, inventorySlot, _currentStrip, _relationMode, _primaryInventoryItem);
			return;
		}

		const byte owner = currentInventoryOwner();
		const Common::String clickedItemName = _delegate->inventoryItemName(owner, itemId);
		if (_relationMode != 0 && _primaryInventoryItem != 0) {
			const uint16 actionHandlerId =
				dialogueInventoryRelationHandler(_primaryInventoryItem, itemId, _relationMode);
			if (actionHandlerId != 0) {
				debugC(2, kDebugScene,
					"Inventory relation accepted: mode=%u primary=0x%02x secondary=0x%02x '%s' slot=%u handler=%u",
					_relationMode, _primaryInventoryItem, itemId, clickedItemName.c_str(),
					inventorySlot, actionHandlerId);
				GameplayLoopCursorState state = makeInventoryItemState(owner, itemId, actionHandlerId);
				closeInventoryPanel();
				if (!dispatchGlobalInventoryItemAction(state))
					_delegate->handleInventoryItemClick(state);
				_relationMode = 0;
				_primaryInventoryItem = 0;
				_currentStrip = kGameplayDefaultStrip;
				_hoverCaption.setCurrentStrip(_currentStrip);
				refreshHoverCaption();
				syncPanelState();
			} else {
				debugC(1, kDebugScene,
					"Inventory relation rejected: no handler for mode=%u primary=0x%02x secondary=0x%02x '%s' slot=%u",
					_relationMode, _primaryInventoryItem, itemId, clickedItemName.c_str(),
					inventorySlot);
				updateInventoryPanelCaption();
				syncPanelState();
			}
			return;
		}

		const uint16 actionHandlerId = fixedInventoryActionHandler(owner, itemId, _currentStrip);
		debugC(2, kDebugScene,
			"Inventory item click: owner=%u item=0x%02x '%s' slot=%u cursor=(%u,%u) strip=%u handler=%u",
			owner, itemId, clickedItemName.c_str(), inventorySlot, cursorX, cursorY,
			_currentStrip, actionHandlerId);
		if ((_currentStrip == kGameplayUseStrip || _currentStrip == kGameplayGiveStrip) &&
				actionHandlerId == 1) {
			_relationMode = _currentStrip == kGameplayUseStrip ? 1 : 2;
			_primaryInventoryItem = itemId;
			updateInventoryPanelCaption();
			debugC(2, kDebugScene, "Inventory relation started: mode=%u primary=0x%02x caption='%s'",
				_relationMode, _primaryInventoryItem, _panelState.captionText.c_str());
			refreshHoverCaption();
			syncPanelState();
			return;
		}

		if (actionHandlerId != 0) {
			GameplayLoopCursorState state = makeInventoryItemState(owner, itemId, actionHandlerId);
			closeInventoryPanel();
			if (!dispatchGlobalInventoryFixedAction(state))
				_delegate->handleInventoryItemClick(state);
			_relationMode = 0;
			_primaryInventoryItem = 0;
			_currentStrip = kGameplayDefaultStrip;
			_hoverCaption.setCurrentStrip(_currentStrip);
			refreshHoverCaption();
			syncPanelState();
		} else {
			debugC(1, kDebugScene,
				"Inventory item click rejected: no fixed handler for owner=%u item=0x%02x '%s' slot=%u strip=%u",
				owner, itemId, clickedItemName.c_str(), inventorySlot, _currentStrip);
			updateInventoryPanelCaption();
			syncPanelState();
		}
		return;
	}

	if (_panelState.verbPanelVisible)
		return;

	_delegate->handleLeftClick(makeCursorState());
	_relationMode = 0;
	_primaryInventoryItem = 0;
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
	updateInventoryPanelCaption();
}

void GameplayLoop::closeInventoryPanel() {
	if (!_panelState.inventoryPanelVisible)
		return;

	_panelState.inventoryPanelVisible = false;
	_panelState.captionText.clear();
	if (_inventoryPanelOpenedFromDefault && _currentStrip == 5 && _relationMode == 0) {
		_currentStrip = kGameplayDefaultStrip;
		_hoverCaption.setCurrentStrip(_currentStrip);
	}
	_inventoryPanelOpenedFromDefault = false;
	refreshHoverCaption();
}

void GameplayLoop::openOptionsMenu() {
	const bool restoreInventoryPanel = _panelState.inventoryPanelVisible;
	const bool restoreVerbPanel = _panelState.verbPanelVisible && !restoreInventoryPanel;

	_vm->cursor()->leaveInteractiveMode();

	Common::Array<byte> palette;
	_delegate->prepareOptionsMenuPalette(palette);
	GameplayOptionsMenu menu(_vm);
	menu.run(palette);
	if (Engine::shouldQuit())
		return;

	_leftButtonDown = false;
	_rightButtonDown = false;
	_keyboardStripMode = false;
	_vm->cursor()->enterInteractiveMode();
	_vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());

	if (restoreInventoryPanel) {
		_panelState.inventoryPanelVisible = true;
		_panelState.verbPanelVisible = false;
		updateInventoryPanelCaption();
	} else if (restoreVerbPanel) {
		_panelState.verbPanelVisible = true;
		_panelState.inventoryPanelVisible = false;
		updatePanelCaption();
	} else {
		_panelState.verbPanelVisible = false;
		_panelState.inventoryPanelVisible = false;
		_panelState.captionText.clear();
		_panelState.itemName.clear();
	}

	refreshHoverCaption();
	syncPanelState();
	_delegate->drawGameplayFrame();
	_delegate->presentGameplayFrame(_hoverCaption, _panelState);
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
		if (cursorY <= kGameplayInventoryCloseY) {
			closeInventoryPanel();
		} else {
			updateInventoryPanelCaption();
		}
		return;
	}

	if (cursorY >= kGameplayInventoryOpenY)
		openInventoryPanel();
}

void GameplayLoop::selectPanelStrip(byte stripIndex) {
	if (stripIndex < kGameplayFirstPanelStrip || stripIndex > kGameplayLastStrip)
		return;

	const bool relationModeWasActive = _relationMode != 0 || _primaryInventoryItem != 0;
	_relationMode = 0;
	_primaryInventoryItem = 0;
	if (relationModeWasActive)
		syncHoverCaptionRelationContext();

	if (_currentStrip == stripIndex) {
		if (relationModeWasActive) {
			if (_panelState.inventoryPanelVisible)
				updateInventoryPanelCaption();
			else
				updatePanelCaption();
		}
		return;
	}

	_currentStrip = stripIndex;
	_hoverCaption.setCurrentStrip(_currentStrip);
	if (_panelState.inventoryPanelVisible)
		updateInventoryPanelCaption();
	else
		updatePanelCaption();
}

byte GameplayLoop::panelStripAt(uint16 cursorX, uint16 cursorY) const {
	if (cursorY < kGameplayVerbPanelTopY ||
			cursorY < kGameplayVerbPanelStripTopY ||
			cursorY >= kGameplayVerbPanelStripTopY + kGameplayVerbPanelStripHeight)
		return 0;

	return inventoryPanelStripAt(cursorX, cursorY - kGameplayVerbPanelStripTopY +
		kGameplayInventoryPanelStripTopY);
}

byte GameplayLoop::inventoryPanelStripAt(uint16 cursorX, uint16 cursorY) const {
	if (cursorY < kGameplayInventoryPanelStripTopY ||
			cursorY >= kGameplayInventoryPanelStripTopY + kGameplayVerbPanelStripHeight)
		return 0;

	for (byte stripIndex = kGameplayFirstPanelStrip; stripIndex <= kGameplayLastStrip; ++stripIndex) {
		const uint16 stripX = kGameplayVerbPanelStripXOffsets[stripIndex];
		if (cursorX >= stripX && cursorX < stripX + kGameplayVerbPanelStripWidth)
			return stripIndex;
	}

	return 0;
}

uint GameplayLoop::inventoryPanelSlotAtPosition(uint16 cursorX, uint16 cursorY) const {
	if (cursorX < kGameplayInventoryGridLeft || cursorY < kGameplayInventoryGridTop)
		return 0;

	const uint16 gridX = cursorX - kGameplayInventoryGridLeft;
	const uint16 gridY = cursorY - kGameplayInventoryGridTop;
	const byte column = gridX / kGameplayInventoryTileStride;
	const byte row = gridY / kGameplayInventoryTileStride;
	if (column >= 8 || row >= 2 ||
			gridX % kGameplayInventoryTileStride >= kGameplayInventoryTileSize ||
			gridY % kGameplayInventoryTileStride >= kGameplayInventoryTileSize)
		return 0;

	const GameplayState &gameState = _vm->gameState();
	const byte owner = gameState.currentInventoryOwnerIndex;
	if (owner >= GameplayState::kInventoryOwnerCount)
		return 0;

	byte firstVisibleSlot = gameState.inventoryFirstVisibleSlotByOwner[owner];
	if (firstVisibleSlot == 0)
		firstVisibleSlot = GameplayState::kInventoryFirstSlot;

	const uint slot = firstVisibleSlot + row * 8 + column;
	if (column < 7 && gridX % kGameplayInventoryTileStride >= kGameplayInventoryNextVisualOverlapX) {
		const uint nextSlot = slot + 1;
		if (nextSlot < GameplayState::kInventoryOwnerSlotStride &&
				nextSlot <= gameState.inventoryItemCountByOwner[owner] &&
				gameState.inventorySlotItemIdByOwner[owner][nextSlot] != 0)
			return nextSlot;
	}

	return slot;
}

byte GameplayLoop::inventoryItemAtPanelPosition(uint16 cursorX, uint16 cursorY) const {
	const uint slot = inventoryPanelSlotAtPosition(cursorX, cursorY);
	if (slot == 0)
		return 0;

	const GameplayState &gameState = _vm->gameState();
	const byte owner = gameState.currentInventoryOwnerIndex;
	if (owner >= GameplayState::kInventoryOwnerCount)
		return 0;

	if (slot >= GameplayState::kInventoryOwnerSlotStride ||
			slot > gameState.inventoryItemCountByOwner[owner])
		return 0;

	return gameState.inventorySlotItemIdByOwner[owner][slot];
}

byte GameplayLoop::currentInventoryOwner() const {
	const GameplayState &gameState = _vm->gameState();
	if (gameState.currentInventoryOwnerIndex >= GameplayState::kInventoryOwnerCount)
		return 0;

	return gameState.currentInventoryOwnerIndex;
}

uint16 GameplayLoop::fixedInventoryActionHandler(byte owner, byte itemId, byte stripIndex) const {
	return _vm->gameState().fixedInventoryVerbHandler(owner, itemId, stripIndex);
}

uint16 GameplayLoop::dialogueInventoryRelationHandler(byte primaryItemId, byte secondaryItemId, byte relationMode) const {
	return _vm->gameState().dialogueInventoryRelationHandler(primaryItemId, secondaryItemId, relationMode);
}

bool GameplayLoop::scrollInventoryPanelPreviousPage() {
	GameplayState &gameState = _vm->gameState();
	const byte owner = currentInventoryOwner();
	byte &firstVisibleSlot = gameState.inventoryFirstVisibleSlotByOwner[owner];
	if (firstVisibleSlot <= GameplayState::kInventoryFirstSlot)
		return false;

	firstVisibleSlot = firstVisibleSlot > 8 ? (byte)(firstVisibleSlot - 8) : GameplayState::kInventoryFirstSlot;
	gameState.inventoryPanelRedrawn = true;
	return true;
}

bool GameplayLoop::scrollInventoryPanelNextPage() {
	GameplayState &gameState = _vm->gameState();
	const byte owner = currentInventoryOwner();
	byte &firstVisibleSlot = gameState.inventoryFirstVisibleSlotByOwner[owner];
	const byte itemCount = gameState.inventoryItemCountByOwner[owner];
	if (firstVisibleSlot == 0)
		firstVisibleSlot = GameplayState::kInventoryFirstSlot;
	if (firstVisibleSlot + GameplayState::kInventoryVisibleSlotCount - 1 >= itemCount)
		return false;

	firstVisibleSlot = (byte)(firstVisibleSlot + 8);
	gameState.inventoryPanelRedrawn = true;
	return true;
}

bool GameplayLoop::isInventoryPanelOptionsButton(uint16 cursorX, uint16 cursorY) const {
	return cursorX > kGameplayInventoryOptionsButtonLeft &&
		cursorX < kGameplayInventoryOptionsButtonRight &&
		cursorY >= kGameplayInventoryOptionsButtonTop &&
		cursorY < kGameplayInventoryOptionsButtonBottom;
}

bool GameplayLoop::isInventoryPanelPreviousPageArrow(uint16 cursorX, uint16 cursorY) const {
	return cursorX >= kGameplayInventoryPageArrowLeft &&
		cursorX <= kGameplayInventoryPageArrowRight &&
		cursorY >= kGameplayInventoryPreviousPageArrowTop &&
		cursorY <= kGameplayInventoryPreviousPageArrowBottom;
}

bool GameplayLoop::isInventoryPanelNextPageArrow(uint16 cursorX, uint16 cursorY) const {
	return cursorX >= kGameplayInventoryPageArrowLeft &&
		cursorX <= kGameplayInventoryPageArrowRight &&
		cursorY >= kGameplayInventoryNextPageArrowTop &&
		cursorY <= kGameplayInventoryNextPageArrowBottom;
}

void GameplayLoop::updatePanelCaption() {
	_panelState.currentStrip = _currentStrip;
	_panelState.captionText = inventoryActionCaption(_currentStrip);
	if (_panelState.resolvedItem != 0 &&
			_delegate->hotspots().hasVerbAction(_panelState.resolvedItem, _currentStrip))
		_panelState.captionText += _panelState.itemName;
}

void GameplayLoop::updateInventoryPanelCaption() {
	const byte owner = currentInventoryOwner();
	const byte itemId = inventoryItemAtPanelPosition(_vm->cursor()->surfaceX(), _vm->cursor()->surfaceY());

	_panelState.currentStrip = _currentStrip;
	_panelState.resolvedItem = itemId;
	_panelState.itemName = itemId == 0 ? Common::String() : _delegate->inventoryItemName(owner, itemId);
	if (_relationMode != 0 && _primaryInventoryItem != 0) {
		_panelState.captionText = inventoryActionCaption(_relationMode == 2 ? kGameplayGiveStrip : kGameplayUseStrip);
		_panelState.captionText += _delegate->inventoryItemName(owner, _primaryInventoryItem);
		_panelState.captionText += _relationMode == 2 ? " a " : " con ";
		if (itemId != 0 &&
				dialogueInventoryRelationHandler(_primaryInventoryItem, itemId, _relationMode) != 0)
			_panelState.captionText += _panelState.itemName;
		return;
	}

	const uint16 actionHandlerId = fixedInventoryActionHandler(owner, itemId, _currentStrip);
	_panelState.captionText = inventoryActionCaption(_currentStrip);
	if (itemId != 0 && actionHandlerId != 0)
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
	state.inventoryOwner = 0;
	state.inventoryActionHandlerId = 0;
	state.relationMode = _relationMode;
	state.primaryInventoryItem = _primaryInventoryItem;
	state.relationModeActive = _relationMode != 0 && _primaryInventoryItem != 0;
	state.inventoryItemSelected = false;
	return state;
}

GameplayLoopCursorState GameplayLoop::makeInventoryItemState(byte owner, byte itemId, uint16 actionHandlerId) const {
	GameplayLoopCursorState state = makeCursorState();
	state.resolvedItem = itemId;
	state.inventoryOwner = owner;
	state.inventoryActionHandlerId = actionHandlerId;
	state.inventoryItemSelected = true;
	return state;
}

bool GameplayLoop::dispatchGlobalInventoryFixedAction(const GameplayLoopCursorState &state) {
	if (state.relationModeActive || !state.inventoryItemSelected)
		return false;

	return dispatchSharedInventoryAction(state.inventoryActionHandlerId);
}

bool GameplayLoop::dispatchGlobalInventoryItemAction(const GameplayLoopCursorState &state) {
	if (!state.relationModeActive || !state.inventoryItemSelected)
		return false;

	return dispatchSharedInventoryAction(state.inventoryActionHandlerId);
}

bool GameplayLoop::dispatchSharedInventoryAction(uint16 handlerId) {
	const bool sharedHandler =
		handlerId == 0 || handlerId == 1 || handlerId == 35 ||
		handlerId == 41 || handlerId == 45 || handlerId == 49 ||
		(handlerId >= 2 && handlerId <= 34) ||
		handlerId == 36 || handlerId == 37 ||
		handlerId == 38 || handlerId == 39 || handlerId == 40 ||
		handlerId == 42 || handlerId == 43 || handlerId == 44 ||
		handlerId == 46 || handlerId == 47 || handlerId == 48 ||
		(handlerId >= 50 && handlerId <= 69);
	if (!sharedHandler)
		return false;

	_vm->cursor()->leaveInteractiveMode();

	switch (handlerId) {
	case 0:
	case 1:
	case 35:
	case 41:
	case 45:
	case 49:
		break;
	case 2:
		_delegate->beginSharedInventorySpeechLine(1, _delegate->randomSharedInventorySpeechFrame(1));
		break;
	case 3:
		_delegate->beginSharedInventorySpeechLine(2, 0);
		break;
	case 4:
		_delegate->beginSharedInventorySpeechLine(3, _delegate->randomSharedInventorySpeechFrame(1));
		break;
	case 5:
	{
		const byte variant = _delegate->randomSharedInventorySpeechFrame(2);
		if (variant == 2)
			_delegate->beginSharedInventorySpeechLine(3, 1);
		else
			_delegate->beginSharedInventorySpeechLine(4, variant);
		break;
	}
	case 6:
		_delegate->beginSharedInventorySpeechLine(5, 0);
		break;
	case 7:
		_delegate->beginSharedInventorySpeechLine(6, _delegate->randomSharedInventorySpeechFrame(1));
		break;
	case 8:
		_delegate->beginSharedInventorySpeechLine(7, 0);
		break;
	case 9:
		_delegate->beginSharedInventorySpeechLine(8, 0);
		break;
	case 10:
		_delegate->beginSharedInventorySpeechLine(9, _delegate->randomSharedInventorySpeechFrame(1));
		break;
	case 11:
		_delegate->beginSharedInventorySpeechLine(0x0a, 0);
		break;
	case 12:
		_delegate->beginSharedInventorySpeechLine(0x0b, 0);
		break;
	case 13:
		_delegate->beginSharedInventorySpeechLine(0x0c, _delegate->randomSharedInventorySpeechFrame(1));
		break;
	case 14:
		_delegate->beginSharedInventorySpeechLine(0x0d, _delegate->randomSharedInventorySpeechFrame(1));
		break;
	case 15:
		_delegate->beginSharedInventorySpeechLine(0x0e, 0);
		break;
	case 16:
		_delegate->beginSharedInventorySpeechLine(0x0f, _delegate->randomSharedInventorySpeechFrame(2));
		break;
	case 17:
		_delegate->beginSharedInventorySpeechLine(0x10, 0);
		break;
	case 18:
		_delegate->beginSharedInventorySpeechLine(0x11, _delegate->randomSharedInventorySpeechFrame(1));
		break;
	case 19:
		_delegate->beginSharedInventorySpeechLine(0x12, _delegate->randomSharedInventorySpeechFrame(2));
		break;
	case 20:
		_delegate->beginSharedInventorySpeechLine(0x13, 0);
		break;
	case 21:
		_delegate->beginSharedInventorySpeechLine(0x14, 0);
		break;
	case 22:
		_delegate->beginSharedInventorySpeechLine(0x15, 0);
		break;
	case 23:
		_delegate->beginSharedInventorySpeechLine(0x16, _delegate->randomSharedInventorySpeechFrame(1));
		break;
	case 24:
		_delegate->beginSharedInventorySpeechLine(0x17, _delegate->randomSharedInventorySpeechFrame(1));
		break;
	case 25:
		_delegate->beginSharedInventorySpeechLine(0x18, _delegate->randomSharedInventorySpeechFrame(1));
		break;
	case 26:
		_delegate->beginSharedInventorySpeechLine(0x19, 0);
		break;
	case 27:
		_delegate->beginSharedInventorySpeechLine(0x1a, 0);
		break;
	case 28:
		_delegate->beginSharedInventorySpeechLine(0x1b, 0);
		break;
	case 29:
		_delegate->beginSharedInventorySpeechLine(0x1c, 0);
		break;
	case 30:
		_delegate->beginSharedInventorySpeechLine(0x1d, 0);
		break;
	case 31:
		_delegate->beginSharedInventorySpeechLine(0x1e, 0);
		break;
	case 32:
		_delegate->beginSharedInventorySpeechLine(0x1f, 0);
		break;
	case 33:
		_delegate->beginSharedInventorySpeechLine(0x20, 0);
		break;
	case 34:
		_delegate->beginSharedInventorySpeechLine(0x21, 0);
		break;
	case 36:
		_delegate->beginSharedInventorySpeechLine(0x23, 0);
		break;
	case 37:
		_delegate->beginSharedInventorySpeechLine(0x43, 1);
		_delegate->beginSharedInventorySpeechLine(0x24, 0);
		_delegate->beginSharedInventorySpeechLine(0x43, 2);
		break;
	case 38:
		_delegate->beginSharedInventorySpeechLine(0x25, 0);
		break;
	case 39:
		_delegate->beginSharedInventorySpeechLine(0x26, 0);
		break;
	case 40:
		_delegate->beginSharedInventorySpeechLine(0x27, 0);
		break;
	case 42:
		_delegate->beginSharedInventorySpeechLine(0x29, 0);
		break;
	case 43:
		_delegate->beginSharedInventorySpeechLine(0x2a, 0);
		break;
	case 44:
		_delegate->beginSharedInventorySpeechLine(0x2b, 0);
		break;
	case 46:
		_delegate->beginSharedInventorySpeechLine(0x2d, 0);
		break;
	case 47:
		_delegate->beginSharedInventorySpeechLine(0x2e, 0);
		break;
	case 48:
		_delegate->beginSharedInventorySpeechLine(0x2f, 0);
		break;
	case 50:
		_delegate->beginSharedInventorySpeechLine(0x31, 0);
		break;
	case 51:
		_delegate->beginSharedInventorySpeechLine(0x32, 0);
		break;
	case 52:
		_delegate->beginSharedInventorySpeechLine(0x33, 0);
		break;
	case 53:
		_delegate->beginSharedInventorySpeechLine(0x34, 0);
		break;
	case 54:
		_delegate->beginSharedInventorySpeechLine(0x35, 0);
		break;
	case 55:
		_delegate->beginSharedInventorySpeechLine(0x36, 0);
		break;
	case 56:
		_delegate->beginSharedInventorySpeechLine(0x37, 0);
		break;
	case 57:
		_delegate->beginSharedInventorySpeechLine(0x38, 0);
		break;
	case 58:
		_delegate->beginSharedInventorySpeechLine(0x39, 0);
		break;
	case 59:
		_delegate->beginSharedInventorySpeechLine(0x3a, 0);
		break;
	case 60:
		_delegate->beginSharedInventorySpeechLine(0x3b, 0);
		break;
	case 61:
		_delegate->beginSharedInventorySpeechLine(0x3c, 0);
		break;
	case 62:
		_delegate->beginSharedInventorySpeechLine(0x3d, 0);
		break;
	case 63:
		_delegate->beginSharedInventorySpeechLine(0x3e, 0);
		break;
	case 64:
		_delegate->beginSharedInventorySpeechLine(0x3f, 0);
		break;
	case 65:
		_delegate->beginSharedInventorySpeechLine(0x40, 0);
		break;
	case 66:
	{
		const byte owner = currentInventoryOwner();
		GameplayState &gameState = _vm->gameState();
		if (gameState.hasInventoryItem(owner, 0x22)) {
			_delegate->beginSharedInventorySpeechLine(0x41, 1);
			break;
		}
		gameState.addInventoryItem(owner, 0x22);
		_delegate->playSharedInventorySound(1);
		_delegate->beginSharedInventorySpeechLine(0x41, 0);
		break;
	}
	case 67:
		_delegate->beginSharedInventorySpeechLine(0x42, 0);
		break;
	case 68:
	{
		const byte owner = currentInventoryOwner();
		GameplayState &gameState = _vm->gameState();
		_delegate->beginSharedInventorySpeechLine(0x43, 0);
		gameState.removeInventoryItem(owner, 0x08);
		gameState.removeInventoryItem(owner, 0x0f);
		gameState.addInventoryItem(owner, 0x06);
		_delegate->playSharedInventorySound(1);
		_delegate->beginSharedInventorySpeechLine(0x43, 1);
		_delegate->beginSharedInventorySpeechLine(0x24, 0);
		_delegate->beginSharedInventorySpeechLine(0x43, 2);
		break;
	}
	case 69:
		_delegate->beginSharedInventorySpeechLine(0x44, 0);
		break;
	default:
		return false;
	}

	if (!Engine::shouldQuit() && !_delegate->shouldExitGameplayLoop()) {
		_vm->cursor()->enterInteractiveMode();
		_vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());
	}
	return true;
}

void GameplayLoop::syncHoverCaptionRelationContext() {
	if (_relationMode == 0 || _primaryInventoryItem == 0) {
		_hoverCaption.setRelationContext(0, 0, Common::String());
		return;
	}

	_hoverCaption.setRelationContext(_relationMode, _primaryInventoryItem,
		_delegate->inventoryItemName(currentInventoryOwner(), _primaryInventoryItem));
}

void GameplayLoop::refreshHoverCaption() {
	syncHoverCaptionRelationContext();
	_hoverCaption.refreshNow(_delegate->hotspots(), _delegate->savedFramebuffer(),
		_vm->cursor()->surfaceX(), _vm->cursor()->surfaceY(),
		_delegate->viewportXOffset(), _delegate->viewportYOffset());
}

} // End of namespace Hollywood
