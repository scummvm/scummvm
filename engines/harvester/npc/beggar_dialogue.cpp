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

#include <functional>

#include "harvester/npc/beggar_dialogue.h"

#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kBeggarInterruptFlag = "BEGGAR_INTERRUPT_CONVERSATION";
static const char *const kBeggarDialog3ActionTag = "BEGGAR_DIALOG_3";

} // End of namespace

bool BeggarDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("BEGGAR");
}

Common::Error BeggarDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &, DialogueSharedState &) {
	if (!runtime.startupScript().getFlagValue(kBeggarInterruptFlag))
		return Common::kNoError;

	StartupInteractionResult interaction;
	if (runtime.startupScript().executeActionTag(kBeggarDialog3ActionTag, interaction)) {
		runtime.applyImmediateDialogueInteractionEffects(interaction);
		runtime.queueDialogueInteractionIfNeeded(interaction);
	}

	return runtime.playDialogueLineWithVariant(0x1276, "BEGGAR", 1);
}

} // End of namespace Harvester
