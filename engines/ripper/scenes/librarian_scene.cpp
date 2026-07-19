/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of their respective copyright holders.
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
 */

#include "ripper/scenes/librarian_scene.h"

#include "common/debug.h"
#include "common/system.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/dialogue.h"
#include "ripper/input.h"
#include "ripper/milestones.h"
#include "ripper/puzzles/ka_book_code.h"
#include "ripper/ripper.h"
#include "ripper/script.h"

namespace Ripper {

namespace {

static const char *const kDeckMedia = "ka_deck.avi";
static const char *const kLoopMedia = "ka_loop.smk";
static const char *const kCardMedia = "ka_card.avi";
static const char *const kBookMedia = "ka_book.avi";
static const char *const kCdMedia = "ka_cd.avi";
static const char *const kCdGateScene = "SB2_1_D";
static const char *const kFirstGreetingAudio = "li1_1_v1.wav";
static const char *const kRepeatGreetingAudio[2] = {
	"li1_1_z1.wav", "li1_1_z3.wav"
};
static const char *const kLoopCueAudio[3] = {
	"library2.wav", "library1.wav", "library0.wav"
};
static const uint kLoopCueFrames[3] = { 1, 28, 45 };
static const uint kLoopCueVolumes[3] = { 25, 20, 10 };

static const uint kDefaultCursor = 14;
static const uint kStartCursor = 11;
static const uint kCardCursor = 6;
static const uint kChoiceCursor = 16;
static const uint16 kEscapeCommand = 0x1b;
static const uint16 kStartCommand = 0x672;
static const uint16 kCardCommand = 0x673;
static const uint16 kChoiceCommandBase = 0x7000;
static const uint16 kFailureCommand = 0x7ffd;
static const uint kLoopStartFrame = 1;

static const uint16 kFirstChoiceId = 0;
static const uint16 kSecondChoiceId = 1;
static const uint16 kBookChoiceId = 2;
static const uint16 kCdChoiceId = 3;
static const uint kChoiceCount = 4;

static const uint kCardSeenFlag = 0xcc;
static const uint kFirstChoiceFlag = 0x14a;
static const uint kSecondChoiceFlag = 0x14b;
static const uint kBookChoiceFlag = 0x14c;
static const uint kCdChoiceFlag = 0x14d;
static const uint kBookSolvedFlag = 0xe1;
static const uint kBookFollowupFlag = 0x53;
static const uint kCdFollowupFlag = 0x54;
static const uint kActOneCompleteFlag = 2;
static const uint kFirstChoiceText = 0xaa;

static const int kStartLeft = 294;
static const int kStartTop = 103;
static const int kStartRight = 418;
static const int kStartBottom = 147;
static const int kCardLeft = 413;
static const int kCardTop = 213;
static const int kCardRight = 467;
static const int kCardBottom = 303;

} // End of anonymous namespace

LibrarianScene::LibrarianScene(RipperEngine *engine) : _engine(engine),
		_chooser(*engine->getScripts()->getDialogue()),
		_random("ripper-ka-dialogue"), _hoveredControl(-1),
		_conversationStarted(false), _voicePending(false) {
}

bool LibrarianScene::initialize() {
	if (!_engine->getResources()->loadGameText(_gameText)) {
		warning("Ripper: could not load Ka dialogue game text");
		return false;
	}
	if (_gameText.size() <= kFirstChoiceText + 3) {
		warning("Ripper: Ka dialogue text table has only %u entries", _gameText.size());
		return false;
	}
	debugC(1, kDebugDialogue,
		"Ripper: initialized Ka dialogue textChoices=4 chooser=shared");
	return true;
}

void LibrarianScene::rebuildChoices() {
	// RunKaDialogueScene at 0x2aef5 builds this list from GAMETEXT.TF entries
	// 0xaa..0xad and the original 0x14a..0x14d progress bits, then passes the
	// resulting item model to the same chooser control 0x4e2 used by opcode 0x17.
	if (_chooser.isPending())
		_chooser.dismissForSceneTransition("ka-choice-rebuild");
	else
		_chooser.clearPending();
	_choices.clear();
	Milestones *milestones = _engine->getMilestones();
	static const Choice choices[kChoiceCount] = {
		{ kFirstChoiceId, kFirstChoiceFlag, kFirstChoiceText, "li1_1_va.wav" },
		{ kSecondChoiceId, kSecondChoiceFlag, kFirstChoiceText + 1, "li1_1_vb.wav" },
		{ kBookChoiceId, kBookChoiceFlag, kFirstChoiceText + 2, "li1_1_vd.wav" },
		{ kCdChoiceId, kCdChoiceFlag, kFirstChoiceText + 3, "li1_1_vc.wav" }
	};
	if (!milestones->isSet(choices[0].flag))
		_choices.push_back(&choices[0]);
	if (!milestones->isSet(choices[1].flag))
		_choices.push_back(&choices[1]);
	if (milestones->isSet(kCardSeenFlag) && !milestones->isSet(kBookSolvedFlag))
		_choices.push_back(&choices[2]);
	if (milestones->isSet(kActOneCompleteFlag) && !milestones->isSet(kCdChoiceFlag) &&
			_engine->getScripts()->hasPlayedScene(kCdGateScene))
		_choices.push_back(&choices[3]);
	if (_conversationStarted && !_choices.empty()) {
		for (uint i = 0; i < _choices.size(); ++i)
			_chooser.appendChoice(_gameText[_choices[i]->textResource], _choices[i]->id);
		_chooser.activateChoices("ka-dialogue");
	}
	debugC(2, kDebugDialogue,
		"Ripper: rebuilt Ka dialogue choices count=%u conversationStarted=%d sharedPending=%d flags=cc:%d e1:%d 54:%d",
		_choices.size(), _conversationStarted, _chooser.isPending(),
		milestones->isSet(kCardSeenFlag), milestones->isSet(kBookSolvedFlag),
		milestones->isSet(kCdFollowupFlag));
}

const LibrarianScene::Choice *LibrarianScene::findAvailableChoice(uint16 id) const {
	for (uint i = 0; i < _choices.size(); ++i) {
		if (_choices[i]->id == id)
			return _choices[i];
	}
	return nullptr;
}

bool LibrarianScene::startVoice(const char *path, const char *source) {
	_engine->getMedia()->stopSoundEffect(_voiceHandle);
	if (!_engine->getMedia()->playSoundEffect(path, _voiceHandle)) {
		warning("Ripper: could not start Ka dialogue voice '%s'", path);
		return false;
	}
	_voicePending = true;
	if (_chooser.isPending())
		_chooser.dismissForSceneTransition("ka-voice-start");
	_choices.clear();
	_hoveredControl = -1;
	debugC(2, kDebugDialogue,
		"Ripper: started Ka dialogue voice media='%s' source=%s", path, source);
	return true;
}

bool LibrarianScene::serviceVoiceCompletion() {
	Milestones *milestones = _engine->getMilestones();
	if (milestones->isSet(kBookChoiceFlag) && !milestones->isSet(kBookSolvedFlag)) {
		KaBookCodePuzzle puzzle(_engine);
		const KaBookCodePuzzle::Result puzzleResult = puzzle.run();
		if (puzzleResult == KaBookCodePuzzle::kLoadFailed)
			return false;
		if (puzzleResult == KaBookCodePuzzle::kSolved) {
			if (!_engine->getMedia()->play(kBookMedia, true))
				return false;
			milestones->set(kBookSolvedFlag, true, "ka-dialogue-book-code");
			milestones->set(kBookFollowupFlag, true, "ka-dialogue-book-code");
		} else {
			milestones->set(kBookChoiceFlag, false, "ka-dialogue-book-code");
		}
	}
	if (milestones->isSet(kCdChoiceFlag) && !milestones->isSet(kCdFollowupFlag)) {
		if (!_engine->getMedia()->play(kCdMedia, true))
			return false;
		milestones->set(kCdFollowupFlag, true, "ka-dialogue-cd");
	}
	rebuildChoices();
	return true;
}

void LibrarianScene::serviceLoopAudio(uint frame) {
	// The original Ka loop queues descriptors 2, 1, and 0 at frames 1, 28,
	// and 45, using the packed 25%, 20%, and 10% volumes initialized at entry.
	for (uint i = 0; i < ARRAYSIZE(kLoopCueFrames); ++i) {
		if (frame != kLoopCueFrames[i])
			continue;
		_engine->getMedia()->playSoundEffect(kLoopCueAudio[i], _loopCueHandles[i],
			kLoopCueVolumes[i]);
		debugC(3, kDebugDialogue,
			"Ripper: queued Ka loop cue media='%s' frame=%u volume=%u",
			kLoopCueAudio[i], frame, kLoopCueVolumes[i]);
	}
}

void LibrarianScene::presentDialogueOverlay(uint frame) {
	if (!_chooser.isPending())
		return;

	// RunKaDialogueScene at 0x2aef5 advances the packetized frame, acquires
	// the UI presentation overlay, submits a complete dirty-region update, and
	// only then advances KA_LOOP.SMK. Draw the shared chooser in the callback;
	// MediaPlayer submits the fully composited frame after this returns.
	_chooser.draw(true);
	debugC(11, kDebugDialogue,
		"Ripper: composited Ka dialogue chooser above loop frame=%u", frame);
}

void LibrarianScene::updateCursor(const Common::Point &point) {
	int hovered = -1;
	uint cursor = kDefaultCursor;
	_chooser.updateHover(point);
	const bool startEnabled = !_voicePending &&
		(!_conversationStarted || _choices.empty());
	const bool cardEnabled = !_voicePending &&
		!_engine->getMilestones()->isSet(kCardSeenFlag);
	if (_chooser.contains(point)) {
		hovered = 2;
		cursor = kChoiceCursor;
	} else if (startEnabled && point.x >= kStartLeft && point.x < kStartRight &&
			point.y >= kStartTop && point.y < kStartBottom) {
		hovered = 0;
		cursor = kStartCursor;
	} else if (cardEnabled && point.x >= kCardLeft && point.x < kCardRight &&
			point.y >= kCardTop && point.y < kCardBottom) {
		hovered = 1;
		cursor = kCardCursor;
	}
	if (hovered != _hoveredControl) {
		debugC(2, kDebugDialogue,
			"Ripper: Ka dialogue hover control=%d previous=%d point=%d,%d cursor=%u",
			hovered, _hoveredControl, point.x, point.y, cursor);
		_hoveredControl = hovered;
	}
	_engine->getCursor()->update(cursor);
	_engine->getCursor()->setVisible(true);
}

uint16 LibrarianScene::serviceInput() {
	while (_engine->getInput()->hasPendingKey()) {
		const uint16 command = _engine->getInput()->consumeKey();
		if (command == kEscapeCommand) {
			if (_voicePending) {
				_engine->getMedia()->stopSoundEffect(_voiceHandle);
				_voicePending = false;
				rebuildChoices();
				debugC(2, kDebugDialogue,
					"Ripper: Escape stopped active Ka dialogue voice");
				continue;
			}
			return kEscapeCommand;
		}
		uint choiceId = 0;
		if (_chooser.serviceKeyboard(command, choiceId))
			return kChoiceCommandBase + choiceId;
	}

	const MouseState mouse = _engine->getInput()->publishMouseState();
	updateCursor(mouse.position);
	uint choiceId = 0;
	if (_chooser.service(mouse, choiceId))
		return kChoiceCommandBase + choiceId;
	if ((mouse.pressed & kMouseButtonLeft) == 0)
		return 0;
	if (_hoveredControl == 0)
		return kStartCommand;
	if (_hoveredControl == 1)
		return kCardCommand;
	return 0;
}

uint16 LibrarianScene::service(uint frame) {
	serviceLoopAudio(frame);
	if (_voicePending && !_engine->getMedia()->isSoundEffectActive(_voiceHandle)) {
		_voicePending = false;
		debugC(2, kDebugDialogue,
			"Ripper: Ka dialogue voice completed frame=%u", frame);
		if (!serviceVoiceCompletion())
			return kFailureCommand;
		updateCursor(_engine->getInput()->peekMouseState().position);
		presentDialogueOverlay(frame);
		return MediaSequenceCallback::kContinueRefreshPalette;
	}
	uint16 command = serviceInput();
	if (command == kStartCommand) {
		const char *voice = !_conversationStarted ? kFirstGreetingAudio :
			kRepeatGreetingAudio[_random.getRandomNumber(1)];
		_conversationStarted = true;
		if (!startVoice(voice, "start-control"))
			return kFailureCommand;
		command = 0;
	} else if (command == kCardCommand) {
		if (_chooser.isPending())
			_chooser.dismissForSceneTransition("ka-card-presentation");
		_choices.clear();
		if (!_engine->getMedia()->play(kCardMedia, true) ||
				!_engine->getMilestones()->set(kCardSeenFlag, true, "ka-dialogue-card"))
			return kFailureCommand;
		if (_conversationStarted)
			rebuildChoices();
		updateCursor(_engine->getInput()->peekMouseState().position);
		presentDialogueOverlay(frame);
		return MediaSequenceCallback::kContinueRefreshPalette;
	} else if (command >= kChoiceCommandBase &&
			command < kChoiceCommandBase + kChoiceCount) {
		const uint16 choiceId = command - kChoiceCommandBase;
		const Choice *choice = findAvailableChoice(choiceId);
		if (!choice)
			return kFailureCommand;
		if (!_engine->getMilestones()->set(choice->flag, true, "ka-dialogue-choice") ||
				!startVoice(choice->audioPath, "choice"))
			return kFailureCommand;
		debugC(2, kDebugDialogue,
			"Ripper: selected Ka dialogue choice id=%u flag=0x%x textResource=0x%x audio='%s'",
			choiceId, choice->flag, choice->textResource, choice->audioPath);
		command = 0;
	}
	presentDialogueOverlay(frame);
	if (command != 0) {
		debugC(2, kDebugDialogue,
			"Ripper: Ka dialogue input command=0x%04x frame=%u choices=%u voicePending=%d",
			command, frame, _choices.size(), _voicePending);
	}
	return command;
}

void LibrarianScene::stopAllAudio() {
	_engine->getMedia()->stopSoundEffect(_ambientHandle);
	_engine->getMedia()->stopSoundEffect(_deckCueHandle);
	for (uint i = 0; i < ARRAYSIZE(_loopCueHandles); ++i)
		_engine->getMedia()->stopSoundEffect(_loopCueHandles[i]);
	_engine->getMedia()->stopSoundEffect(_voiceHandle);
}

LibrarianScene::Result LibrarianScene::run(uint sceneArgument) {
	if (!initialize())
		return kLoadFailed;
	if (_chooser.isPending())
		_chooser.dismissForSceneTransition("ka-entry");
	else
		_chooser.clearPending();
	debugC(1, kDebugDialogue,
		"Ripper: entered Ka dialogue scene argument=%u deck='%s' loop='%s' toolbarMask=0x0000",
		sceneArgument, kDeckMedia, kLoopMedia);
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->setSelectionIndex(kDefaultCursor);
	_engine->getCursor()->dispatchSelectionIndexChange(kDefaultCursor);
	_engine->getCursor()->setVisible(false);
	_engine->getMedia()->playSoundEffect("deck10.wav", _deckCueHandle);
	if (!_engine->getMedia()->play(kDeckMedia, false)) {
		stopAllAudio();
		return kLoadFailed;
	}
	// RunKaDialogueScene at 0x2aef5 issues display command 0x14 after
	// KA_DECK.AVI. ClearGenericVideoLogicalPage at 0x45ed8 zeroes the page so
	// KA_LOOP.SMK's 640x300 palette cannot recolor stale pixels in the bands.
	g_system->fillScreen(0);
	g_system->updateScreen();
	debugC(2, kDebugDialogue,
		"Ripper: cleared Ka deck display before loop media='%s' source=display-command-0x14",
		kLoopMedia);
	_engine->getMedia()->playSoundEffect("library3.wav", _ambientHandle, 100, true);

	uint16 command = 0;
	const bool played = _engine->getMedia()->playPuzzleSequence(
		kLoopMedia, kLoopStartFrame, this, &command);
	const Result result = (!played && !_engine->shouldQuit()) || command == kFailureCommand ?
		kLoadFailed : kExited;

	if (_chooser.isPending())
		_chooser.dismissForSceneTransition("ka-exit");
	stopAllAudio();
	_engine->getCursor()->setVisible(false);
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	debugC(result == kExited ? 1 : 2, kDebugDialogue,
		"Ripper: left Ka dialogue scene result=%d conversationStarted=%d choices=%u quit=%d",
		result, _conversationStarted, _choices.size(), _engine->shouldQuit());
	return result;
}

} // End of namespace Ripper
