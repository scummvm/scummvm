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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/wac/database.h"

#include "audio/mixer.h"
#include "common/debug.h"
#include "common/random.h"
#include "common/system.h"
#include "common/util.h"
#include "graphics/surface.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/media.h"
#include "ripper/milestones.h"
#include "ripper/modal/modal_dialog.h"
#include "ripper/ripper.h"
#include "ripper/settings.h"
#include "ripper/wac/wac.h"

namespace Ripper {

static const uint32 kDosTickMillis = 55;
static const int kWacDatabaseLeftInset = 5;
static const int kWacDatabaseRightInset = 20;
static const int kWacDatabaseTextInset = 2;
static const byte kWacDatabaseBackground = 4;
static const byte kWacDatabaseNormalText = 251;
static const int kWacMediaLeft = 50;
static const int kWacMediaTop = 50;
static const int kWacMediaWidth = 350;
static const int kWacMediaHeight = 282;
static const int kWacTextPanelWidth = 330;
static const int kWacTextPanelHeight = 222;
static const uint kWacJournalProgressResource = 0xae;
static const uint kWacJournalTextResource = 0xaf;
static const uint kWacJournalPromptResource = 0xb5;
static const uint kWacJournalPasswordBaseResource = 0xb6;
static const uint kWacJournalHelpResource = 407;
static const uint kWacJournalUnlockedFlag = 0xd8;
static const uint kWacJournalRevealFlagBase = 0xfa;
static const uint kWacJournalCompletionFlagBase = 0x104;
static const uint kWacJournalSectionCount = 8;
static const uint kWacJournalPasswordMaximumLength = 20;
static const uint32 kWacJournalRevealInterval = 36 * kDosTickMillis;
static const uint32 kWacJournalShuffleInterval = kDosTickMillis;
static const int kWacJournalProgressLeft = kWacMediaLeft + 50;
static const int kWacJournalProgressTop = kWacMediaTop + 1;
static const int kWacJournalTextEntryWidth = 200;
static const int kWacJournalTextEntryHeight = 42;
static const int kWacJournalTextEntryTop =
	kWacMediaTop + kWacTextPanelHeight + 5;
static const uint16 kNoAction = WacManager::kNoAction;
static const uint16 kExitAction = WacManager::kExitAction;

static void shuffleJournalLine(Common::String &line,
		Common::RandomSource &randomSource) {
	if (line.size() < 2)
		return;
	for (uint iteration = 0; iteration < 20; ++iteration) {
		const uint first = randomSource.getRandomNumber(line.size() - 1);
		const uint second = randomSource.getRandomNumber(line.size() - 1);
		const char temporary = line[first];
		line.setChar(line[second], first);
		line.setChar(temporary, second);
	}
}

bool WacDatabaseSession::drawJournalTextPanel(
		const Common::Array<Common::String> &lines, uint progress,
		TextScrollState &scroll) {
	const Common::Rect bounds(kWacMediaLeft, kWacMediaTop,
		kWacMediaLeft + kWacTextPanelWidth, kWacMediaTop + kWacTextPanelHeight);
	if (!engine()->getModalDialog()->drawRetainedTextPanelLines(
			lines, bounds, scroll.firstVisible, scroll.maximumFirstVisible, scroll.visibleRows,
			ModalDialogManager::kWacPresentation,
			scroll.control))
		return false;

	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return false;
	}
	screen->fillRect(Common::Rect(kWacMediaLeft, bounds.bottom,
		kWacMediaLeft + kWacMediaWidth, kWacMediaTop + kWacMediaHeight),
		kWacDatabaseBackground);
	const Common::String progressText = Common::String::format("%s %u%%",
		resourceString(kWacJournalProgressResource).c_str(), progress);
	drawText((byte *)screen->getPixels(), screen->pitch,
		kWacJournalProgressLeft, kWacJournalProgressTop, progressText,
		kWacDatabaseNormalText);
	g_system->unlockScreen();
	drawAnimatedCorner(bounds.left, bounds.top);
	engine()->getCursor()->refresh();
	presentScreen();
	return true;
}

bool WacDatabaseSession::drawJournalTextPanelLine(
		const Common::Array<Common::String> &lines,
		uint firstVisible, uint visibleRows, uint lineIndex) {
	const Common::Rect bounds(kWacMediaLeft, kWacMediaTop,
		kWacMediaLeft + kWacTextPanelWidth, kWacMediaTop + kWacTextPanelHeight);
	return engine()->getModalDialog()->drawRetainedTextPanelLine(
		lines, bounds, firstVisible, visibleRows, lineIndex,
		ModalDialogManager::kWacPresentation);
}

