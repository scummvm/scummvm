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

#include "hollywood/console.h"

#include "common/str.h"
#include "common/util.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/hollywood.h"
#include "hollywood/scenes/scene_registry.h"

namespace Hollywood {

const byte kFrankieDrMoscaMachineItem = 0x0a;
const byte kFrankieMachineActivatorItem = 0x1f;
const byte kFrankieBrainItem = 0x25;
const byte kFrankieRemoteControlItem = 0x05;
const byte kFrankieSunglassesItem = 0x27;
const byte kFrankieDiaryItem = 0x33;
const byte kFrankieUmbrellaItem = 0x5b;

const byte kKarnakPapyrusItem = 0x12;
const byte kKarnakTigerToothItem = 0x26;
const byte kKarnakParchmentItem = 0x2a;
const byte kKarnakSeedsItem = 0x2b;
const byte kKarnakNileFlowerItem = 0x2c;
const byte kKarnakLampItem = 0x3c;
const byte kKarnakOilItem = 0x43;
const byte kKarnakShovelItem = 0x50;
const byte kKarnakMagnifierItem = 0x5a;

const byte kFrankieBodyAssemblyItems[] = {
	0x30, 0x42, 0x4c
};

const byte kFrankieSerumItems[] = {
	0x44, 0x3e, 0x38, 0x5d, 0x57
};

static bool addInventoryItemIfMissing(GameplayState &state, byte owner, byte itemId) {
	if (state.hasInventoryItem(owner, itemId))
		return false;

	state.addInventoryItem(owner, itemId);
	return state.hasInventoryItem(owner, itemId);
}

Console::Console(HollywoodEngine *vm) :
		GUI::Debugger(),
		_vm(vm) {
	registerCmd("get", WRAP_METHOD(Console, cmdGet));
	registerCmd("solve", WRAP_METHOD(Console, cmdSolve));
}

bool Console::cmdGet(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s <item id|all|frankie|karnak>\n", argv[0]);
		return true;
	}

	GameplayState &state = _vm->gameState();
	const byte owner = inventoryOwner();
	state.currentInventoryOwnerIndex = owner;
	if (owner == 0)
		state.initializeRonItemResourcePages();
	else if (owner == 1)
		state.initializeSueItemResourcePages();

	const Common::String argument(argv[1]);
	if (argument.equalsIgnoreCase("all")) {
		const byte itemCount = state.giveInventoryItemsWithResourcePages(owner);
		debugPrintf("Added %u inventory items with resource pages to owner %u\n",
			itemCount, owner);
		return true;
	}
	if (argument.equalsIgnoreCase("frankie")) {
		if (owner != 0) {
			debugPrintf("Frankenstein revival items can only be given to Ron\n");
			return true;
		}

		uint addedCount = 0;
		addedCount += addInventoryItemIfMissing(state, owner, kFrankieUmbrellaItem);
		if (!state.scene3040HiddenObjectVisible && !state.scene3030MachineActivated)
			addedCount += addInventoryItemIfMissing(state, owner, kFrankieDrMoscaMachineItem);
		if (!state.scene3030MachineActivated)
			addedCount += addInventoryItemIfMissing(state, owner, kFrankieMachineActivatorItem);
		if (!state.frankensteinDiaryRead)
			addedCount += addInventoryItemIfMissing(state, owner, kFrankieDiaryItem);

		if (state.scene3070FrankensteinBodyState == 0) {
			for (uint i = 0; i < ARRAYSIZE(kFrankieBodyAssemblyItems); ++i)
				addedCount += addInventoryItemIfMissing(state, owner,
					kFrankieBodyAssemblyItems[i]);
		}
		if (state.scene3070FrankensteinBodyState < 2)
			addedCount += addInventoryItemIfMissing(state, owner, kFrankieBrainItem);

		const uint serumItemCount = ARRAYSIZE(kFrankieSerumItems);
		uint serumProgress = MIN<uint>(state.scene3070SerumIngredientCount, serumItemCount);
		for (uint i = 0; i < serumItemCount; ++i) {
			if (state.hasInventoryItem(owner, kFrankieSerumItems[i]))
				++serumProgress;
		}
		for (uint i = 0; i < serumItemCount && serumProgress < serumItemCount; ++i) {
			if (addInventoryItemIfMissing(state, owner, kFrankieSerumItems[i])) {
				++addedCount;
				++serumProgress;
			}
		}

		addedCount += addInventoryItemIfMissing(state, owner, kFrankieRemoteControlItem);
		addedCount += addInventoryItemIfMissing(state, owner, kFrankieSunglassesItem);
		debugPrintf("Added %u inventory items needed for Frankenstein's revival\n", addedCount);
		return true;
	}
	if (argument.equalsIgnoreCase("karnak") || argument.equalsIgnoreCase("karnac")) {
		if (owner != 0) {
			debugPrintf("Sphinx and Karnak ceremony items can only be given to Ron\n");
			return true;
		}

		uint addedCount = 0;
		if (state.scene2040SphinxFaceState == 0)
			addedCount += addInventoryItemIfMissing(state, owner, kKarnakShovelItem);
		if (state.scene2040SphinxFaceState < 2)
			addedCount += addInventoryItemIfMissing(state, owner, kKarnakSeedsItem);
		if (state.scene2040SphinxBasePatchState == 0)
			addedCount += addInventoryItemIfMissing(state, owner, kKarnakTigerToothItem);
		if (!state.scene2050LabyrinthLampReady) {
			addedCount += addInventoryItemIfMissing(state, owner, kKarnakLampItem);
			if (!state.ronLampFueled)
				addedCount += addInventoryItemIfMissing(state, owner, kKarnakOilItem);
		}
		addedCount += addInventoryItemIfMissing(state, owner, kKarnakPapyrusItem);
		addedCount += addInventoryItemIfMissing(state, owner, kKarnakParchmentItem);
		addedCount += addInventoryItemIfMissing(state, owner, kKarnakMagnifierItem);
		if (!state.scene2020PrincessGone)
			addedCount += addInventoryItemIfMissing(state, owner, kKarnakNileFlowerItem);
		const bool bypassedInterview = state.scene2040SphinxExitInterviewState < 2;
		if (bypassedInterview)
			state.scene2040SphinxExitInterviewState = 2;

		debugPrintf("Added %u inventory items needed for the sphinx and Karnak ceremony\n",
			addedCount);
		if (bypassedInterview)
			debugPrintf("Enabled the sphinx entrance without the prerequisite conversations\n");
		return true;
	}

