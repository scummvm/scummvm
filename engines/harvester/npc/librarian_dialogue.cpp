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


#include "harvester/npc/librarian_dialogue.h"

#include "common/array.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kLibrarianNpc = "LIBRARIAN";
static const char *const kPcSpeaker = "PC";
static const char *const kRahRoomName = "RAH";
static const int kLibrarianResponseLineIndex = 0xfa;

static const DialogueLineEntry kLibrarianCainbookTradeLines[] = {
	{ 0x1db1, kLibrarianNpc, 1 },
	{ 0x1db7, kPcSpeaker, 0 },
	{ 0x1dbb, kLibrarianNpc, 0 },
	{ 0x1dc2, kPcSpeaker, 4 },
	{ 0x1dc6, kLibrarianNpc, 0 }
};

static const DialogueLineEntry kLibrarianIntroLines[] = {
	{ 0x1d5b, kLibrarianNpc, 0 },
	{ 0x1d5f, kPcSpeaker, 0 },
	{ 0x1d63, kLibrarianNpc, 2 },
	{ 0x1d67, kPcSpeaker, 0 },
	{ 0x1d6c, kLibrarianNpc, 0 },
	{ 0x1d77, kLibrarianNpc, 0 }
};

static const DialogueLineEntry kLibrarianNoItemFollowupLines[] = {
	{ 0x1d8f, kLibrarianNpc, 0 },
	{ 0x1d94, kPcSpeaker, 0 },
	{ 0x1d98, kLibrarianNpc, 0 },
	{ 0x1da1, kPcSpeaker, 0 },
	{ 0x1da5, kLibrarianNpc, 0 }
};

} // End of namespace

bool LibrarianDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase(kLibrarianNpc);
}

Common::Error LibrarianDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &) {
	auto hasVisibleInventoryItem = [&](const char *objectName) {
		Common::Array<ObjectRecord> inventoryObjects;
		runtime.startupScript().getVisibleInventoryObjects(inventoryObjects);
		for (const ObjectRecord &inventoryObject : inventoryObjects) {
			if (inventoryObject.objectName.equalsIgnoreCase(objectName))
				return true;
		}

		return false;
	};
	auto playSequence = [&](const DialogueLineEntry *lines, uint count) {
		return runtime.playDialogueEntrySequence(lines, count);
	};
	auto completeCainbookTrade = [&]() -> Common::Error {
		(void)runtime.startupScript().addRuntimeObjectToInventory("CLUE");

		Common::Error lineError = playSequence(
			kLibrarianCainbookTradeLines, ARRAYSIZE(kLibrarianCainbookTradeLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		(void)runtime.startupScript().setRuntimeObjectVisible(
			kRahRoomName, "CAINBOOK", false);
		return Common::kNoError;
	};

	if (!hasVisibleInventoryItem("CLUE") && hasVisibleInventoryItem("CAINBOOK"))
		return completeCainbookTrade();

	if (!usedItemName.empty()) {
		if (!usedItemName.equalsIgnoreCase("CAINBOOK"))
			return runtime.playDialogueLine(0x1d5b, kLibrarianNpc);

		return completeCainbookTrade();
	}

	Common::Error lineError = playSequence(kLibrarianIntroLines, ARRAYSIZE(kLibrarianIntroLines));
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	int responseIndex = 0;
	Common::Error responseError = runtime.runResponseMenu(kLibrarianResponseLineIndex, responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	if (responseIndex == 1) {
		lineError = runtime.playDialogueLine(0x1d85, kLibrarianNpc);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	} else if (responseIndex == 2) {
		lineError = runtime.playDialogueLineWithVariant(0x1d8b, kLibrarianNpc, 4);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	return playSequence(kLibrarianNoItemFollowupLines, ARRAYSIZE(kLibrarianNoItemFollowupLines));
}

} // End of namespace Harvester