uint16 WacDatabaseSession::runJournal(byte entryIndex,
		const Common::String &entryLabel) {
	const Common::String &journalText =
		resourceString(kWacJournalTextResource);
	const Common::String &prompt =
		resourceString(kWacJournalPromptResource);
	const uint puzzleLevel = CLIP<uint>(
		engine()->getSettings()->getPuzzleLevel(), 1, 3);
	const uint passwordResource = kWacJournalPasswordBaseResource + puzzleLevel;
	const Common::String &expectedPassword =
		resourceString(passwordResource);
	if (journalText.empty() || prompt.empty() || expectedPassword.empty()) {
		warning("Ripper: WAC journal resources are incomplete text=0x%x prompt=0x%x password=0x%x",
			kWacJournalTextResource, kWacJournalPromptResource, passwordResource);
		return kNoAction;
	}

	Common::Array<Common::String> sourceLines;
	// The tertiary WAC text panel reserves five pixels on the left, twenty on
	// the right, and two more on either side of the actual text rows.
	wrapText(journalText,
		kWacTextPanelWidth - kWacDatabaseLeftInset -
			kWacDatabaseRightInset - kWacDatabaseTextInset * 2,
		sourceLines);
	Common::Array<Common::String> displayLines = sourceLines;
	Milestones *milestones = engine()->getMilestones();
	milestones->set(kWacJournalRevealFlagBase, true, "wac-journal-entry");

	uint revealFlagIndex = 0;
	uint revealedLines = 0;
	bool unlocked = milestones->isSet(kWacJournalUnlockedFlag);
	if (unlocked) {
		while (revealedLines < sourceLines.size() &&
				revealFlagIndex < kWacJournalSectionCount &&
				milestones->isSet(kWacJournalRevealFlagBase + revealFlagIndex)) {
			milestones->set(kWacJournalCompletionFlagBase + revealFlagIndex,
				true, "wac-journal-resume");
			++revealedLines;
			if (revealedLines < sourceLines.size() &&
					sourceLines[revealedLines].empty())
				++revealFlagIndex;
		}
	}
	bool revealComplete = revealedLines >= sourceLines.size();

	Common::RandomSource randomSource("ripper-wac-journal");
	if (!revealComplete) {
		for (uint lineIndex = revealedLines; lineIndex < displayLines.size(); ++lineIndex)
			shuffleJournalLine(displayLines[lineIndex], randomSource);
	}

	DatabaseInputState input(DatabaseInputMode::kTextPanel);
	TextScrollState &scroll = input.text;
	uint progress = sourceLines.empty() ? 100 :
		revealedLines * 100 / sourceLines.size();
	if (!drawJournalTextPanel(displayLines, progress, scroll))
		return kNoAction;

	engine()->getInput()->discardMouseTransitions();
	debugC(1, kDebugWac,
		"Ripper: entered WAC journal scene entry=%u label='%s' lines=%u unlocked=%d revealed=%u section=%u passwordResource=0x%x",
		entryIndex, entryLabel.c_str(), sourceLines.size(), unlocked,
		revealedLines, revealFlagIndex, passwordResource);
	debugC(2, kDebugWac,
		"Ripper: WAC journal chooser geometry panel=%d,%d,%d,%d progress=%d,%d password=%d,%d,%d,%d",
		kWacMediaLeft, kWacMediaTop, kWacTextPanelWidth, kWacTextPanelHeight,
		kWacJournalProgressLeft, kWacJournalProgressTop,
		kWacMediaLeft, kWacJournalTextEntryTop,
		kWacJournalTextEntryWidth, kWacJournalTextEntryHeight);

	bool cancelled = false;
	while (!unlocked && !engine()->shouldQuit()) {
		// The single-row password control includes the WAC heading and frame insets.
		const Common::Rect entryBounds(kWacMediaLeft, kWacJournalTextEntryTop,
			kWacMediaLeft + kWacJournalTextEntryWidth,
			kWacJournalTextEntryTop + kWacJournalTextEntryHeight);
		if (!engine()->getModalDialog()->beginTextEntry(prompt,
				kWacJournalPasswordMaximumLength, kWacJournalHelpResource,
				"wac-journal-password", ModalDialogManager::kWacPresentation,
				entryBounds)) {
			cancelled = true;
			break;
		}

		Common::String enteredPassword;
		ModalDialogManager::TextEntryResult textResult =
			ModalDialogManager::kTextEntryPending;
		while (textResult == ModalDialogManager::kTextEntryPending &&
				!engine()->shouldQuit()) {
			textResult =
				engine()->getModalDialog()->serviceTextEntry(
					enteredPassword);
			serviceIdleEffects();
			presentScreen();
			g_system->delayMillis(10);
		}
		if (textResult == ModalDialogManager::kTextEntryCancelled ||
				textResult == ModalDialogManager::kTextEntryFailed) {
			cancelled = true;
			break;
		}
		if (!textAnswersMatch(enteredPassword, expectedPassword)) {
			debugC(2, kDebugWac,
				"Ripper: WAC journal rejected decryption key resource=0x%x enteredLength=%u",
				passwordResource, enteredPassword.size());
			continue;
		}

		milestones->set(kWacJournalUnlockedFlag, true, "wac-journal-password");
		unlocked = true;
		revealedLines = 0;
		revealFlagIndex = 0;
		revealComplete = false;
		progress = 0;
		debugC(1, kDebugWac,
			"Ripper: WAC journal accepted decryption key resource=0x%x flag=0x%x",
			passwordResource, kWacJournalUnlockedFlag);
	}

	Audio::SoundHandle journalAudio;
	bool audioStarted = false;
	if (unlocked && !revealComplete && !cancelled) {
		audioStarted = engine()->getMedia()->playVoiceClip(
			"wacjrnl.wav", journalAudio);
		if (!drawJournalTextPanel(displayLines, progress, scroll))
			cancelled = true;
	}

	DatabaseInputOutcome result = cancelled ? DatabaseInputOutcome::kCloseEntry : DatabaseInputOutcome::kContinue;
	uint32 revealStartMillis = g_system->getMillis(true);
	uint32 lastShuffleMillis = revealStartMillis;
	uint visibleShuffleLine = MAX<uint>(revealedLines, scroll.firstVisible);
	while (!cancelled && !engine()->shouldQuit()) {
		const DatabaseInputResult event = serviceDatabaseMediaInput(entryIndex, input);
		bool redraw = false;
		int redrawLine = -1;
		if (event.outcome == DatabaseInputOutcome::kRedrawText ||
				event.outcome == DatabaseInputOutcome::kRefreshPalette) {
			visibleShuffleLine = MAX<uint>(revealedLines, scroll.firstVisible);
			redraw = true;
		} else if (event.closesEntry()) {
			result = event.outcome;
			break;
		}

		const uint32 now = g_system->getMillis(true);
		if (unlocked && !revealComplete &&
				now - revealStartMillis >= kWacJournalRevealInterval) {
			displayLines[revealedLines] = sourceLines[revealedLines];
			const bool sectionEnabled =
				revealFlagIndex < kWacJournalSectionCount &&
				milestones->isSet(kWacJournalRevealFlagBase + revealFlagIndex);
			if (sectionEnabled) {
				milestones->set(kWacJournalCompletionFlagBase + revealFlagIndex,
					true, "wac-journal-reveal");
				++revealedLines;
				if (revealedLines < sourceLines.size() &&
						sourceLines[revealedLines].empty())
					++revealFlagIndex;
			}
			revealComplete = revealedLines >= sourceLines.size();
			progress = sourceLines.empty() ? 100 :
				revealedLines * 100 / sourceLines.size();
			revealStartMillis = now;
			visibleShuffleLine = MAX<uint>(revealedLines, scroll.firstVisible);
			redraw = true;
			debugC(2, kDebugWac,
				"Ripper: WAC journal reveal line=%u/%u section=%u enabled=%d progress=%u complete=%d delayTicks=36",
				revealedLines, sourceLines.size(), revealFlagIndex,
				sectionEnabled, progress, revealComplete);
		}

		if (unlocked && !revealComplete &&
				now - lastShuffleMillis >= kWacJournalShuffleInterval) {
			const uint firstShuffleLine = MAX<uint>(revealedLines, scroll.firstVisible);
			const uint shuffleLimit = MIN<uint>(displayLines.size(),
				scroll.firstVisible + scroll.visibleRows);
			if (visibleShuffleLine < firstShuffleLine ||
					visibleShuffleLine >= shuffleLimit)
				visibleShuffleLine = firstShuffleLine;
			if (visibleShuffleLine < shuffleLimit) {
				const uint shuffledLine = visibleShuffleLine;
				shuffleJournalLine(displayLines[shuffledLine], randomSource);
				++visibleShuffleLine;
				redrawLine = shuffledLine;
			}
			lastShuffleMillis = now;
		}

		if (redraw && !drawJournalTextPanel(displayLines, progress, scroll))
			break;
		if (!redraw && redrawLine >= 0 &&
				!drawJournalTextPanelLine(displayLines, scroll.firstVisible,
					scroll.visibleRows, redrawLine))
			break;
		presentScreen();
		g_system->delayMillis(10);
	}

	if (audioStarted)
		engine()->getMedia()->stopSoundEffect(journalAudio);
	clearDatabaseMediaViewport();
	drawDatabase();
	debugC(1, kDebugWac,
		"Ripper: left WAC journal scene entry=%u outcome=%d unlocked=%d revealed=%u/%u section=%u audioStarted=%d",
		entryIndex, (int)result, unlocked, revealedLines, sourceLines.size(),
		revealFlagIndex, audioStarted);
	if (result == DatabaseInputOutcome::kSelectEntry &&
			_databaseChooser.selectedIndex() < _databaseEntries.size())
		return dispatchDatabaseEntry(
			_databaseEntries[_databaseChooser.selectedIndex()]);
	return result == DatabaseInputOutcome::kExitWac ? kExitAction : kNoAction;
}

} // End of namespace Ripper