	uint itemId = 0;
	if (!parseItemId(argv[1], itemId)) {
		debugPrintf("Invalid inventory item id '%s'\n", argv[1]);
		debugPrintf("Usage: %s <item id|all|frankie|karnak>\n", argv[0]);
		return true;
	}

	if (itemId == 0 || itemId >= GameplayState::kInventoryOwnerSlotStride) {
		debugPrintf("Inventory item id %u is out of range; valid range is 1..%u\n",
			itemId, GameplayState::kInventoryLastSlot);
		return true;
	}

	if (state.inventoryItemResourcePageByOwnerAndItemId[owner][itemId] == 0) {
		debugPrintf("Inventory item %u does not belong to owner %u or has no resource page\n",
			itemId, owner);
		return true;
	}

	if (state.hasInventoryItem(owner, (byte)itemId)) {
		debugPrintf("Inventory owner %u already has item %u\n", owner, itemId);
		return true;
	}

	state.addInventoryItem(owner, (byte)itemId);
	if (state.hasInventoryItem(owner, (byte)itemId))
		debugPrintf("Added inventory item %u to owner %u\n", itemId, owner);
	else
		debugPrintf("Could not add inventory item %u to owner %u\n", itemId, owner);
	return true;
}

bool Console::cmdSolve(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	GameplayState &state = _vm->gameState();
	const int sceneNumber = gameplaySceneNumberForState(state.mainFlowStateId);
	switch (sceneNumber) {
	case 2050: {
		for (uint tile = 0; tile < GameplayState::kScene2050MuralTilePermutationSize; ++tile)
			state.scene2050MuralTilePermutation[tile] = (byte)tile;
		const bool guidedPuzzle = state.scene2050MuralPuzzleState == 1 ||
			state.scene2050MuralPuzzleState == 3;
		const bool alreadySolved = state.scene2050MuralPuzzleState >= 2 &&
			state.egyptSealPuzzleProgress != 0;
		state.scene2050MuralPuzzleState = guidedPuzzle ? 3 : 2;

		if (alreadySolved) {
			debugPrintf("The scene 2050 scarab mural is already solved\n");
			return true;
		}

		debugPrintf("Solved the scene 2050 scarab mural; close the debugger to play the completion sequence\n");
		return true;
	}
	case 3060:
		if (state.scene3060SecretDoorRevealState != 0) {
			debugPrintf("The secret passage to Frankie's laboratory is already open\n");
			return true;
		}

		state.scene3060SecretDoorRevealState = 1;
		_vm->requestSceneRestart();
		debugPrintf("Solved the scene 3060 globe puzzle; close the debugger to refresh the scene\n");
		return true;
	default:
		if (sceneNumber < 0)
			debugPrintf("State 0x%04x does not belong to a gameplay scene\n",
				(uint)state.mainFlowStateId);
		else
			debugPrintf("No puzzle solver is available for scene %d\n", sceneNumber);
		return true;
	}
}

bool Console::parseItemId(const char *argument, uint &itemId) {
	char *endPtr = nullptr;
	const long parsedValue = strtol(argument, &endPtr, 0);
	if (endPtr == argument || *endPtr != 0 || parsedValue < 0)
		return false;

	itemId = (uint)parsedValue;
	return true;
}

byte Console::inventoryOwner() const {
	const GameplayState &state = _vm->gameState();
	if (state.currentInventoryOwnerIndex < GameplayState::kInventoryOwnerCount)
		return state.currentInventoryOwnerIndex;

	return 0;
}

} // End of namespace Hollywood
