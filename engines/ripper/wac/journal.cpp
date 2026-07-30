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

#include "ripper/wac/journal.h"

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
#include "ripper/modal_dialog.h"
#include "ripper/ripper.h"
#include "ripper/settings.h"
#include "ripper/wac/database.h"
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
static const uint16 kWacDatabaseSelectionChanged = 0xfffe;
static const uint16 kWacDatabaseTextScrolled = 0xfffd;
static const uint16 kNoAction = WacManager::kNoAction;
static const uint16 kExitAction = WacManager::kExitAction;

static bool wacJournalAnswersMatch(const Common::String &entered,
		const Common::String &expected) {
	uint enteredIndex = 0;
	uint expectedIndex = 0;
	while (true) {
		while (enteredIndex < entered.size() &&
				!Common::isAlnum((byte)entered[enteredIndex]))
			++enteredIndex;
		while (expectedIndex < expected.size() &&
				!Common::isAlnum((byte)expected[expectedIndex]))
			++expectedIndex;
		if (enteredIndex == entered.size() || expectedIndex == expected.size())
			break;
		byte enteredCharacter = (byte)entered[enteredIndex++];
		byte expectedCharacter = (byte)expected[expectedIndex++];
		if (enteredCharacter >= 'A' && enteredCharacter <= 'Z')
			enteredCharacter += 'a' - 'A';
		if (expectedCharacter >= 'A' && expectedCharacter <= 'Z')
			expectedCharacter += 'a' - 'A';
		if (enteredCharacter != expectedCharacter)
			return false;
	}
	while (enteredIndex < entered.size() &&
			!Common::isAlnum((byte)entered[enteredIndex]))
		++enteredIndex;
	while (expectedIndex < expected.size() &&
			!Common::isAlnum((byte)expected[expectedIndex]))
		++expectedIndex;
	return enteredIndex == entered.size() && expectedIndex == expected.size();
}

static void shuffleJournalLine(Common::String &line,
		Common::RandomSource &randomSource) {
	if (line.size() < 2)
		return;
	// RunWacJournalRevealScene performs twenty random byte swaps per wrapped
	// line on entry and again while each unrevealed row remains visible.
	for (uint iteration = 0; iteration < 20; ++iteration) {
		const uint first = randomSource.getRandomNumber(line.size() - 1);
		const uint second = randomSource.getRandomNumber(line.size() - 1);
		const char temporary = line[first];
		line.setChar(line[second], first);
		line.setChar(temporary, second);
	}
}

WacJournalPuzzle::WacJournalPuzzle(WacDatabaseSession *database) :
		_database(database) {
}

void WacJournalPuzzle::wrapText(const Common::String &text, uint maximumWidth,
		Common::Array<Common::String> &lines) const {
	lines.clear();
	Common::String line;
	Common::String word;
	for (uint index = 0; index <= text.size(); ++index) {
		const char character = index < text.size() ? text[index] : '\n';
		if (character == '\r')
			continue;
		if (character != ' ' && character != '\n') {
			word += character;
			continue;
		}

		if (!word.empty()) {
			const Common::String candidate = line.empty() ? word : line + " " + word;
			if (!line.empty() && _database->measureText(candidate) > maximumWidth) {
				lines.push_back(line);
				line = word;
			} else {
				line = candidate;
			}
			word.clear();
		}
		if (character == '\n') {
			lines.push_back(line);
			line.clear();
		}
	}
	if (lines.empty())
		lines.push_back(Common::String());
}

