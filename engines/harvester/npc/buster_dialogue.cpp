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


#include "graphics/screen.h"
#include "harvester/harvester.h"

#include "harvester/npc/buster_dialogue.h"

#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kDialogueC058FstPath = "GRAPHIC/FST/C058.FST";
static const char *const kDialogueC085FstPath = "GRAPHIC/FST/C085.FST";
static const char *const kDialogueC086FstPath = "GRAPHIC/FST/C086.FST";

static const int kBusterEvidenceResponseLine = 0x23;
static const int kBusterIntroTopicBufferLine = 0x24;
static const int kBusterSecondIntroResponseLine = 0x25;
static const int kBusterSecondIntroFollowupResponseLine = 0x26;
static const int kBusterExitTopicLine = 0x27;
static const int kBusterTopic0x28Or0x29Lines[] = { 0x28, 0x29 };
static const int kBusterTopic0x2cOr0x2dLines[] = { 0x2c, 0x2d };
static const int kBusterTopic0x2cResponseLine = 0x2e;
static const int kBusterTopic0x2cFollowupResponseLine = 0x2f;
static const int kBusterTopic0x2cNestedResponseLine = 0x30;
static const int kBusterTopic0x32Line = 0x32;
static const int kBusterTopic0x32TopicBufferLine = 0x33;
static const int kBusterTopic0x34Line = 0x34;
static const int kBusterTopic0x34TopicBufferLine = 0x35;
static const int kBusterTopic0x36Line = 0x36;
static const int kBusterTopic0x36TopicBufferLine = 0x37;
static const int kBusterTopic0x38Line = 0x38;
static const int kBusterTopic0x38ResponseLine = 0x39;
static const int kBusterTopic0x38TopicBufferLine = 0x3a;
static const int kBusterTopic0x3bLine = 0x3b;
static const int kBusterTopic0x3bResponseLine = 0x3c;
static const int kBusterTopic0x3bTopicBufferLine = 0x3d;
static const int kBusterTopic0x3eLine = 0x3e;
static const int kBusterTopic0x3eTopicBufferLine = 0x3f;
static const int kBusterTopic0x40Or0x41Lines[] = { 0x40, 0x41 };
static const int kBusterTopic0x40ResponseLine = 0x42;
static const int kBusterTopic0x40TopicBufferLine = 0x43;
static const int kBusterTopic0x44Or0x45Or0x46Lines[] = { 0x44, 0x45, 0x46 };
static const int kBusterTopic0x44TopicBufferLine = 0x47;
static const int kBusterTopic0x48Line = 0x48;
static const int kBusterTopic0x49Line = 0x49;

static const DialogueLineEntry kBusterCorpsePhotoLines[] = {
	{ 0x1884, "BUSTER", 0 },
	{ 0x1889, "PC", 4 },
	{ 0x188e, "BUSTER", 2 },
	{ 0x1895, "BUSTER", 2 }
};

static const DialogueLineEntry kBusterIntroLines[] = {
	{ 0x163c, "BUSTER", 2 },
	{ 0x1640, "PC", 0 },
	{ 0x1644, "BUSTER", 0 },
	{ 0x1649, "PC", 0 },
	{ 0x164d, "BUSTER", 0 },
	{ 0x1654, "PC", 0 },
	{ 0x1658, "BUSTER", 0 },
	{ 0x165e, "PC", 4 },
	{ 0x1662, "BUSTER", 0 },
	{ 0x1669, "PC", 0 },
	{ 0x166e, "BUSTER", 0 },
	{ 0x1673, "PC", 0 },
	{ 0x1677, "BUSTER", 0 },
	{ 0x167e, "BUSTER", 0 },
	{ 0x1682, "BUSTER", 2 },
	{ 0x168e, "BUSTER", 0 }
};

static const DialogueLineEntry kBusterSecondIntroPreludeLines[] = {
	{ 0x1828, "BUSTER", 2 },
	{ 0x182c, "PC", 2 },
	{ 0x1830, "BUSTER", 2 }
};

static const DialogueLineEntry kBusterThirdIntroLines[] = {
	{ 0x18c5, "BUSTER", 2 },
	{ 0x18c9, "PC", 4 },
	{ 0x18cd, "BUSTER", 2 }
};

static const DialogueLineEntry kBusterTopic0x28Or0x29LinesSequence[] = {
	{ 0x1698, "BUSTER", 0 },
	{ 0x169c, "PC", 4 },
	{ 0x16a1, "BUSTER", 0 }
};

