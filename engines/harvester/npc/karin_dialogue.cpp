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


#include "harvester/npc/karin_dialogue.h"

#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kKarinNpc = "KARIN";
static const char *const kKarinStandingNpc = "KARIN_STANDING";
static const char *const kKarinCem10Npc = "KARIN_CEM10";
static const char *const kKarinOfficeNpc = "KARIN_OFFICE";
static const char *const kPcSpeaker = "PC";
static const char *const kEdnaNpc = "EDNA";

static const DialogueLineEntry kKarinCem10AliveLines[] = {
	{ 0x10a5, kPcSpeaker, 4 },
	{ 0x10a9, kKarinNpc, 3 },
	{ 0x10ae, kPcSpeaker, 2 },
	{ 0x10b2, kKarinNpc, 3 },
	{ 0x10b6, kPcSpeaker, 0 },
	{ 0x10ba, kKarinNpc, 3 },
	{ 0x10bf, kPcSpeaker, 0 },
	{ 0x10c3, kKarinNpc, 3 }
};

static const DialogueLineEntry kKarinFirstNoItemLines[] = {
	{ 0x1076, kPcSpeaker, 1 },
	{ 0x107a, kKarinNpc, 1 },
	{ 0x107e, kPcSpeaker, 1 },
	{ 0x1082, kKarinNpc, 1 },
	{ 0x1087, kPcSpeaker, 0 },
	{ 0x108b, kKarinNpc, 1 }
};

static const DialogueLineEntry kKarinFirstPhotoReplyLines[] = {
	{ 0x10e3, kKarinNpc, 4 },
	{ 0x10e7, kEdnaNpc, 2 }
};

static const DialogueLineEntry kKarinInvMagLines[] = {
	{ 0x10ee, kKarinNpc, 0 },
	{ 0x10f2, kEdnaNpc, 2 }
};

} // End of namespace

bool KarinDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase(kKarinNpc) ||
		npcName.equalsIgnoreCase(kKarinStandingNpc) ||
		npcName.equalsIgnoreCase(kKarinCem10Npc) ||
		npcName.equalsIgnoreCase(kKarinOfficeNpc);
}

Common::Error KarinDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	KarinRoomDialogueState &state = _state;
	auto playKarinLine = [&](int wavId, int headVariant = 0) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, kKarinNpc, headVariant);
	};
	auto playSequence = [&](const DialogueLineEntry *lines, uint count) -> Common::Error {
		return runtime.playDialogueEntrySequence(lines, count);
	};
	auto executeActionTagIfSet = [&](const char *tag) {
		InteractionResult interaction;
		if (!runtime.executeActionTag(tag, interaction))
			return;

		runtime.applyImmediateDialogueInteractionEffects(interaction);
		runtime.queueDialogueInteractionIfNeeded(interaction);
	};

	const bool karinFoundAlive = runtime.startupScript().getFlagValue("KARIN_FOUND_ALIVE");
	if (karinFoundAlive && runtime.startupScript().getFlagValue("IN_CEM10") &&
			!state.cem10AliveLinePlayed) {
		state.cem10AliveLinePlayed = true;
		return playSequence(kKarinCem10AliveLines, ARRAYSIZE(kKarinCem10AliveLines));
	}

	if (usedItemName.empty()) {
		if (!karinFoundAlive && !state.notFoundAliveLinePlayed) {
			state.notFoundAliveLinePlayed = true;
			Common::Error lineError = playSequence(
				kKarinFirstNoItemLines, ARRAYSIZE(kKarinFirstNoItemLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		return playKarinLine(0x10dc);
	}

	const bool photoEvidence = usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
		usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY") ||
		usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL");
	if (photoEvidence) {
		if (!state.photoReplyOverrideFlag) {
			if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL"))
				sharedState.discussedWhaleyHerrillPhoto = 1;
			else
				sharedState.discussedCasketPhotoEvidence = 1;

			Common::Error lineError = playSequence(
				kKarinFirstPhotoReplyLines, ARRAYSIZE(kKarinFirstPhotoReplyLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			runtime.setActiveSpeakerPortrait(kKarinNpc, 0);
			state.photoReplyOverrideFlag = true;
			return Common::kNoError;
		}

		Common::Error lineError = playKarinLine(0x10e3, 4);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		(void)runtime.startupScript().setRuntimeFlagValue("DNALFT_PERVERT", true);
		executeActionTagIfSet("CALL_POLICE");
		return Common::kNoError;
	}

	if (usedItemName.equalsIgnoreCase("INV_MAG")) {
		Common::Error lineError = playSequence(kKarinInvMagLines, ARRAYSIZE(kKarinInvMagLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		(void)runtime.startupScript().setRuntimeFlagValue("DNALFT_PERVERT", true);
		executeActionTagIfSet("CALL_POLICE");
		return Common::kNoError;
	}

	return playKarinLine(0x10d6, 1);
}

} // End of namespace Harvester
