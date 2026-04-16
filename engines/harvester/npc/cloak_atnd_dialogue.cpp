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


#include "harvester/npc/cloak_atnd_dialogue.h"

#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kCloakAtndNpc = "CLOAK_ATND";
static const char *const kCleanedClothesFlag = "CLEANED_CLOTHES";
static const char *const kCleanClothesActionTag = "CLEAN_CLOTHES";
static const char *const kPaymentItemName = "BARCASHFIVE";
static const int kNeedCleaningPaymentLine = 0x1066;

} // End of namespace

bool CloakAtndDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase(kCloakAtndNpc);
}

Common::Error CloakAtndDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &) {
	if (!usedItemName.empty()) {
		if (!usedItemName.equalsIgnoreCase(kPaymentItemName))
			return Common::kNoError;

		InteractionResult interaction;
		if (runtime.executeActionTag(kCleanClothesActionTag, interaction)) {
			runtime.applyImmediateDialogueInteractionEffects(interaction);
			runtime.queueDialogueInteractionIfNeeded(interaction);
		}

		return Common::kNoError;
	}

	if (runtime.startupScript().getFlagValue(kCleanedClothesFlag))
		return Common::kNoError;

	return runtime.playDialogueLine(kNeedCleaningPaymentLine, kCloakAtndNpc);
}

} // End of namespace Harvester
