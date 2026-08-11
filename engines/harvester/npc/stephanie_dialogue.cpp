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


#include "harvester/npc/stephanie_dialogue.h"

#include "graphics/screen.h"
#include "harvester/harvester.h"
#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kDialogueC022AFstPath = "GRAPHIC/FST/C022A.FST";
static const char *const kDialogueC022BFstPath = "GRAPHIC/FST/C022B.FST";
static const char *const kDialogueC023AFstPath = "GRAPHIC/FST/C023A.FST";
static const char *const kDialogueC025AFstPath = "GRAPHIC/FST/C025A.FST";
static const char *const kDialogueC025BFstPath = "GRAPHIC/FST/C025B.FST";
static const char *const kDialogueC007FstPath = "GRAPHIC/FST/C007.FST";
static const int kStephanieSpyholeTopicLines[] = { 0x2f9, 0x2fa, 0x2fb, 0x2fc, 0x2fd };

} // End of namespace

bool StephanieDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("STEPHANIE");
}

Common::Error StephanieDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	StephanieRoomDialogueState &state = _state;
	Common::String &stephanieTopicBuffer = state.currentTopicBuffer;
	int &stephanieTopicBufferLineIndex = state.currentTopicBufferLineIndex;

	auto assignStephanieTopicBuffer = [&](int responseLineIndex) {
		runtime.assignTopicBuffer(stephanieTopicBuffer, stephanieTopicBufferLineIndex,
			responseLineIndex, "Stephanie topic buffer");
	};
	auto playPcLine = [&](int wavId, int headVariant = 0) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, "PC", headVariant);
	};
	auto playStephanieLine = [&](int wavId, const char *speakerId = "STEPHANIE") -> Common::Error {
		return runtime.playDialogueLine(wavId, speakerId);
	};
	auto playStephanieLineWithVariant = [&](int wavId, int headVariant) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, "STEPHANIE", headVariant);
	};
	auto executeDialogueActionTag = [&](const char *tag) {
		InteractionResult interaction;
		if (runtime.executeActionTag(tag, interaction)) {
			runtime.applyImmediateDialogueInteractionEffects(interaction);
			runtime.queueDialogueInteractionIfNeeded(interaction);
		}
	};
	auto clearScreenToBlack = [&]() {
		Graphics::Screen *screen = runtime.engine().getScreen();
		if (!screen)
			return;

		screen->fillRect(screen->getBounds(), 0);
		screen->update();
	};

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("NOTE") ||
				usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
			sharedState.discussedNoteCheckbookEvidence = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownEvidenceOfBlackmail, true);
			return playStephanieLineWithVariant(0x466a, 2);
		}
		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			sharedState.discussedCasketPhotoEvidence = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);
			Common::Error lineError = playStephanieLineWithVariant(0x4671, 4);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLine(0x4675, "PC");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			if (sharedState.dialogueStateD2ec0) {
				lineError = runtime.playDialogueLine(0x467b, "PC");
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
			if (sharedState.dialogueStateD2ebc) {
				lineError = runtime.playDialogueLine(0x467f, "PC");
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
			lineError = playStephanieLineWithVariant(0x4684, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			return runtime.playDialogueLine(0x4689, "PC");
		}
		if ((usedItemName.equalsIgnoreCase("LEDGER") ||
				usedItemName.equalsIgnoreCase("LEDGER2")) &&
				runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) {
			sharedState.discussedLedgerEvidence = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownLedgersToAnyone, true);
			const DialogueLineEntry lines[] = {
				{ 0x468f, "PC", 0 },
				{ 0x4694, "STEPHANIE", 0 },
				{ 0x469a, "PC", 0 },
				{ 0x469e, "STEPHANIE", 2 }
			};
			return runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
		}

		return playStephanieLine(0x4662);
	}

	if (state.introPending) {
		state.introPending = false;
		state.introDayIndex = runtime.startupScript().getCurrentStoryDayIndex();
		const DialogueLineEntry introLines[] = {
			{ 0x43db, "STEPHANIE", 2 },
			{ 0x43df, "PC", 0 },
			{ 0x43e3, "STEPHANIE", 0 },
			{ 0x43e7, "PC", 0 },
			{ 0x43eb, "STEPHANIE", 2 },
			{ 0x43f0, "PC", 0 },
			{ 0x43f5, "STEPHANIE", 3 },
			{ 0x43f9, "PC", 0 },
			{ 0x43fd, "STEPHANIE", 0 }
		};
		Common::Error lineError = runtime.playDialogueEntrySequence(introLines, ARRAYSIZE(introLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		assignStephanieTopicBuffer(0x2cf);
	} else if (runtime.startupScript().getCurrentStoryDayIndex() == state.introDayIndex) {
		Common::Error lineError = playStephanieLine(0x459c);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		assignStephanieTopicBuffer(0x2d0);
	} else if (state.firstFollowupPending) {
		state.firstFollowupPending = false;
		state.firstFollowupDayIndex = runtime.startupScript().getCurrentStoryDayIndex();

		Common::Error lineError = playStephanieLineWithVariant(0x45a7, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (sharedState.dialogueStateD2f08) {
			lineError = runtime.playDialogueLine(0x45ae, "PC");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		lineError = runtime.playDialogueLine(
			(runtime.startupScript().getFlagValue("HAVE_LODGE_APP") &&
				runtime.startupScript().getCurrentStoryDayIndex() == 1) ? 0x45b4 : 0x45b9,
			"PC");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = playStephanieLineWithVariant(0x45be, 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x2d1, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		const DialogueLineEntry followupLines[] = {
			{ 0x45d8, "STEPHANIE", 2 },
			{ 0x45de, "PC", 0 },
			{ 0x45e2, "STEPHANIE", 3 }
		};
		Common::Error lineSeqError = runtime.playDialogueEntrySequence(
			followupLines, ARRAYSIZE(followupLines));
		if (lineSeqError.getCode() != Common::kNoError)
			return lineSeqError;
	} else if (runtime.startupScript().getCurrentStoryDayIndex() == state.firstFollowupDayIndex) {
		Common::Error lineError = playStephanieLine(0x4654);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		state.secondFollowupPending = true;
	} else if (state.secondFollowupPending) {
		state.secondFollowupPending = false;
		state.secondFollowupDayIndex = runtime.startupScript().getCurrentStoryDayIndex();

		const DialogueLineEntry nastyIntroLines[] = {
			{ 0x45e8, "STEPHANIE", 1 },
			{ 0x45ed, "PC", 0 },
			{ 0x45f2, "STEPHANIE", 0 }
		};
		Common::Error lineError = runtime.playDialogueEntrySequence(
			nastyIntroLines, ARRAYSIZE(nastyIntroLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueFst(kDialogueC023AFstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueLine(0x45fe, "PC");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = playStephanieLine(0x4602);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x2d3, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;
		if (responseIndex == 1) {
			lineError = playStephanieLineWithVariant(0x460e, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (responseIndex == 2) {
			lineError = playStephanieLineWithVariant(0x4612, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		const DialogueLineEntry nastyFollowupLines[] = {
			{ 0x4616, "STEPHANIE", 3 },
			{ 0x461b, "PC", 3 },
			{ 0x461f, "STEPHANIE", 0 },
			{ 0x4625, "PC", 0 },
			{ 0x4629, "STEPHANIE", 0 }
		};
		lineError = runtime.playDialogueEntrySequence(nastyFollowupLines, ARRAYSIZE(nastyFollowupLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueFst(kDialogueC025AFstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		responseIndex = 0;
		responseError = runtime.runResponseMenu(0x2d5, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;
		if (responseIndex == 1) {
			clearScreenToBlack();
			(void)runtime.playDialogueFst(kDialogueC025BFstPath);
			(void)runtime.playDialogueFst(kDialogueC007FstPath);
			executeDialogueActionTag("LEAVE_STEPH_AFTER_NASTY");
			return Common::kNoError;
		}
		if (responseIndex == 2) {
			lineError = playPcLine(0x4649, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playStephanieLineWithVariant(0x464e, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	} else {
		Common::Error lineError = playStephanieLine(0x4654);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getCurrentStoryDayIndex() == 5 && !state.dayFiveLinePlayed) {
		state.dayFiveLinePlayed = true;
		const DialogueLineEntry dayFiveLines[] = {
			{ 0x46aa, "STEPHANIE", 0 },
			{ 0x46ae, "PC", 0 },
			{ 0x46b3, "STEPHANIE", 2 },
			{ 0x46bf, "STEPHANIE", 0 },
			{ 0x46ba, "PC", 0 },
			{ 0x46c5, "PC", 0 },
			{ 0x46c9, "STEPHANIE", 3 }
		};
		Common::Error lineError = runtime.playDialogueEntrySequence(dayFiveLines, ARRAYSIZE(dayFiveLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("BURNED_TV_STATION") &&
			!state.burnedTvStationShown) {
		state.burnedTvStationShown = true;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x4918, "STEPHANIE", 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		const DialogueLineEntry openingLines[] = {
			{ 0x491e, "PC", 0 },
			{ 0x4928, "STEPHANIE", 3 },
			{ 0x492c, "PC", 3 },
			{ 0x4930, "STEPHANIE", 3 }
		};
		lineError = runtime.playDialogueEntrySequence(openingLines, ARRAYSIZE(openingLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x2d7, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = playStephanieLineWithVariant(0x493a, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			clearScreenToBlack();
			(void)runtime.playDialogueFst(kDialogueC007FstPath);
			executeDialogueActionTag("LEAVE_STEPH_AFTER_NASTY");
			return Common::kNoError;
		}
		if (responseIndex == 2) {
			const DialogueLineEntry responseLines[] = {
				{ 0x4944, "STEPHANIE", 3 },
				{ 0x4949, "PC", 3 },
				{ 0x494d, "STEPHANIE", 3 }
			};
			lineError = runtime.playDialogueEntrySequence(responseLines, ARRAYSIZE(responseLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	}
	if (runtime.startupScript().getFlagValue("SCRATCHED_TUCKER") &&
			!state.scratchedTuckerShown) {
		state.scratchedTuckerShown = true;
		const DialogueLineEntry openingLines[] = {
			{ 0x46d2, "STEPHANIE", 0 },
			{ 0x46db, "PC", 0 },
			{ 0x46e0, "STEPHANIE", 0 }
		};
		Common::Error lineError = runtime.playDialogueEntrySequence(openingLines, ARRAYSIZE(openingLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x2d9, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			const DialogueLineEntry responseOneLines[] = {
				{ 0x46ea, "STEPHANIE", 2 },
				{ 0x46ee, "PC", 0 },
				{ 0x46f2, "STEPHANIE", 2 },
				{ 0x46f6, "PC", 0 },
				{ 0x46fb, "STEPHANIE", 2 }
			};
			lineError = runtime.playDialogueEntrySequence(responseOneLines, ARRAYSIZE(responseOneLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int followupResponseIndex = 0;
			responseError = runtime.runResponseMenu(0x2da, followupResponseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (followupResponseIndex == 1) {
				const DialogueLineEntry followupLines[] = {
					{ 0x4705, "STEPHANIE", 2 },
					{ 0x4709, "PC", 2 }
				};
				lineError = runtime.playDialogueEntrySequence(followupLines, ARRAYSIZE(followupLines));
			} else if (followupResponseIndex == 2) {
				const DialogueLineEntry followupLines[] = {
					{ 0x470e, "STEPHANIE", 2 },
					{ 0x470f, "PC", 0 }
				};
				lineError = runtime.playDialogueEntrySequence(followupLines, ARRAYSIZE(followupLines));
			} else {
				lineError = playStephanieLineWithVariant(0x4717, 3);
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (responseIndex == 2) {
			lineError = playStephanieLine(0x471e);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	}
	if (runtime.startupScript().getFlagValue("BOLT_OF_CLOTH_TAKEN") &&
			!state.boltOfClothTakenShown) {
		state.boltOfClothTakenShown = true;
		Common::Error lineError = playStephanieLine(0x4724);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x2db, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = playStephanieLine(0x472f);
		} else if (responseIndex == 2) {
			state.boltOfClothEscalationTriggered = true;
			const DialogueLineEntry responseTwoLines[] = {
				{ 0x4736, "STEPHANIE", 2 },
				{ 0x473b, "PC", 0 },
				{ 0x4740, "STEPHANIE", 2 },
				{ 0x4744, "PC", 0 },
				{ 0x4748, "STEPHANIE", 2 },
				{ 0x474d, "PC", 4 },
				{ 0x4753, "STEPHANIE", 0 }
			};
			lineError = runtime.playDialogueEntrySequence(responseTwoLines, ARRAYSIZE(responseTwoLines));
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("BARBER_POLE_STOLEN") &&
			!state.barberPoleStolenShown) {
		state.barberPoleStolenShown = true;
		Common::Error lineError = playStephanieLine(0x47a0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x2dd, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			const DialogueLineEntry responseOneLines[] = {
				{ 0x47af, "STEPHANIE", 0 },
				{ 0x47b4, "PC", 0 },
				{ 0x47b8, "STEPHANIE", 0 }
			};
			lineError = runtime.playDialogueEntrySequence(responseOneLines, ARRAYSIZE(responseOneLines));
		} else if (responseIndex == 2) {
			const DialogueLineEntry responseTwoLines[] = {
				{ 0x47be, "STEPHANIE", 2 },
				{ 0x47c2, "PC", 3 },
				{ 0x47c7, "STEPHANIE", 4 },
				{ 0x47cc, "PC", 2 },
				{ 0x47d0, "STEPHANIE", 2 },
				{ 0x47d6, "PC", 2 },
				{ 0x47da, "STEPHANIE", 0 },
				{ 0x47de, "PC", 2 },
				{ 0x47e3, "STEPHANIE", 0 },
				{ 0x47eb, "PC", 0 },
				{ 0x47ef, "STEPHANIE", 2 },
				{ 0x482b, "STEPHANIE", 2 },
				{ 0x4830, "PC", 4 },
				{ 0x4834, "STEPHANIE", 2 },
				{ 0x4839, "PC", 0 },
				{ 0x483d, "STEPHANIE", 0 },
				{ 0x47fe, "PC", 0 }
			};
			lineError = runtime.playDialogueEntrySequence(responseTwoLines, ARRAYSIZE(responseTwoLines));
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("DINER_BURNED") &&
			!state.dinerBurnedShown) {
		state.dinerBurnedShown = true;
		const DialogueLineEntry openingLines[] = {
			{ 0x4845, "STEPHANIE", 3 },
			{ 0x484a, "PC", 0 },
			{ 0x484e, "STEPHANIE", 0 }
		};
		Common::Error lineError = runtime.playDialogueEntrySequence(openingLines, ARRAYSIZE(openingLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x2df, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = playStephanieLineWithVariant(0x4858, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			if (runtime.startupScript().getFlagValue("KARIN_FOUND_ALIVE")) {
				lineError = playStephanieLineWithVariant(0x485c, 3);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}

			int followupResponseIndex = 0;
			responseError = runtime.runResponseMenu(0x2e0, followupResponseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (followupResponseIndex == 1) {
				const DialogueLineEntry followupLines[] = {
					{ 0x4866, "STEPHANIE", 2 },
					{ 0x486b, "PC", 4 },
					{ 0x4870, "STEPHANIE", 2 }
				};
				lineError = runtime.playDialogueEntrySequence(followupLines, ARRAYSIZE(followupLines));
			} else if (followupResponseIndex == 2) {
				const DialogueLineEntry responseTwoLines[] = {
					{ 0x4877, "STEPHANIE", 3 },
					{ 0x487b, "PC", 0 },
					{ 0x4880, "STEPHANIE", 0 }
				};
				lineError = runtime.playDialogueEntrySequence(responseTwoLines, ARRAYSIZE(responseTwoLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				int nestedResponseIndex = 0;
				responseError = runtime.runResponseMenu(0x2e1, nestedResponseIndex);
				if (responseError.getCode() != Common::kNoError)
					return responseError;

				if (nestedResponseIndex == 1) {
					lineError = playStephanieLineWithVariant(0x488e, 2);
				} else if (nestedResponseIndex == 2) {
					const DialogueLineEntry nestedLines[] = {
						{ 0x4895, "STEPHANIE", 1 },
						{ 0x4899, "PC", 3 },
						{ 0x489d, "STEPHANIE", 3 },
						{ 0x461b, "PC", 3 }
					};
					lineError = runtime.playDialogueEntrySequence(nestedLines, ARRAYSIZE(nestedLines));
				}
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (responseIndex == 2) {
			const DialogueLineEntry responseTwoLines[] = {
				{ 0x48c2, "STEPHANIE", 0 },
				{ 0x48c6, "PC", 0 },
				{ 0x48ca, "STEPHANIE", 0 }
			};
			lineError = runtime.playDialogueEntrySequence(responseTwoLines, ARRAYSIZE(responseTwoLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			if (runtime.startupScript().getFlagValue("KARIN_FOUND_ALIVE") &&
					!state.dinerBurnedKarinAliveFollowupShown) {
				state.dinerBurnedKarinAliveFollowupShown = true;
				const DialogueLineEntry karinAliveLines[] = {
					{ 0x48ce, "STEPHANIE", 3 },
					{ 0x48d3, "STEPHANIE", 3 },
					{ 0x48d8, "PC", 0 },
					{ 0x48dc, "STEPHANIE", 0 },
					{ 0x48e1, "PC", 0 },
					{ 0x48e5, "STEPHANIE", 0 }
				};
				lineError = runtime.playDialogueEntrySequence(karinAliveLines, ARRAYSIZE(karinAliveLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
		}
	}
	if (runtime.startupScript().getFlagValue("PC_ESCAPED_JAIL") &&
			!state.pcEscapedJailShown) {
		state.pcEscapedJailShown = true;
		Common::Error lineError = playStephanieLineWithVariant(0x48eb, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x2e3, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			const DialogueLineEntry responseLines[] = {
				{ 0x48f7, "STEPHANIE", 3 },
				{ 0x4908, "PC", 0 },
				{ 0x490d, "STEPHANIE", 0 },
				{ 0x4912, "PC", 0 }
			};
			lineError = runtime.playDialogueEntrySequence(responseLines, ARRAYSIZE(responseLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	}
	bool ranKarinKidnapedBranch = false;
	if (runtime.startupScript().getFlagValue("KARIN_KIDNAPED") &&
			!state.karinKidnapedLinePlayed &&
			sharedState.karinKidnapedDialogueState) {
		state.karinKidnapedLinePlayed = true;
		ranKarinKidnapedBranch = true;
		const DialogueLineEntry kidnapedLines[] = {
			{ 0x4957, "STEPHANIE", 0 },
			{ 0x495b, "PC", 0 },
			{ 0x4960, "STEPHANIE", 0 },
			{ 0x4965, "PC", 0 },
			{ 0x4969, "STEPHANIE", 0 }
		};
		Common::Error lineError = runtime.playDialogueEntrySequence(kidnapedLines, ARRAYSIZE(kidnapedLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	// Native only reaches the kidnap tail from the post-kidnap continuation, but 0x4975
	// always fires once that branch runs; the spyhole flag only prepends 0x4970.
	if (ranKarinKidnapedBranch) {
		Common::Error lineError = Common::kNoError;
		if (state.playedSpyholeBranch) {
			lineError = runtime.playDialogueLine(0x4970, "STEPHANIE");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		lineError = runtime.playDialogueLine(0x4975, "STEPHANIE");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (sharedState.discussedMrPottsTuesdayNightAlibi) {
			lineError = runtime.playDialogueLineWithVariant(0x4980, "PC", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (sharedState.discussedMrsPottsTuesdayNightAlibi) {
			const DialogueLineEntry lines[] = {
				{ 0x4984, "PC", 0 },
				{ 0x4989, "STEPHANIE", 0 },
				{ 0x498f, "STEPHANIE", 0 }
			};
			lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	}
	if ((runtime.startupScript().getFlagValue("KARIN_FOUND_ALIVE") ||
				runtime.startupScript().getFlagValue("KARIN_FOUND_DEAD")) &&
			!state.karinOutcomeLinePlayed) {
		state.karinOutcomeLinePlayed = true;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x4996, "STEPHANIE", 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (runtime.startupScript().getFlagValue("KARIN_FOUND_ALIVE")) {
			const DialogueLineEntry aliveLines[] = {
				{ 0x499a, "PC", 2 },
				{ 0x49a1, "STEPHANIE", 2 },
				{ 0x49ad, "PC", 0 },
				{ 0x49b1, "STEPHANIE", 4 },
				{ 0x49b6, "PC", 0 },
				{ 0x49ba, "STEPHANIE", 0 }
			};
			lineError = runtime.playDialogueEntrySequence(aliveLines, ARRAYSIZE(aliveLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			if (sharedState.dwayneCompletedKarinAliveFollowup) {
				lineError = playStephanieLine(0x49c1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				const DialogueLineEntry outcomeTailLines[] = {
					{ 0x49ea, "STEPHANIE", 2 },
					{ 0x49ef, "PC", 2 }
				};
				return runtime.playDialogueEntrySequence(outcomeTailLines, ARRAYSIZE(outcomeTailLines));
			}
		} else {
			const DialogueLineEntry deadLines[] = {
				{ 0x49da, "PC", 3 },
				{ 0x49e0, "STEPHANIE", 3 },
				{ 0x49e4, "PC", 3 }
			};
			lineError = runtime.playDialogueEntrySequence(deadLines, ARRAYSIZE(deadLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			const DialogueLineEntry outcomeTailLines[] = {
				{ 0x49ea, "STEPHANIE", 2 },
				{ 0x49ef, "PC", 2 }
			};
			return runtime.playDialogueEntrySequence(outcomeTailLines, ARRAYSIZE(outcomeTailLines));
		}
	}

	if (stephanieTopicBufferLineIndex < 0)
		assignStephanieTopicBuffer(0x2d0);

	for (;;) {
		Common::String selectedTopic;
		Common::Error menuError = runtime.runKeywordMenu(
			stephanieTopicBuffer, stephanieTopicBufferLineIndex, selectedTopic);
		if (menuError.getCode() != Common::kNoError)
			return menuError;
		if (selectedTopic.empty())
			return Common::kNoError;
		if (runtime.lastKeywordSelectionWasGenericBye())
			return playStephanieLine(0x46a4);
		if (runtime.matchesResponseLine(selectedTopic, 0x2e5) ||
				runtime.matchesResponseLine(selectedTopic, 0x2e6)) {
			Common::Error lineError = playStephanieLine(0x4406);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x2e7, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1) {
				const DialogueLineEntry lines[] = {
					{ 0x4412, "STEPHANIE", 0 },
					{ 0x4416, "PC", 0 },
					{ 0x441a, "STEPHANIE", 0 }
				};
				lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				responseIndex = 0;
				responseError = runtime.runResponseMenu(0x2e8, responseIndex);
				if (responseError.getCode() != Common::kNoError)
					return responseError;
				if (responseIndex == 1)
					lineError = playStephanieLine(0x4427);
				else if (responseIndex == 2)
					lineError = playStephanieLine(0x442c);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueLine(0x4430, "PC");
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
			if (responseIndex == 2) {
				lineError = playStephanieLine(0x4434);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueLine(0x4439, "PC");
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = playStephanieLine(0x443d);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
			lineError = playStephanieLine(0x4441);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignStephanieTopicBuffer(0x2e9);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x2ea)) {
			Common::Error lineError = playStephanieLine(0x444b);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLine(0x4451, "PC");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playStephanieLine(0x4455);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x2eb, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1) {
				const DialogueLineEntry lines[] = {
					{ 0x4463, "STEPHANIE", 0 },
					{ 0x446b, "PC", 0 },
					{ 0x446f, "STEPHANIE", 0 },
					{ 0x4477, "PC", 0 },
					{ 0x447b, "STEPHANIE", 0 }
				};
				lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else if (responseIndex == 2) {
				lineError = playStephanieLine(0x4480);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
			assignStephanieTopicBuffer(0x2ec);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x2ed)) {
			Common::Error lineError = playStephanieLine(0x448a);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x2ee, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1) {
				lineError = playStephanieLineWithVariant(0x4495, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else if (responseIndex == 2) {
				lineError = playStephanieLine(0x4499);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueLine(0x449e, "PC");
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
			const DialogueLineEntry lines[] = {
				{ 0x44a2, "PC", 0 },
				{ 0x44a6, "STEPHANIE", 0 },
				{ 0x44ab, "PC", 0 },
				{ 0x44af, "STEPHANIE", 0 }
			};
			lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignStephanieTopicBuffer(0x2ef);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x2f0)) {
			const DialogueLineEntry lines[] = {
				{ 0x44ba, "STEPHANIE", 0 },
				{ 0x44bf, "PC", 0 },
				{ 0x44c4, "STEPHANIE", 1 },
				{ 0x44c9, "PC", 0 },
				{ 0x44ce, "STEPHANIE", 0 },
				{ 0x44dd, "PC", 0 },
				{ 0x44e9, "STEPHANIE", 0 }
			};
			Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignStephanieTopicBuffer(0x2f2);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x2f3)) {
			Common::Error lineError = playStephanieLineWithVariant(0x44f1, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC022AFstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLine(
				sharedState.discussedLodgeTopic ? 0x4505 : 0x450a, "PC");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			if (sharedState.dialogueStateD2f00) {
				lineError = runtime.playDialogueLine(0x450f, "PC");
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
			const DialogueLineEntry lines[] = {
				{ 0x4513, "STEPHANIE", 2 },
				{ 0x4517, "PC", 0 },
				{ 0x451b, "STEPHANIE", 2 },
				{ 0x451f, "PC", 0 },
				{ 0x4523, "STEPHANIE", 0 },
				{ 0x452a, "PC", 0 },
				{ 0x452e, "STEPHANIE", 0 }
			};
			lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC022BFstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x2f6)) {
			Common::Error lineError = runtime.playDialogueLine(0x453d, "PC");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playStephanieLineWithVariant(0x4541, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x2f7, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1) {
				lineError = playStephanieLineWithVariant(0x454b, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else if (responseIndex == 2) {
				lineError = playStephanieLineWithVariant(0x454f, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
			assignStephanieTopicBuffer(0x2f8);
			continue;
		}
		// The spyhole continuation is a hidden typed-topic branch. Native only reaches
		// it after the visible buffer has advanced to dialog.rsp[0x2f8], while the
		// generic BYE path remains the immediate 0x46a4 exit.
		if (stephanieTopicBufferLineIndex == 0x2f8 &&
				runtime.lastKeywordSelectionWasTypedInput() &&
				runtime.matchesAnyResponseLine(selectedTopic, kStephanieSpyholeTopicLines,
					ARRAYSIZE(kStephanieSpyholeTopicLines))) {
			state.playedSpyholeBranch = true;
			Common::Error lineError = runtime.playDialogueLine(0x4a05, "PC");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0x4a0a, "STEPHANIE", 4);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLine(0x4a0e, "PC");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0x4a12, "STEPHANIE", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLine(
				sharedState.confrontedMrPottsAboutSpyhole ? 0x4a19 : 0x4a1e, "PC");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0x4a23, "STEPHANIE", 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x2e4)) {
			return playStephanieLine(0x46a4);
		}

		Common::Error lineError = playStephanieLine(0x465c);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
}

} // End of namespace Harvester
