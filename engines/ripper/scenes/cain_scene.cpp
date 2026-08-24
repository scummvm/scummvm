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
 */

#include "ripper/scenes/cain_scene.h"

#include "common/debug.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/dialogue.h"
#include "ripper/input.h"
#include "ripper/milestones.h"
#include "ripper/resources.h"
#include "ripper/ripper.h"

namespace Ripper {

namespace {

static const char *const kLoopMedia = "cainloop.smk";
static const char *const kLoopAudio = "cainloop.wav";
static const char *const kZapMedia = "cain_zap.avi";
static const char *const kFirstGreetingAudio = "mc3_1_v1.wav";
static const char *const kRepeatGreetingAudio = "mc3_1_z1.wav";

static const uint kDefaultCursor = 14;
static const uint kTalkCursor = 11;
static const uint kChoiceCursor = 16;
static const uint kVoiceCursor = 19;
static const uint kLoopStartFrame = 1;
static const uint kLoopAudioVolume = 50;
static const uint16 kEscapeCommand = 0x1b;
static const uint16 kTalkCommand = 0x672;
static const uint16 kChoiceCommandBase = 0x7000;
static const uint16 kFailureCommand = 0x7ffd;

// RunCainDialogueScene at 0x2c160 uses one contiguous choice-progress range.
// There is no choice ID two because 0x15b stores conversation-started state;
// choice ID three resumes RIPPER.LE's mapping at 0x15c.
static const uint kFirstChoiceFlag = 0x159;
static const uint kConversationStartedFlag = 0x15b;
static const uint kChoiceZeroGateFlag = 0x5c;
static const uint kChoiceThreeGateFlag = 0x5d;
static const uint kZapPlayedFlag = 0x60;
static const uint kChoiceZeroExitUnlockFlag = 0x161;
static const uint kChoiceOneFollowupFlag = 0x198;
static const uint kChoiceFiveFollowupFlag = 0x19e;
static const uint kFirstChoiceText = 0xbe;

static const int kTalkLeft = 244;
static const int kTalkTop = 120;
static const int kTalkRight = 524;
static const int kTalkBottom = 270;

} // End of anonymous namespace

CainScene::CainScene(RipperEngine *engine) : Scene(engine),
		_hoveredControl(-1), _conversationStarted(false), _voicePending(false),
		_choiceListRebuildPending(false) {
}

const CainScene::Choice *CainScene::choiceTable() {
	static const Choice choices[] = {
		{ 0, kFirstChoiceFlag + 0, kFirstChoiceText + 0, "mc3_1_g1.wav" },
		{ 1, kFirstChoiceFlag + 1, kFirstChoiceText + 1, "mc3_1_g2.wav" },
		{ 3, kFirstChoiceFlag + 3, kFirstChoiceText + 3, "mc3_1_v3.wav" },
		{ 4, kFirstChoiceFlag + 4, kFirstChoiceText + 4, "mc3_1_va.wav" },
		{ 5, kFirstChoiceFlag + 5, kFirstChoiceText + 5, "mc3_1_vd.wav" },
		{ 6, kFirstChoiceFlag + 6, kFirstChoiceText + 6, "mc3_1_ve.wav" },
		{ 7, kFirstChoiceFlag + 7, kFirstChoiceText + 7, "mc3_1_vf.wav" }
	};
	return choices;
}

uint CainScene::choiceCount() {
	return 7;
}

bool CainScene::initialize() {
	if (!_engine->getResources()->loadGameText(_gameText)) {
		warning("Ripper: could not load Cain dialogue game text");
		return false;
	}
	if (_gameText.size() < kFirstChoiceText + 7) {
		warning("Ripper: Cain dialogue text table has only %u entries", _gameText.size());
		return false;
	}
	_conversationStarted =
		_engine->getMilestones()->isSet(kConversationStartedFlag);
	_choiceListRebuildPending = _conversationStarted;
	debugC(1, kDebugDialogue,
		"Ripper: initialized Cain dialogue textChoices=%u conversationStarted=%d chooser=shared",
		choiceCount(), _conversationStarted);
	return true;
}

void CainScene::rebuildChoices() {
	if (_chooser.isPending())
		_chooser.dismissForSceneTransition("cain-choice-rebuild");
	else
		_chooser.clearPending();
	_choices.clear();
	_choiceListRebuildPending = false;

	Milestones *milestones = _engine->getMilestones();
	const Choice *choices = choiceTable();
	const bool firstQuestionAsked = milestones->isSet(kFirstChoiceFlag + 4);
	if (!firstQuestionAsked) {
		_choices.push_back(&choices[3]);
	} else {
		for (uint i = 4; i < choiceCount(); ++i) {
			if (!milestones->isSet(choices[i].flag))
				_choices.push_back(&choices[i]);
		}
	}
	if (!milestones->isSet(kChoiceZeroGateFlag)) {
		if (milestones->isSet(kChoiceZeroExitUnlockFlag) &&
				!milestones->isSet(kFirstChoiceFlag))
			_choices.push_back(&choices[0]);
	} else if (!milestones->isSet(kFirstChoiceFlag + 1)) {
		_choices.push_back(&choices[1]);
	}
	if (milestones->isSet(kChoiceThreeGateFlag) &&
			!milestones->isSet(kFirstChoiceFlag + 3))
		_choices.push_back(&choices[2]);

	if (_conversationStarted && !_choices.empty()) {
		for (uint i = 0; i < _choices.size(); ++i)
			_chooser.appendChoice(_gameText[_choices[i]->textResource - 1],
				_choices[i]->id);
		_chooser.activateChoices("cain-dialogue");
	}
	debugC(2, kDebugDialogue,
		"Ripper: rebuilt Cain dialogue choices count=%u conversationStarted=%d "
		"sharedPending=%d flags=159:%d 15a:%d 15d:%d 15e:%d 15f:%d 160:%d 161:%d",
		_choices.size(), _conversationStarted, _chooser.isPending(),
		milestones->isSet(kFirstChoiceFlag),
		milestones->isSet(kFirstChoiceFlag + 1),
		milestones->isSet(kFirstChoiceFlag + 4),
		milestones->isSet(kFirstChoiceFlag + 5),
		milestones->isSet(kFirstChoiceFlag + 6),
		milestones->isSet(kFirstChoiceFlag + 7),
		milestones->isSet(kChoiceZeroExitUnlockFlag));
}

const CainScene::Choice *CainScene::findAvailableChoice(uint16 id) const {
	for (uint i = 0; i < _choices.size(); ++i) {
		if (_choices[i]->id == id)
			return _choices[i];
	}
	return nullptr;
}

bool CainScene::startVoice(const char *path, const char *source) {
	stopAudio(_voiceHandle);
	if (!_engine->getMedia()->playSoundEffect(path, _voiceHandle)) {
		warning("Ripper: could not start Cain dialogue voice '%s'", path);
		return false;
	}
	_voicePending = true;
	if (_chooser.isPending())
		_chooser.dismissForSceneTransition("cain-voice-start");
	_choices.clear();
	_hoveredControl = -1;
	_engine->getCursor()->setSelectionIndex(kVoiceCursor);
	_engine->getCursor()->dispatchSelectionIndexChange(kVoiceCursor);
	debugC(2, kDebugDialogue,
		"Ripper: started Cain dialogue voice media='%s' source=%s cursor=%u input=escape-only",
		path, source, kVoiceCursor);
	return true;
}

bool CainScene::serviceVoiceCompletion() {
	Milestones *milestones = _engine->getMilestones();
	if (milestones->isSet(kFirstChoiceFlag + 6) &&
			!milestones->isSet(kZapPlayedFlag)) {
		if (!_engine->getMedia()->play(kZapMedia, true) ||
				!milestones->set(kZapPlayedFlag, true, "cain-dialogue-zap"))
			return false;
		debugC(2, kDebugDialogue,
			"Ripper: completed Cain zap presentation media='%s' triggerFlag=0x%x completionFlag=0x%x",
			kZapMedia, kFirstChoiceFlag + 6, kZapPlayedFlag);
	}
	if (milestones->isSet(kFirstChoiceFlag + 5) &&
			!milestones->set(kChoiceFiveFollowupFlag, true,
				"cain-dialogue-choice-followup"))
		return false;
	if (milestones->isSet(kFirstChoiceFlag + 1) &&
			!milestones->set(kChoiceOneFollowupFlag, true,
				"cain-dialogue-choice-followup"))
		return false;
	rebuildChoices();
	return true;
}

bool CainScene::finishConversation() {
	Milestones *milestones = _engine->getMilestones();
	return !milestones->isSet(kFirstChoiceFlag + 7) ||
		milestones->set(kChoiceZeroExitUnlockFlag, true, "cain-dialogue-exit");
}

void CainScene::presentDialogueOverlay(uint frame) {
	if (!_chooser.isPending())
		return;
	_chooser.draw(true);
	debugC(11, kDebugDialogue,
		"Ripper: composited Cain dialogue chooser above loop frame=%u", frame);
}

void CainScene::updateCursor(const Common::Point &point) {
	int hovered = -1;
	uint cursor = kDefaultCursor;
	if (_voicePending) {
		_engine->getCursor()->update(kVoiceCursor);
		_engine->getCursor()->setVisible(true);
		return;
	}
	_chooser.updateHover(point);
	const bool talkEnabled = !_conversationStarted || _choices.empty();
	if (_chooser.contains(point)) {
		hovered = 1;
		cursor = kChoiceCursor;
	} else if (talkEnabled && point.x >= kTalkLeft && point.x < kTalkRight &&
			point.y >= kTalkTop && point.y < kTalkBottom) {
		hovered = 0;
		cursor = kTalkCursor;
	}
	if (hovered != _hoveredControl) {
		debugC(2, kDebugDialogue,
			"Ripper: Cain dialogue hover control=%d previous=%d point=%d,%d cursor=%u talkEnabled=%d",
			hovered, _hoveredControl, point.x, point.y, cursor, talkEnabled);
		_hoveredControl = hovered;
	}
	_engine->getCursor()->update(cursor);
	_engine->getCursor()->setVisible(true);
}

uint16 CainScene::serviceInput() {
	while (_engine->getInput()->hasPendingKey()) {
		const uint16 command = _engine->getInput()->consumeKey();
		if (command == kEscapeCommand) {
			if (_voicePending) {
				stopAudio(_voiceHandle);
				_voicePending = false;
				_engine->getCursor()->setSelectionIndex(kDefaultCursor);
				_engine->getCursor()->dispatchSelectionIndexChange(kDefaultCursor);
				if (!serviceVoiceCompletion())
					return kFailureCommand;
				updateCursor(_engine->getInput()->peekMouseState().position);
				debugC(2, kDebugDialogue,
					"Ripper: Escape stopped active Cain dialogue voice");
				return MediaSequenceCallback::kContinueRefreshPalette;
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
	if ((mouse.pressed & kMouseButtonLeft) != 0 && _hoveredControl == 0)
		return kTalkCommand;
	return 0;
}

uint16 CainScene::service(uint frame) {
	if (_choiceListRebuildPending && _conversationStarted && !_voicePending)
		rebuildChoices();
	if (_voicePending && !_engine->getMedia()->isSoundEffectActive(_voiceHandle)) {
		_voicePending = false;
		_engine->getCursor()->setSelectionIndex(kDefaultCursor);
		_engine->getCursor()->dispatchSelectionIndexChange(kDefaultCursor);
		debugC(2, kDebugDialogue,
			"Ripper: Cain dialogue voice completed frame=%u", frame);
		if (!serviceVoiceCompletion())
			return kFailureCommand;
		updateCursor(_engine->getInput()->peekMouseState().position);
		presentDialogueOverlay(frame);
		return MediaSequenceCallback::kContinueRefreshPalette;
	}

	uint16 command = serviceInput();
	if (command == kTalkCommand) {
		const bool firstGreeting = !_conversationStarted;
		if (!startVoice(firstGreeting ? kFirstGreetingAudio : kRepeatGreetingAudio,
				"talk-control"))
			return kFailureCommand;
		if (firstGreeting && !_engine->getMilestones()->set(
				kConversationStartedFlag, true, "cain-dialogue-start"))
			return kFailureCommand;
		_conversationStarted = true;
		command = 0;
	} else if (command >= kChoiceCommandBase &&
			command < kChoiceCommandBase + 8) {
		const uint16 choiceId = command - kChoiceCommandBase;
		const Choice *choice = findAvailableChoice(choiceId);
		if (!choice || !_engine->getMilestones()->set(
				choice->flag, true, "cain-dialogue-choice") ||
				!startVoice(choice->audioPath, "choice"))
			return kFailureCommand;
		debugC(2, kDebugDialogue,
			"Ripper: selected Cain dialogue choice id=%u flag=0x%x textResource=0x%x audio='%s'",
			choiceId, choice->flag, choice->textResource, choice->audioPath);
		command = 0;
	}
	presentDialogueOverlay(frame);
	if (command != 0 && command != MediaSequenceCallback::kContinueRefreshPalette) {
		debugC(2, kDebugDialogue,
			"Ripper: Cain dialogue input command=0x%04x frame=%u choices=%u voicePending=%d",
			command, frame, _choices.size(), _voicePending);
	}
	return command;
}

void CainScene::stopAllAudio() {
	stopAudio(_loopAudioHandle);
	stopAudio(_voiceHandle);
}

CainScene::Result CainScene::run(uint sceneArgument) {
	if (!initialize())
		return kLoadFailed;
	prepare("cain-entry", kDefaultCursor, true);
	debugC(1, kDebugDialogue,
		"Ripper: entered Cain dialogue scene argument=%u loop='%s' audio='%s' toolbarMask=0x0000",
		sceneArgument, kLoopMedia, kLoopAudio);
	if (!_engine->getMedia()->playSoundEffect(kLoopAudio, _loopAudioHandle,
			kLoopAudioVolume, true)) {
		stopAllAudio();
		finish("cain-load-failure", -1, false);
		return kLoadFailed;
	}

	uint16 command = 0;
	const bool played = _engine->getMedia()->playPuzzleSequence(
		kLoopMedia, kLoopStartFrame, this, &command);
	Result result = (!played && !_engine->shouldQuit()) || command == kFailureCommand ?
		kLoadFailed : kExited;
	if (result == kExited && !finishConversation())
		result = kLoadFailed;

	stopAllAudio();
	finish("cain-exit", -1, false);
	debugC(result == kExited ? 1 : 2, kDebugDialogue,
		"Ripper: left Cain dialogue scene result=%d command=0x%04x "
		"conversationStarted=%d choices=%u quit=%d",
		result, command, _conversationStarted, _choices.size(), _engine->shouldQuit());
	return result;
}

} // End of namespace Ripper