bool WacJournalPuzzle::drawTextPanel(
		const Common::Array<Common::String> &lines, uint progress,
		uint firstVisible, uint &maximumFirstVisible, uint &visibleRows) {
	const Common::Rect bounds(kWacMediaLeft, kWacMediaTop,
		kWacMediaLeft + kWacTextPanelWidth, kWacMediaTop + kWacTextPanelHeight);
	if (!_database->engine()->getModalDialog()->drawRetainedTextPanelLines(
			lines, bounds, firstVisible, maximumFirstVisible, visibleRows,
			ModalDialogManager::kWacPresentation,
			static_cast<ModalDialogManager::TextPanelScrollControl>(
				_database->_databaseTextScrollControl)))
		return false;

	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return false;
	}
	for (int y = bounds.bottom; y < kWacMediaTop + kWacMediaHeight; ++y)
		memset(screen->getBasePtr(kWacMediaLeft, y), kWacDatabaseBackground,
			kWacMediaWidth);
	const Common::String progressText = Common::String::format("%s %u%%",
		_database->resourceString(kWacJournalProgressResource).c_str(), progress);
	_database->drawText((byte *)screen->getPixels(), screen->pitch,
		kWacJournalProgressLeft, kWacJournalProgressTop, progressText,
		kWacDatabaseNormalText);
	g_system->unlockScreen();
	_database->drawAnimatedCorner(bounds.left, bounds.top);
	_database->engine()->getCursor()->refresh();
	g_system->updateScreen();
	return true;
}

bool WacJournalPuzzle::drawTextPanelLine(
		const Common::Array<Common::String> &lines,
		uint firstVisible, uint visibleRows, uint lineIndex) {
	const Common::Rect bounds(kWacMediaLeft, kWacMediaTop,
		kWacMediaLeft + kWacTextPanelWidth, kWacMediaTop + kWacTextPanelHeight);
	return _database->engine()->getModalDialog()->drawRetainedTextPanelLine(
		lines, bounds, firstVisible, visibleRows, lineIndex,
		ModalDialogManager::kWacPresentation);
}