static const DialogueLineEntry kBusterTopic0x28Or0x29TailLines[] = {
	{ 0x16ad, "BUSTER", 2 },
	{ 0x16b6, "BUSTER", 0 }
};

static const DialogueLineEntry kBusterTopic0x32Lines[] = {
	{ 0x170a, "PC", 0 },
	{ 0x170f, "BUSTER", 0 },
	{ 0x1710, "BUSTER", 0 },
	{ 0x1711, "BUSTER", 0 },
	{ 0x1712, "BUSTER", 0 }
};

static const DialogueLineEntry kBusterTopic0x34Lines[] = {
	{ 0x1727, "BUSTER", 0 },
	{ 0x1728, "BUSTER", 0 },
	{ 0x1729, "BUSTER", 0 },
	{ 0x172a, "BUSTER", 0 },
	{ 0x172b, "BUSTER", 0 }
};

static const DialogueLineEntry kBusterTopic0x36Lines[] = {
	{ 0x173d, "PC", 0 },
	{ 0x1742, "BUSTER", 0 },
	{ 0x1749, "PC", 0 },
	{ 0x174d, "BUSTER", 0 },
	{ 0x174e, "BUSTER", 0 },
	{ 0x174f, "BUSTER", 0 },
	{ 0x1750, "BUSTER", 2 },
	{ 0x1762, "BUSTER", 2 }
};

static const DialogueLineEntry kBusterTopic0x38ResponseTwoLines[] = {
	{ 0x1787, "BUSTER", 0 },
	{ 0x1791, "BUSTER", 0 },
	{ 0x1796, "PC", 0 },
	{ 0x179d, "BUSTER", 0 }
};

static const DialogueLineEntry kBusterTopic0x3bTailLines[] = {
	{ 0x17bb, "BUSTER", 0 },
	{ 0x17c3, "BUSTER", 0 }
};

static const DialogueLineEntry kBusterTopic0x3eLines[] = {
	{ 0x17cc, "BUSTER", 0 },
	{ 0x17cd, "BUSTER", 0 },
	{ 0x17ce, "BUSTER", 0 },
	{ 0x17db, "BUSTER", 0 }
};

static const DialogueLineEntry kBusterTopic0x40Or0x41PreludeLines[] = {
	{ 0x17e4, "BUSTER", 0 },
	{ 0x17e5, "BUSTER", 0 },
	{ 0x17e6, "BUSTER", 0 },
	{ 0x17e7, "BUSTER", 0 },
	{ 0x17f7, "BUSTER", 0 }
};

static Common::Error playFatalBusterBranch(DialogueRuntime &runtime, int wavId, int headVariant = 0) {
	Common::Error lineError = runtime.playDialogueLineWithVariant(wavId, "BUSTER", headVariant);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	Graphics::Screen *screen = runtime.engine().getScreen();
	if (screen) {
		screen->fillRect(screen->getBounds(), 0);
		screen->update();
	}
	lineError = runtime.playDialogueFst(kDialogueC086FstPath);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	return runtime.runGameOverScreen();
}

} // End of namespace

