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

#ifndef HOLLYWOOD_GAMEPLAY_GAME_LOOP_H
#define HOLLYWOOD_GAMEPLAY_GAME_LOOP_H

#include "common/array.h"
#include "common/keyboard.h"
#include "common/path.h"
#include "common/str.h"
#include "common/types.h"

#include "hollywood/gameplay/hotspots.h"
#include "hollywood/gameplay/inventory_media.h"

namespace Graphics {
struct Surface;
}

namespace Hollywood {

class HollywoodEngine;

struct GameplayLoopCursorState {
	uint16 cursorX;
	uint16 cursorY;
	uint16 sceneX;
	uint16 sceneY;
	byte currentStrip;
	byte requestedStrip;
	byte resolvedItem;
	byte inventoryOwner;
	uint16 inventoryActionHandlerId;
	byte relationMode;
	byte primaryInventoryItem;
	bool relationModeActive;
	bool inventoryItemSelected;
};

struct GameplayPanelState {
	GameplayPanelState();

	bool visible() const { return verbPanelVisible || inventoryPanelVisible; }

	bool verbPanelVisible;
	bool inventoryPanelVisible;
	byte currentStrip;
	byte requestedStrip;
	byte resolvedItem;
	Common::String itemName;
	Common::String captionText;
};

/**
 * Supplies scene behavior to GameplayLoop.
 *
 * GameplayLoop polls input and owns cursor and panel state, forwarding resolved
 * actions here. The delegate advances and draws the scene, presents the composed
 * frame, runs blocking auxiliary viewers with scene-safe restoration, and
 * reports scene-driven exit conditions.
 */
class GameplayLoopDelegate {
public:
	virtual ~GameplayLoopDelegate();

	virtual const SceneHotspotTable &hotspots() const = 0;
	virtual const Graphics::Surface &savedFramebuffer() const = 0;
	virtual uint16 viewportXOffset() const = 0;
	virtual uint16 viewportYOffset() const;

	virtual void prepareGameplayLoop();
	virtual void advanceGameplayLoop(uint32 deltaMillis) = 0;
	virtual void drawGameplayFrame() = 0;
	virtual void presentGameplayFrame(const SceneHoverCaption &hoverCaption, const GameplayPanelState &panelState) = 0;
	virtual void invalidatePresentationPalette();
	virtual void prepareOptionsMenuPalette(Common::Array<byte> &palette) const;
	virtual Common::Path optionsMenuSoundBank0ArchiveName() const;
	virtual bool optionsMenuSpeechPreviewSampleId(uint16 &sampleId) const;
	virtual void suspendAudioForOptionsMenu();
	virtual bool shouldExitGameplayLoop() const;
	virtual bool isInventoryPanelAvailable() const;
	virtual Common::String inventoryItemName(byte owner, byte itemId) const;
	virtual void beginSharedInventorySpeechLine(uint16 rowIndex, byte frameIndex);
	virtual byte randomSharedInventorySpeechFrame(byte maxFrameIndex);
	virtual void playSharedInventorySound(byte sampleId);
	virtual void showTravelScreenViewer();
	// Returns false only when the requested media cannot be loaded.
	virtual bool showInventoryMedia(InventoryMediaId mediaId);
	virtual bool playSueTapeRecording();
	virtual void handleLeftClick(const GameplayLoopCursorState &state);
	virtual void handleInventoryItemClick(const GameplayLoopCursorState &state);
	virtual void handleRightClick(const GameplayLoopCursorState &state);
};

/**
 * Runs the blocking interactive loop for one playable scene.
 *
 * run() prepares the delegate and presents an initial frame, then polls input
 * and advances, draws, and presents through the delegate until the engine quits
 * or the delegate requests an exit.
 */
class GameplayLoop {
public:
	GameplayLoop(HollywoodEngine *vm, GameplayLoopDelegate *delegate);

	bool run();
	void setCurrentStrip(byte stripIndex);

	byte currentStrip() const { return _currentStrip; }
	const SceneHoverCaption &hoverCaption() const { return _hoverCaption; }

private:
	bool pollEvents();
	void handleKeyDown(const Common::KeyState &keyState);
	void handleKeyUp(const Common::KeyState &keyState);
	void handleLeftClick();
	void handleRightClick();
	void enterKeyboardStripMode();
	void leaveKeyboardStripMode();
	void selectPreviousKeyboardStrip();
	void selectNextKeyboardStrip();
	void selectNextStrip();
	void openVerbPanel();
	void closeVerbPanel();
	void openInventoryPanel();
	void closeInventoryPanel();
	void openOptionsMenu();
	void updatePanelHover(uint32 deltaMillis);
	void updatePanelFromMousePosition();
	void selectPanelStrip(byte stripIndex);
	byte panelStripAt(uint16 cursorX, uint16 cursorY) const;
	byte inventoryPanelStripAt(uint16 cursorX, uint16 cursorY) const;
	uint inventoryPanelSlotAtPosition(uint16 cursorX, uint16 cursorY) const;
	byte inventoryItemAtPanelPosition(uint16 cursorX, uint16 cursorY) const;
	byte currentInventoryOwner() const;
	uint16 fixedInventoryActionHandler(byte owner, byte itemId, byte stripIndex) const;
	uint16 dialogueInventoryRelationHandler(byte primaryItemId, byte secondaryItemId, byte relationMode) const;
	bool scrollInventoryPanelPreviousPage();
	bool scrollInventoryPanelNextPage();
	bool isInventoryPanelOptionsButton(uint16 cursorX, uint16 cursorY) const;
	bool isInventoryPanelPreviousPageArrow(uint16 cursorX, uint16 cursorY) const;
	bool isInventoryPanelNextPageArrow(uint16 cursorX, uint16 cursorY) const;
	void updatePanelCaption();
	void updateInventoryPanelCaption();
	void syncPanelState();
	GameplayLoopCursorState makeCursorState() const;
	GameplayLoopCursorState makeInventoryItemState(byte owner, byte itemId, uint16 actionHandlerId) const;
	void clearButtonLatches();
	void syncHoverCaptionRelationContext();
	void refreshHoverCaption();

	HollywoodEngine *_vm;
	GameplayLoopDelegate *_delegate;
	SceneHoverCaption _hoverCaption;
	byte _currentStrip;
	bool _leftButtonDown;
	bool _rightButtonDown;
	bool _keyboardStripMode;
	bool _inventoryPanelOpenedFromDefault;
	byte _relationMode;
	byte _primaryInventoryItem;
	GameplayPanelState _panelState;
	uint32 _panelHoverTimer;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_GAMEPLAY_GAME_LOOP_H