uint16 WacJournalPuzzle::run(byte entryIndex,
		const Common::String &entryLabel) {
	// RunWacJournalRevealScene at 0x24261 owns control 0x7b2. It wraps resource
	// 0xaf once, scrambles those row buffers in place, and restores one original
	// row after the current section's 36-tick delay while flags 0xfa+n permit it.
	const Common::String &journalText =
		_database->resourceString(kWacJournalTextResource);
	const Common::String &prompt =
		_database->resourceString(kWacJournalPromptResource);
	const uint puzzleLevel = CLIP<uint>(
		_database->engine()->getSettings()->getPuzzleLevel(), 1, 3);
	const uint passwordResource = kWacJournalPasswordBaseResource + puzzleLevel;
	const Common::String &expectedPassword =
		_database->resourceString(passwordResource);
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
	Milestones *milestones = _database->engine()->getMilestones();
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

	uint firstVisible = 0;
	uint maximumFirstVisible = 0;
	uint visibleRows = 0;
	_database->_databaseTextScrollControl =
		ModalDialogManager::kTextPanelScrollNone;
	_database->_databaseTextScrollDragging = false;
	_database->_databaseTextScrollDragOffset = 0;
	uint progress = sourceLines.empty() ? 100 :
		revealedLines * 100 / sourceLines.size();
	if (!drawTextPanel(displayLines, progress, firstVisible,
			maximumFirstVisible, visibleRows))
		return kNoAction;

	_database->engine()->getInput()->discardMouseTransitions();
	debugC(1, kDebugWac,
		"Ripper: entered WAC journal scene entry=%u label='%s' function=RunWacJournalRevealScene@0x24261 lines=%u unlocked=%d revealed=%u section=%u passwordResource=0x%x",
		entryIndex, entryLabel.c_str(), sourceLines.size(), unlocked,
		revealedLines, revealFlagIndex, passwordResource);
	debugC(2, kDebugWac,
		"Ripper: WAC journal chooser geometry panel=%d,%d,%d,%d progress=%d,%d password=%d,%d,%d,%d source=RunWacJournalRevealScene@0x24538",
		kWacMediaLeft, kWacMediaTop, kWacTextPanelWidth, kWacTextPanelHeight,
		kWacJournalProgressLeft, kWacJournalProgressTop,
		kWacMediaLeft, kWacJournalTextEntryTop,
		kWacJournalTextEntryWidth, kWacJournalTextEntryHeight);

	bool cancelled = false;
	while (!unlocked && !_database->engine()->shouldQuit()) {
		// RunWacJournalRevealScene at 0x24538 places the password control five
		// pixels below the journal panel. Its -1 size request produces one
		// 14-pixel row plus the WAC heading, bottom, and frame insets.
		const Common::Rect entryBounds(kWacMediaLeft, kWacJournalTextEntryTop,
			kWacMediaLeft + kWacJournalTextEntryWidth,
			kWacJournalTextEntryTop + kWacJournalTextEntryHeight);
		if (!_database->engine()->getModalDialog()->beginTextEntry(prompt,
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
				!_database->engine()->shouldQuit()) {
			textResult =
				_database->engine()->getModalDialog()->serviceTextEntry(
					enteredPassword);
			_database->serviceIdleEffects();
			g_system->updateScreen();
			g_system->delayMillis(10);
		}
		if (textResult == ModalDialogManager::kTextEntryCancelled ||
				textResult == ModalDialogManager::kTextEntryFailed) {
			cancelled = true;
			break;
		}
		if (!wacJournalAnswersMatch(enteredPassword, expectedPassword)) {
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
		audioStarted = _database->engine()->getMedia()->playVoiceClip(
			"wacjrnl.wav", journalAudio);
		if (!drawTextPanel(displayLines, progress, firstVisible,
				maximumFirstVisible, visibleRows))
			cancelled = true;
	}

	uint16 result = cancelled ? 0x1b : kNoAction;
	uint32 revealStartMillis = g_system->getMillis(true);
	uint32 lastShuffleMillis = revealStartMillis;
	uint visibleShuffleLine = MAX<uint>(revealedLines, firstVisible);
	while (!cancelled && !_database->engine()->shouldQuit()) {
		const uint16 command = _database->serviceDatabaseMediaInput(entryIndex,
			&firstVisible, maximumFirstVisible, visibleRows);
		bool redraw = false;
		int redrawLine = -1;
		if (command == kWacDatabaseTextScrolled ||
				command == MediaSequenceCallback::kContinueRefreshPalette) {
			visibleShuffleLine = MAX<uint>(revealedLines, firstVisible);
			redraw = true;
		} else if (command == kWacDatabaseSelectionChanged ||
				command == kExitAction || command == 0x1b) {
			result = command;
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
			visibleShuffleLine = MAX<uint>(revealedLines, firstVisible);
			redraw = true;
			debugC(2, kDebugWac,
				"Ripper: WAC journal reveal line=%u/%u section=%u enabled=%d progress=%u complete=%d delayTicks=36",
				revealedLines, sourceLines.size(), revealFlagIndex,
				sectionEnabled, progress, revealComplete);
		}

		if (unlocked && !revealComplete &&
				now - lastShuffleMillis >= kWacJournalShuffleInterval) {
			const uint firstShuffleLine = MAX<uint>(revealedLines, firstVisible);
			const uint shuffleLimit = MIN<uint>(displayLines.size(),
				firstVisible + visibleRows);
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

		if (redraw && !drawTextPanel(displayLines, progress, firstVisible,
				maximumFirstVisible, visibleRows))
			break;
		if (!redraw && redrawLine >= 0 &&
				!drawTextPanelLine(displayLines, firstVisible,
					visibleRows, redrawLine))
			break;
		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	if (audioStarted)
		_database->engine()->getMedia()->stopSoundEffect(journalAudio);
	_database->clearDatabaseMediaViewport();
	_database->_databaseTextScrollControl =
		ModalDialogManager::kTextPanelScrollNone;
	_database->_databaseTextScrollDragging = false;
	_database->drawDatabase();
	debugC(1, kDebugWac,
		"Ripper: left WAC journal scene entry=%u result=0x%x unlocked=%d revealed=%u/%u section=%u audioStarted=%d",
		entryIndex, result, unlocked, revealedLines, sourceLines.size(),
		revealFlagIndex, audioStarted);
	if (result == kWacDatabaseSelectionChanged &&
			_database->_databaseSelection < _database->_databaseEntries.size())
		return _database->dispatchDatabaseEntry(
			_database->_databaseEntries[_database->_databaseSelection]);
	return result == kExitAction ? kExitAction : kNoAction;
}

} // End of namespace Ripper