Common::Error BusterDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	Common::String topicBuffer;
	int topicBufferLineIndex = -1;

	DialogueTopicBuffer topics(runtime, topicBuffer, topicBufferLineIndex, "Buster topic buffer");

	auto isCorpsePhoto = [&](const Common::String &itemName) {
		return itemName.equalsIgnoreCase("CASKET_PHOTO") ||
			itemName.equalsIgnoreCase("CASKET_PHOTOCOPY");
	};
	auto isBlackmailEvidence = [&](const Common::String &itemName) {
		return itemName.equalsIgnoreCase("NOTE_CASKET_PHOTO") ||
			itemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
			itemName.equalsIgnoreCase("CHECKBOOK") ||
			itemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY");
	};

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			sharedState.discussedWhaleyHerrillPhoto = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
			return runtime.playDialogueLineWithVariant(0x187b, "BUSTER", 0);
		}
		if (isCorpsePhoto(usedItemName)) {
			sharedState.discussedCasketPhotoEvidence = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);
			return runtime.playDialogueEntrySequence(kBusterCorpsePhotoLines, ARRAYSIZE(kBusterCorpsePhotoLines));
		}
		if (isBlackmailEvidence(usedItemName)) {
			sharedState.discussedNoteCheckbookEvidence = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownEvidenceOfBlackmail, true);
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x189b, "BUSTER", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(
				kBusterEvidenceResponseLine, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = runtime.playDialogueLineWithVariant(0x18a6, "BUSTER", 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueLineWithVariant(0x18aa, "PC", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				return runtime.playDialogueLineWithVariant(0x18ae, "BUSTER", 0);
			}
			if (responseIndex == 2) {
				lineError = runtime.playDialogueLineWithVariant(0x18b5, "BUSTER", 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				return runtime.playDialogueLineWithVariant(0x18be, "BUSTER", 2);
			}
			return Common::kNoError;
		}
		return runtime.playDialogueLineWithVariant(0x1875, "BUSTER", 0);
	}

	if (_state.introPending) {
		_state.introPending = false;
		_state.secondIntroPending = true;
		Common::Error lineError = runtime.playDialogueEntrySequence(kBusterIntroLines, ARRAYSIZE(kBusterIntroLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		topics.assign(kBusterIntroTopicBufferLine);
	} else if (_state.secondIntroPending) {
		_state.secondIntroPending = false;
		_state.thirdIntroPending = true;
		Common::Error lineError = runtime.playDialogueEntrySequence(kBusterSecondIntroPreludeLines,
			ARRAYSIZE(kBusterSecondIntroPreludeLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(
			kBusterSecondIntroResponseLine, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1)
			return playFatalBusterBranch(runtime, 0x183a, 2);
		if (responseIndex == 2) {
			lineError = runtime.playDialogueLineWithVariant(0x1841, "BUSTER", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			if (!_state.secondIntroResponseTwoSeen) {
				lineError = runtime.playDialogueLineWithVariant(0x185e, "BUSTER", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				_state.secondIntroResponseTwoSeen = true;
				_state.secondIntroPending = true;
				_state.thirdIntroPending = false;
			} else {
				lineError = runtime.playDialogueLineWithVariant(0x1845, "BUSTER", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				int followupResponseIndex = 0;
				responseError = runtime.runResponseMenu(
					kBusterSecondIntroFollowupResponseLine, followupResponseIndex);
				if (responseError.getCode() != Common::kNoError)
					return responseError;

				if (followupResponseIndex == 1)
					return playFatalBusterBranch(runtime, 0x1850);
				if (followupResponseIndex == 2) {
					lineError = runtime.playDialogueLineWithVariant(0x1858, "BUSTER", 0);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
				}
			}
		}
	} else if (_state.thirdIntroPending) {
		_state.thirdIntroPending = false;
		return runtime.playDialogueEntrySequence(kBusterThirdIntroLines, ARRAYSIZE(kBusterThirdIntroLines));
	} else {
		return runtime.playDialogueLineWithVariant(0x1868, "BUSTER", 0);
	}

	for (;;) {
		Common::String selectedTopic;
		Common::Error menuError = runtime.runKeywordMenu(topicBuffer, topicBufferLineIndex, selectedTopic);
		if (menuError.getCode() != Common::kNoError)
			return menuError;
		if (selectedTopic.empty())
			return Common::kNoError;

		if (runtime.matchesResponseLine(selectedTopic, kBusterExitTopicLine))
			return runtime.playDialogueLineWithVariant(0x1818, "BUSTER", 0);

		if (runtime.matchesAnyResponseLine(selectedTopic, kBusterTopic0x28Or0x29Lines,
				ARRAYSIZE(kBusterTopic0x28Or0x29Lines))) {
			Common::Error lineError = runtime.playDialogueEntrySequence(kBusterTopic0x28Or0x29LinesSequence,
				ARRAYSIZE(kBusterTopic0x28Or0x29LinesSequence));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC085FstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueEntrySequence(kBusterTopic0x28Or0x29TailLines,
				ARRAYSIZE(kBusterTopic0x28Or0x29TailLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			topics.assign(0x2b);
			continue;
		}

		if (runtime.matchesAnyResponseLine(selectedTopic, kBusterTopic0x2cOr0x2dLines,
				ARRAYSIZE(kBusterTopic0x2cOr0x2dLines))) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x16be, "BUSTER", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0x16c5, "BUSTER", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(
				kBusterTopic0x2cResponseLine, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1)
				return playFatalBusterBranch(runtime, 0x16d0);
			if (responseIndex == 2) {
				lineError = runtime.playDialogueLineWithVariant(0x16d6, "BUSTER", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				int followupResponseIndex = 0;
				responseError = runtime.runResponseMenu(
					kBusterTopic0x2cFollowupResponseLine, followupResponseIndex);
				if (responseError.getCode() != Common::kNoError)
					return responseError;

				if (followupResponseIndex == 1) {
					lineError = runtime.playDialogueLineWithVariant(0x16e2, "BUSTER", 0);
					if (lineError.getCode() != Common::kNoError)
						return lineError;

					int nestedResponseIndex = 0;
					responseError = runtime.runResponseMenu(
						kBusterTopic0x2cNestedResponseLine, nestedResponseIndex);
					if (responseError.getCode() != Common::kNoError)
						return responseError;

					if (nestedResponseIndex == 1)
						return playFatalBusterBranch(runtime, 0x16ef);
					if (nestedResponseIndex == 2) {
						lineError = runtime.playDialogueLineWithVariant(0x16f6, "BUSTER", 0);
						if (lineError.getCode() != Common::kNoError)
							return lineError;

						topics.assign(0x31);
					}
				} else if (followupResponseIndex == 2) {
					return playFatalBusterBranch(runtime, 0x1702);
				}
			}
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, kBusterTopic0x32Line)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(kBusterTopic0x32Lines, ARRAYSIZE(kBusterTopic0x32Lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			topics.assign(kBusterTopic0x32TopicBufferLine);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, kBusterTopic0x34Line)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(kBusterTopic0x34Lines, ARRAYSIZE(kBusterTopic0x34Lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			topics.assign(kBusterTopic0x34TopicBufferLine);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, kBusterTopic0x36Line)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(kBusterTopic0x36Lines, ARRAYSIZE(kBusterTopic0x36Lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			topics.assign(kBusterTopic0x36TopicBufferLine);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, kBusterTopic0x38Line)) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x176b, "BUSTER", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC058FstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(
				kBusterTopic0x38ResponseLine, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1)
				return runtime.playDialogueLineWithVariant(0x1781, "BUSTER", 0);
			if (responseIndex == 2) {
				lineError = runtime.playDialogueEntrySequence(kBusterTopic0x38ResponseTwoLines,
					ARRAYSIZE(kBusterTopic0x38ResponseTwoLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}

			topics.assign(kBusterTopic0x38TopicBufferLine);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, kBusterTopic0x3bLine)) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x17a5, "BUSTER", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(
				kBusterTopic0x3bResponseLine, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = runtime.playDialogueLineWithVariant(0x17b3, "BUSTER", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else if (responseIndex == 2) {
				lineError = runtime.playDialogueLineWithVariant(0x17b7, "BUSTER", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}

			lineError = runtime.playDialogueEntrySequence(kBusterTopic0x3bTailLines, ARRAYSIZE(kBusterTopic0x3bTailLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			topics.assign(kBusterTopic0x3bTopicBufferLine);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, kBusterTopic0x3eLine)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(kBusterTopic0x3eLines, ARRAYSIZE(kBusterTopic0x3eLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			topics.assign(kBusterTopic0x3eTopicBufferLine);
			continue;
		}

		if (runtime.matchesAnyResponseLine(selectedTopic, kBusterTopic0x40Or0x41Lines,
				ARRAYSIZE(kBusterTopic0x40Or0x41Lines))) {
			Common::Error lineError = runtime.playDialogueEntrySequence(kBusterTopic0x40Or0x41PreludeLines,
				ARRAYSIZE(kBusterTopic0x40Or0x41PreludeLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(
				kBusterTopic0x40ResponseLine, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1)
				return playFatalBusterBranch(runtime, 0x1804);
			if (responseIndex == 2) {
				lineError = runtime.playDialogueLineWithVariant(0x180b, "BUSTER", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				topics.assign(kBusterTopic0x40TopicBufferLine);
			}
			continue;
		}

		if (runtime.matchesAnyResponseLine(selectedTopic, kBusterTopic0x44Or0x45Or0x46Lines,
				ARRAYSIZE(kBusterTopic0x44Or0x45Or0x46Lines))) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x181e, "BUSTER", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			topics.assign(kBusterTopic0x44TopicBufferLine);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, kBusterTopic0x48Line))
			return playFatalBusterBranch(runtime, 0x18df, 2);

		if (runtime.matchesResponseLine(selectedTopic, kBusterTopic0x49Line)) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x186f, "BUSTER", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	}
}

} // End of namespace Harvester
