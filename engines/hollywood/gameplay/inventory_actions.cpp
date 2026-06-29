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

#include "hollywood/gameplay/inventory_actions.h"

#include "common/events.h"
#include "common/system.h"

#include "hollywood/gameplay/cursor.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

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

void GameplayLoopDelegate::showTravelScreenViewer() {
}

bool isRonInventoryAction(uint16 handlerId) {
	switch (handlerId) {
	case 74:
	case 75:
	case 76:
	case 85:
	case 86:
	case 87:
	case 215:
	case 216:
	case 217:
		return true;
	default:
		return false;
	}
}

void dispatchRonInventoryAction(HollywoodEngine *vm, GameplayLoopDelegate *delegate,
		uint16 handlerId, byte owner) {
	GameplayState &gameState = vm->gameState();

	switch (handlerId) {
	case 74:
		delegate->beginSharedInventorySpeechLine(0x46, 0);
		break;
	case 75:
		delegate->showTravelScreenViewer();
		break;
	case 76:
		delegate->beginSharedInventorySpeechLine(0x47, 0);
		break;
	case 85:
		if (gameState.ronTapeRecorderState == 0) {
			delegate->beginSharedInventorySpeechLine(0x50, 0);
			gameState.ronTapeRecorderState = 1;
		} else if (gameState.ronTapeRecorderState == 1) {
			delegate->beginSharedInventorySpeechLine(0x50, 1);
		} else if (gameState.ronTapeRecorderState == 2) {
			delegate->beginSharedInventorySpeechLine(0x50, 2);
		}
		break;
	case 86:
		if (gameState.ronTapeRecorderState < 2)
			delegate->beginSharedInventorySpeechLine(0x50, 1);
		break;
	case 87:
		if (gameState.ronTapeRecorderState < 2) {
			delegate->beginSharedInventorySpeechLine(0x50, 1);
		} else {
			gameState.addInventoryItem(owner, 0x16);
			delegate->playSharedInventorySound(1);
			gameState.ronTapeRecorderState = 1;
		}
		break;
	case 215:
		delegate->beginSharedInventorySpeechLine(0xc8, 0);
		break;
	case 216:
		delegate->beginSharedInventorySpeechLine(0xc9, 0);
		break;
	case 217:
		if (gameState.ronWalletOpened) {
			delegate->beginSharedInventorySpeechLine(0xca, 1);
			break;
		}
		gameState.addInventoryItem(owner, 0x29);
		delegate->playSharedInventorySound(1);
		gameState.ronWalletOpened = true;
		delegate->beginSharedInventorySpeechLine(0xca, 0);
		break;
	default:
		break;
	}
}

bool dispatchInventoryFixedAction(HollywoodEngine *vm, GameplayLoopDelegate *delegate,
		const GameplayLoopCursorState &state) {
	if (state.relationModeActive || !state.inventoryItemSelected)
		return false;

	return dispatchSharedInventoryAction(vm, delegate, state.inventoryActionHandlerId,
		state.inventoryOwner);
}

bool dispatchInventoryRelationAction(HollywoodEngine *vm, GameplayLoopDelegate *delegate,
		const GameplayLoopCursorState &state) {
	if (!state.relationModeActive || !state.inventoryItemSelected)
		return false;

	return dispatchSharedInventoryAction(vm, delegate, state.inventoryActionHandlerId,
		state.inventoryOwner);
}

bool dispatchSharedInventoryAction(HollywoodEngine *vm, GameplayLoopDelegate *delegate,
		uint16 handlerId, byte owner) {
	// Mirrors the low shared table installed by InstallSceneActionCallbackTable
	// at 004d6000. This is not the larger main-scene callback table.
	const bool sharedHandler =
		handlerId == 0 || handlerId == 1 || handlerId == 35 ||
		handlerId == 41 || handlerId == 45 || handlerId == 49 ||
		(handlerId >= 2 && handlerId <= 34) ||
		handlerId == 36 || handlerId == 37 ||
		handlerId == 38 || handlerId == 39 || handlerId == 40 ||
		handlerId == 42 || handlerId == 43 || handlerId == 44 ||
		handlerId == 46 || handlerId == 47 || handlerId == 48 ||
		(handlerId >= 50 && handlerId <= 71);
	const bool ronHandler = owner == 0 && isRonInventoryAction(handlerId);
	if (!sharedHandler && !ronHandler)
		return false;

	vm->cursor()->leaveInteractiveMode();

	if (ronHandler) {
		dispatchRonInventoryAction(vm, delegate, handlerId, owner);
		if (!Engine::shouldQuit() && !delegate->shouldExitGameplayLoop()) {
			vm->cursor()->enterInteractiveMode();
			vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());
		}
		return true;
	}

	switch (handlerId) {
	case 0:
	case 1:
	case 35:
	case 41:
	case 45:
	case 49:
		// Original default callback: consume the action without speech.
		break;
	case 2:
		delegate->beginSharedInventorySpeechLine(1, delegate->randomSharedInventorySpeechFrame(1));
		break;
	case 3:
		delegate->beginSharedInventorySpeechLine(2, 0);
		break;
	case 4:
		delegate->beginSharedInventorySpeechLine(3, delegate->randomSharedInventorySpeechFrame(1));
		break;
	case 5:
	{
		const byte variant = delegate->randomSharedInventorySpeechFrame(2);
		if (variant == 2)
			delegate->beginSharedInventorySpeechLine(3, 1);
		else
			delegate->beginSharedInventorySpeechLine(4, variant);
		break;
	}
	case 6:
		delegate->beginSharedInventorySpeechLine(5, 0);
		break;
	case 7:
		delegate->beginSharedInventorySpeechLine(6, delegate->randomSharedInventorySpeechFrame(1));
		break;
	case 8:
		delegate->beginSharedInventorySpeechLine(7, 0);
		break;
	case 9:
		delegate->beginSharedInventorySpeechLine(8, 0);
		break;
	case 10:
		delegate->beginSharedInventorySpeechLine(9, delegate->randomSharedInventorySpeechFrame(1));
		break;
	case 11:
		delegate->beginSharedInventorySpeechLine(0x0a, 0);
		break;
	case 12:
		delegate->beginSharedInventorySpeechLine(0x0b, 0);
		break;
	case 13:
		delegate->beginSharedInventorySpeechLine(0x0c, delegate->randomSharedInventorySpeechFrame(1));
		break;
	case 14:
		delegate->beginSharedInventorySpeechLine(0x0d, delegate->randomSharedInventorySpeechFrame(1));
		break;
	case 15:
		delegate->beginSharedInventorySpeechLine(0x0e, 0);
		break;
	case 16:
		delegate->beginSharedInventorySpeechLine(0x0f, delegate->randomSharedInventorySpeechFrame(2));
		break;
	case 17:
		delegate->beginSharedInventorySpeechLine(0x10, 0);
		break;
	case 18:
		delegate->beginSharedInventorySpeechLine(0x11, delegate->randomSharedInventorySpeechFrame(1));
		break;
	case 19:
		delegate->beginSharedInventorySpeechLine(0x12, delegate->randomSharedInventorySpeechFrame(2));
		break;
	case 20:
		delegate->beginSharedInventorySpeechLine(0x13, 0);
		break;
	case 21:
		delegate->beginSharedInventorySpeechLine(0x14, 0);
		break;
	case 22:
		delegate->beginSharedInventorySpeechLine(0x15, 0);
		break;
	case 23:
		delegate->beginSharedInventorySpeechLine(0x16, delegate->randomSharedInventorySpeechFrame(1));
		break;
	case 24:
		delegate->beginSharedInventorySpeechLine(0x17, delegate->randomSharedInventorySpeechFrame(1));
		break;
	case 25:
		delegate->beginSharedInventorySpeechLine(0x18, delegate->randomSharedInventorySpeechFrame(1));
		break;
	case 26:
		delegate->beginSharedInventorySpeechLine(0x19, 0);
		break;
	case 27:
		delegate->beginSharedInventorySpeechLine(0x1a, 0);
		break;
	case 28:
		delegate->beginSharedInventorySpeechLine(0x1b, 0);
		break;
	case 29:
		delegate->beginSharedInventorySpeechLine(0x1c, 0);
		break;
	case 30:
		delegate->beginSharedInventorySpeechLine(0x1d, 0);
		break;
	case 31:
		delegate->beginSharedInventorySpeechLine(0x1e, 0);
		break;
	case 32:
		delegate->beginSharedInventorySpeechLine(0x1f, 0);
		break;
	case 33:
		delegate->beginSharedInventorySpeechLine(0x20, 0);
		break;
	case 34:
		delegate->beginSharedInventorySpeechLine(0x21, 0);
		break;
	case 36:
		delegate->beginSharedInventorySpeechLine(0x23, 0);
		break;
	case 37:
		// Original callback 0x25: row 0x43 frame 1, row 0x24 frame 0,
		// then row 0x43 frame 2, with a render pass between each line.
		delegate->beginSharedInventorySpeechLine(0x43, 1);
		delegate->beginSharedInventorySpeechLine(0x24, 0);
		delegate->beginSharedInventorySpeechLine(0x43, 2);
		break;
	case 38:
		delegate->beginSharedInventorySpeechLine(0x25, 0);
		break;
	case 39:
		delegate->beginSharedInventorySpeechLine(0x26, 0);
		break;
	case 40:
		delegate->beginSharedInventorySpeechLine(0x27, 0);
		break;
	case 42:
		delegate->beginSharedInventorySpeechLine(0x29, 0);
		break;
	case 43:
		delegate->beginSharedInventorySpeechLine(0x2a, 0);
		break;
	case 44:
		delegate->beginSharedInventorySpeechLine(0x2b, 0);
		break;
	case 46:
		delegate->beginSharedInventorySpeechLine(0x2d, 0);
		break;
	case 47:
		delegate->beginSharedInventorySpeechLine(0x2e, 0);
		break;
	case 48:
		delegate->beginSharedInventorySpeechLine(0x2f, 0);
		break;
	case 50:
		delegate->beginSharedInventorySpeechLine(0x31, 0);
		break;
	case 51:
		delegate->beginSharedInventorySpeechLine(0x32, 0);
		break;
	case 52:
		delegate->beginSharedInventorySpeechLine(0x33, 0);
		break;
	case 53:
		delegate->beginSharedInventorySpeechLine(0x34, 0);
		break;
	case 54:
		delegate->beginSharedInventorySpeechLine(0x35, 0);
		break;
	case 55:
		delegate->beginSharedInventorySpeechLine(0x36, 0);
		break;
	case 56:
		delegate->beginSharedInventorySpeechLine(0x37, 0);
		break;
	case 57:
		delegate->beginSharedInventorySpeechLine(0x38, 0);
		break;
	case 58:
		delegate->beginSharedInventorySpeechLine(0x39, 0);
		break;
	case 59:
		delegate->beginSharedInventorySpeechLine(0x3a, 0);
		break;
	case 60:
		delegate->beginSharedInventorySpeechLine(0x3b, 0);
		break;
	case 61:
		delegate->beginSharedInventorySpeechLine(0x3c, 0);
		break;
	case 62:
		delegate->beginSharedInventorySpeechLine(0x3d, 0);
		break;
	case 63:
		delegate->beginSharedInventorySpeechLine(0x3e, 0);
		break;
	case 64:
		delegate->beginSharedInventorySpeechLine(0x3f, 0);
		break;
	case 65:
		delegate->beginSharedInventorySpeechLine(0x40, 0);
		break;
	case 66:
	{
		// Original callback 0x42: grant item 0x22 once.
		GameplayState &gameState = vm->gameState();
		if (gameState.hasInventoryItem(owner, 0x22)) {
			delegate->beginSharedInventorySpeechLine(0x41, 1);
			break;
		}
		gameState.addInventoryItem(owner, 0x22);
		delegate->playSharedInventorySound(1);
		delegate->beginSharedInventorySpeechLine(0x41, 0);
		break;
	}
	case 67:
		delegate->beginSharedInventorySpeechLine(0x42, 0);
		break;
	case 68:
	{
		// Original callback 0x44: combine item 0x08 with item 0x0f,
		// grant item 0x06, then run callback 0x25's speech sequence.
		GameplayState &gameState = vm->gameState();
		delegate->beginSharedInventorySpeechLine(0x43, 0);
		gameState.removeInventoryItem(owner, 0x08);
		gameState.removeInventoryItem(owner, 0x0f);
		gameState.addInventoryItem(owner, 0x06);
		delegate->playSharedInventorySound(1);
		delegate->beginSharedInventorySpeechLine(0x43, 1);
		delegate->beginSharedInventorySpeechLine(0x24, 0);
		delegate->beginSharedInventorySpeechLine(0x43, 2);
		break;
	}
	case 69:
		delegate->beginSharedInventorySpeechLine(0x44, 0);
		break;
	case 70:
	{
		// Original callback 0x46: advance item 0x1a through its alternate pages.
		GameplayState &gameState = vm->gameState();
		if ((gameState.multiToolKnifeState & 1) != 0) {
			if (gameState.multiToolKnifeState < 9)
				++gameState.multiToolKnifeState;
			else
				gameState.multiToolKnifeState = 0;
		}

		byte pageIndex = 0;
		switch (gameState.multiToolKnifeState) {
		case 0:
			gameState.multiToolKnifeState = 1;
			pageIndex = 0x1e;
			break;
		case 2:
			gameState.multiToolKnifeState = 3;
			pageIndex = 0x2d;
			break;
		case 4:
			gameState.multiToolKnifeState = 5;
			pageIndex = 0x7d;
			break;
		case 6:
			gameState.multiToolKnifeState = 7;
			pageIndex = 0x0e;
			break;
		case 8:
			gameState.multiToolKnifeState = 9;
			pageIndex = 0x18;
			break;
		default:
			break;
		}

		if (owner < GameplayState::kInventoryOwnerCount && pageIndex != 0) {
			gameState.inventoryItemResourcePageByOwnerAndItemId[owner][0x1a] = pageIndex;
			gameState.inventoryPanelDirty = true;
		}
		delegate->playSharedInventorySound(1);
		delegate->beginSharedInventorySpeechLine(0x15, 0);
		break;
	}
	case 71:
	{
		// Original callback 0x47: reset item 0x1a to page 0x68 after a valid state.
		GameplayState &gameState = vm->gameState();
		if ((gameState.multiToolKnifeState & 1) == 0) {
			delegate->beginSharedInventorySpeechLine(0x0b, 0);
			break;
		}

		if (gameState.multiToolKnifeState == 9)
			gameState.multiToolKnifeState = 0;
		else
			++gameState.multiToolKnifeState;

		if (owner < GameplayState::kInventoryOwnerCount) {
			gameState.inventoryItemResourcePageByOwnerAndItemId[owner][0x1a] = 0x68;
			gameState.inventoryPanelDirty = true;
		}
		delegate->playSharedInventorySound(1);
		delegate->beginSharedInventorySpeechLine(0x15, 0);
		break;
	}
	default:
		return false;
	}

	if (!Engine::shouldQuit() && !delegate->shouldExitGameplayLoop()) {
		vm->cursor()->enterInteractiveMode();
		vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());
	}
	return true;
}

} // End of namespace Hollywood
