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

#include "ripper/script.h"

#include "ripper/briefing.h"
#include "ripper/dialogue.h"

#include "common/debug.h"
#include "common/ptr.h"
#include "common/serializer.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/util.h"

#include "ripper/detection.h"
#include "ripper/cursor.h"
#include "ripper/cyber.h"
#include "ripper/input.h"
#include "ripper/media.h"
#include "ripper/milestones.h"
#include "ripper/modal_dialog.h"
#include "ripper/puzzles/calculator.h"
#include "ripper/puzzles/crystal.h"
#include "ripper/puzzles/rolodex.h"
#include "ripper/puzzles/table_gate.h"
#include "ripper/resources.h"
#include "ripper/ripper.h"
#include "ripper/scenes/tube_scene.h"
#include "ripper/toolbar.h"
#include "ripper/world_map.h"

namespace Ripper {

// RunStartupFrontEndLoop at 0x10778 uses selection cursor 0xe for the white
// menu pointer. PollInteractionAndResolveSelection at 0x13c8d enters the same
// front-end action service for the top 50-pixel band of every scene frame.
static const uint kToolbarCursor = 14;
static const uint kDialogueCursor = 16;
static const uint16 kHelpCommand = 0x3b00;
static const uint16 kDialogueEnterCommand = 0x0d;
static const uint16 kDialogueUpCommand = 0x4800;
static const uint16 kDialogueDownCommand = 0x5000;
static const uint16 kCyberLeftCommand = 0x4b00;
static const uint16 kCyberRightCommand = 0x4d00;
static const uint16 kCyberChooseCommand = 0x0d;
static const uint16 kCyberEscapeCommand = 0x1b;
static const uint kGeneralHelpResource = 400;
static const uint kPromptHelpResource = 0x19b;
static const uint kCyberHelpResource = 0x1a4;
static const int kSceneInteractionOriginY = 50;

static const uint32 kScriptHeaderSize = 0xe9;
static const uint32 kFrameRecordSize = 0x22;
static const uint32 kInteractionRecordSize = 0x25;
static const byte kCallbackTerminator = 'c';
static const uint kDefaultPaletteFadeSteps = 9;
static const uint kSceneTextEntryMaximumLength = 60;
static const int kCallbackSuspendedForText = -5;

class ScriptManager::IdleMediaCallback : public MediaSequenceCallback {
public:
	IdleMediaCallback(ScriptManager *manager, CompiledScript &script,
			uint32 callbackOffset, uint targetFrame, uint initialFrame) :
			_manager(manager), _script(script), _callbackOffset(callbackOffset),
			_targetFrame(targetFrame), _nextFrame(initialFrame), _result(0),
			_resumeCommand(0), _requestSelector(0), _failureFrame(0),
			_started(false), _textPending(false), _serviced(false),
			_succeeded(false) {
	}

	uint16 service(uint frame) override {
		if (_serviced || (!_started && frame < _targetFrame))
			return 0;
		if (_textPending) {
			const ModalDialogManager::TextEntryResult entryResult =
				_manager->_engine->getModalDialog()->serviceTextEntry(_entered);
			if (entryResult == ModalDialogManager::kTextEntryPending)
				return 0;
			if (entryResult == ModalDialogManager::kTextEntryFailed) {
				_succeeded = false;
				_serviced = true;
				return 1;
			}
			_textPending = false;
			const bool matched = entryResult == ModalDialogManager::kTextEntryAccepted &&
				ScriptManager::textAnswersMatch(_entered, _expected);
			debugC(1, kDebugCyber,
				"Ripper: scene text request completed script='%s' selector=%u accepted=%d matched=%d length=%u failureFrame=%u",
				_script.getMemberName().c_str(), _requestSelector,
				entryResult == ModalDialogManager::kTextEntryAccepted, matched,
				_entered.size(), _failureFrame);
			if (!matched) {
				_nextFrame = _failureFrame;
				_result = -2;
				_succeeded = true;
				_serviced = true;
				return 1;
			}
			return execute(frame, _resumeCommand);
		}
		_started = true;
		return execute(frame, 0);
	}

	bool continueAfterEnd() const override { return _textPending; }
	bool ownsInput() const override { return _textPending; }

	bool beginTextRequest(const Common::String &prompt,
			const Common::String &expected, uint failureFrame,
			uint selector, uint resumeCommand) {
		_expected = expected;
		_failureFrame = failureFrame;
		_requestSelector = selector;
		_resumeCommand = resumeCommand;
		_entered.clear();
		_textPending = _manager->_engine->getModalDialog()->beginTextEntry(prompt,
			kSceneTextEntryMaximumLength, kCyberHelpResource,
			"scene-script-password");
		return _textPending;
	}

	bool serviced() const { return _serviced; }
	bool succeeded() const { return _succeeded; }
	int result() const { return _result; }
	uint nextFrame() const { return _nextFrame; }
	uint targetFrame() const { return _targetFrame; }

private:
	uint16 execute(uint frame, uint commandStart) {
		_manager->_sceneCallbackFrame = frame;
		_manager->_activeIdleMediaCallback = this;
		_succeeded = _manager->executeCallback(_script, _callbackOffset,
			_result, &_nextFrame, commandStart);
		_manager->_activeIdleMediaCallback = nullptr;
		_manager->_sceneCallbackFrame = 0;
		if (_succeeded && _result == kCallbackSuspendedForText)
			return 0;
		_serviced = true;
		return 1;
	}
	ScriptManager *_manager;
	CompiledScript &_script;
	uint32 _callbackOffset;
	uint _targetFrame;
	uint _nextFrame;
	int _result;
	uint _resumeCommand;
	uint _requestSelector;
	uint _failureFrame;
	Common::String _expected;
	Common::String _entered;
	bool _started;
	bool _textPending;
	bool _serviced;
	bool _succeeded;
};

static Common::String compiledScriptMemberName(const Common::String &target) {
	// RunSceneScriptLoop at 0x124e9 replaces everything from the first dot with
	// ".run" before opening the compiled member. Script sources therefore pass
	// names such as DK1.SCR even though SCRIPT.PL stores the DK1 runtime.
	return target.substr(0, target.findFirstOf('.')) + ".run";
}

static const char *scriptOpcodeName(ScriptOpcode opcode) {
	switch (opcode) {
	case kMilestoneCondition: return "milestone condition";
	case kScenePlayedCondition: return "scene played condition";
	case kDialogueChoiceCondition: return "dialogue choice condition";
	case kPreviousSceneCondition: return "previous scene condition";
	case kDefaultBranch: return "default branch";
	case kClearBranchFlag: return "clear branch state";
	case kSetMilestoneFlag: return "set milestone flag";
	case kClearMilestoneFlag: return "clear milestone flag";
	case kDisableInteraction: return "disable interaction";
	case kSetInteractionSelection: return "set interaction selection";
	case kResetInteractionSelection: return "reset interaction selection";
	case kShowTextPanel: return "show text panel";
	case kSelectFrame: return "select frame";
	case kAddDialogueChoice: return "add dialogue choice";
	case kAddConditionalDialogueChoice: return "add conditional dialogue choice";
	case kStartDialogue: return "start dialogue";
	case kDispatchSceneAction: return "dispatch scene action";
	case kRequestTextInput: return "request text input";
	case kPlayMedia: return "play media";
	case kPreviewMedia: return "preview media";
	case kFadePalette: return "fade palette";
	case kStartSceneRuntime: return "start scene runtime";
	case kSetNamedFlag: return "set named flag";
	case kLoadAudio: return "load audio slot";
	case kConfigureAudio: return "configure audio slot";
	case kClearAudio: return "clear audio slot";
	case kStopAudio: return "stop audio slot";
	case kSetAudioVolume: return "set audio slot volume";
	case kWaitForFrameCounter: return "wait for frame counter";
	default: return opcode <= kNoOp7 ? "no-op" : "unknown";
	}
}

static const char *sceneActionName(uint action) {
	switch (action) {
	case 0: return "no action";
	case 1: return "circuit chip placement puzzle";
	case 2: return "scene selection menu";
	case 3: return "unlock-gated selection menu";
	case 4: return "calculator puzzle";
	case 5: return "rolodex sequence puzzle";
	case 6: return "cyber menu transition";
	case 7: return "clock puzzle";
	case 8: return "KD shooting gallery";
	case 9: return "GC/CSH four-choice sequence puzzle";
	case 10: return "table gate lever puzzle";
	case 11: return "CD-in-book button sequence puzzle";
	case 12: return "board arrangement puzzle";
	case 13: return "blob shooter";
	case 14: return "EBZ2S unlock-gated action menu";
	case 15: return "mechini combat encounter";
	case 16: return "key group puzzle";
	case 17: return "date selection puzzle";
	case 18: return "KI skull maze puzzle";
	case 19: return "web grid shift puzzle";
	case 20: return "Horus word puzzle";
	case 21: return "six-digit code puzzle";
	case 22: return "shock lever puzzle";
	case 23: return "tarot card puzzle";
	case 24: return "tube switch scene";
	case 25: return "KK tile match puzzle";
	case 26: return "ratini combat encounter";
	case 27: return "atkini combat encounter";
	case 28: return "gym selector";
	case 29: return "crystal piece placement puzzle";
	case 30: return "set chooser template mode";
	case 31: return "no-op";
	case 32: return "clear active display";
	case 33: return "stained glass puzzle";
	case 34: return "keypad sequence puzzle";
	case 35: return "set UI selection index";
	case 36: return "update UI selection";
	case 37: return "set scene runtime value";
	case 38: return "board game";
	case 39: return "no-op";
	case 40: return "KA dialogue scene";
	case 41: return "KB scene script";
	case 42: return "KC or Wofford media scene";
	case 43: return "KD scene script";
	case 44: return "no-op";
	case 45: return "KF scene script";
	case 46: return "KG scene script";
	case 47: return "KH scene script";
	case 48: return "KI scene script";
	case 49: return "KJ scene script";
	case 50: return "KK scene script";
	case 51: return "KL scene script";
	case 52: return "KM scene script";
	case 53: return "KN scene script";
	case 54: return "KP scene script";
	case 55: return "KQ scene script";
	case 56: return "KR scene script";
	case 57:
	case 58:
	case 59: return "no-op";
	case 60: return "key group puzzle";
	case 61: return "eight-button sequence puzzle";
	case 62: return "Cain dialogue scene";
	case 63: return "append resource string to RIPPER.TXT";
	case 300: return "arm briefing media trigger";
	case 9999: return "terminate scene runtime";
	default: return "unknown";
	}
}

CompiledScript::CompiledScript() : _version(0), _argumentLayoutOffset(0),
		_argumentLayoutStride(0), _argumentLayoutCount(0) {
}

bool CompiledScript::canRead(uint32 offset, uint32 size) const {
	return offset <= _data.size() && size <= _data.size() - offset;
}

uint16 CompiledScript::readUint16(uint32 offset) const {
	return (uint16)_data[offset] | ((uint16)_data[offset + 1] << 8);
}

int16 CompiledScript::readSint16(uint32 offset) const {
	return (int16)readUint16(offset);
}

uint32 CompiledScript::readUint32(uint32 offset) const {
	return (uint32)_data[offset] |
		((uint32)_data[offset + 1] << 8) |
		((uint32)_data[offset + 2] << 16) |
		((uint32)_data[offset + 3] << 24);
}

Common::String CompiledScript::getString(uint32 offset) const {
	if (offset >= _data.size())
		return Common::String();

	Common::String value;
	while (offset < _data.size() && _data[offset] != 0)
		value += (char)_data[offset++];
	return value;
}

bool CompiledScript::load(AssetLibrary &library, const Common::String &memberName) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(library.createReadStreamForMember(memberName));
	if (!stream) {
		warning("Ripper: could not open compiled script '%s'", memberName.c_str());
		return false;
	}

	const int64 size64 = stream->size();
	if (size64 < kScriptHeaderSize || size64 > 0xffffffffLL) {
		warning("Ripper: invalid compiled script size %lld for '%s'", size64, memberName.c_str());
		return false;
	}

	_memberName = memberName;
	_data.resize((uint32)size64);
	if (stream->read(_data.data(), _data.size()) != _data.size()) {
		warning("Ripper: truncated compiled script '%s'", memberName.c_str());
		return false;
	}

	if (!parseHeader() || !validateCallbacks())
		return false;

	debugC(1, kDebugScripts,
		"Ripper: parsed script '%s' source='%s' version=%u.%02u frames=%u interactions=%u bytes=%u",
		_memberName.c_str(), _sourceName.c_str(), _version / 100, _version % 100,
		_frames.size(), _interactions.size(), _data.size());
	return true;
}

bool CompiledScript::parseHeader() {
	_version = readUint16(0);
	if (_version != 150) {
		warning("Ripper: script '%s' has unsupported compiled version %u.%02u",
			_memberName.c_str(), _version / 100, _version % 100);
		return false;
	}

	_sourceName = getString(0x82);
	_argumentLayoutOffset = readUint32(0xc2);
	_argumentLayoutStride = _data[0xc6];
	_argumentLayoutCount = readUint16(0xc7);
	const uint32 frameOffset = readUint32(0xdd);
	const uint16 frameCount = readUint16(0xe1);
	const uint32 interactionOffset = readUint32(0xe3);
	const uint16 interactionCount = readUint16(0xe7);

	if (_argumentLayoutStride == 0 ||
		!canRead(_argumentLayoutOffset, (uint32)_argumentLayoutStride * _argumentLayoutCount) ||
		!canRead(frameOffset, (uint32)frameCount * kFrameRecordSize) ||
		!canRead(interactionOffset, (uint32)interactionCount * kInteractionRecordSize)) {
		warning("Ripper: script '%s' has invalid table ranges", _memberName.c_str());
		return false;
	}

	_frames.clear();
	for (uint i = 0; i < frameCount; ++i) {
		const uint32 offset = frameOffset + i * kFrameRecordSize;
		ScriptFrame frame;
		frame.presentationType = _data[offset];
		frame.mediaNameOffset = readUint32(offset + 1);
		frame.labelOffset = readUint32(offset + 5);
		frame.textOffset = readUint32(offset + 9);
		frame.interactionCount = _data[offset + 0xd];
		frame.enterCallbackOffset = readUint32(offset + 0xe);
		frame.idleCallbackOffset = readUint32(offset + 0x12);
		frame.exitCallbackOffset = readUint32(offset + 0x16);
		frame.firstInteractionIndex = readUint16(offset + 0x1a);
		frame.x = readSint16(offset + 0x1c);
		frame.y = readSint16(offset + 0x1e);
		frame.initialSelection = readUint16(offset + 0x20);

		if ((frame.mediaNameOffset != 0 && frame.mediaNameOffset >= _data.size()) ||
			(frame.labelOffset != 0 && frame.labelOffset >= _data.size()) ||
			(frame.textOffset != 0 && frame.textOffset >= _data.size()) ||
			(uint32)frame.firstInteractionIndex + frame.interactionCount > interactionCount) {
			warning("Ripper: script '%s' has invalid frame %u", _memberName.c_str(), i);
			return false;
		}

		_frames.push_back(frame);
		debugC(3, kDebugScripts,
			"Ripper: script='%s' frame=%u type=%u media='%s' label='%s' interactions=%u enter=0x%x idle=0x%x exit=0x%x",
			_memberName.c_str(), i, frame.presentationType, getString(frame.mediaNameOffset).c_str(),
			getString(frame.labelOffset).c_str(), frame.interactionCount, frame.enterCallbackOffset,
			frame.idleCallbackOffset, frame.exitCallbackOffset);
	}

	_interactions.clear();
	for (uint i = 0; i < interactionCount; ++i) {
		const uint32 offset = interactionOffset + i * kInteractionRecordSize;
		ScriptInteraction interaction;
		interaction.label.clear();
		for (uint labelIndex = 0; labelIndex < 0xb && _data[offset + labelIndex] != 0; ++labelIndex)
			interaction.label += (char)_data[offset + labelIndex];
		interaction.x = readSint16(offset + 0xb);
		interaction.y = readSint16(offset + 0xd);
		interaction.width = readSint16(offset + 0xf);
		interaction.height = readSint16(offset + 0x11);
		interaction.keyboardCommand = readUint16(offset + 0x13);
		interaction.initialSelection = _data[offset + 0x15];
		interaction.conditionOffset = readUint32(offset + 0x16);
		interaction.callbackOffset = readUint32(offset + 0x1b);
		interaction.flags = _data[offset + 0x21];
		if (interaction.callbackOffset != 0 && interaction.callbackOffset >= _data.size()) {
			warning("Ripper: script '%s' has invalid interaction callback %u", _memberName.c_str(), i);
			return false;
		}
		_interactions.push_back(interaction);
	}

	return true;
}

bool CompiledScript::decodeCallback(uint32 offset, bool decodeText, Common::Array<ScriptCommand> &commands) const {
	commands.clear();
	if (offset == 0)
		return true;

	uint32 cursor = offset;
	while (cursor < _data.size()) {
		ScriptCommand command;
		command.offset = cursor;
		const byte opcode = _data[cursor++];
		if (opcode == kCallbackTerminator)
			return true;
		if (opcode >= _argumentLayoutCount || !canRead(cursor, 2)) {
			warning("Ripper: script '%s' has invalid opcode 0x%02x at 0x%x",
				_memberName.c_str(), opcode, command.offset);
			return false;
		}
		command.opcode = (ScriptOpcode)opcode;

		command.selector = readUint16(cursor);
		cursor += 2;
		const uint32 layout = _argumentLayoutOffset + opcode * _argumentLayoutStride;
		for (uint argumentIndex = 0; argumentIndex < _argumentLayoutStride; ++argumentIndex) {
			const byte type = _data[layout + argumentIndex];
			if (type == 0)
				break;

			ScriptArgument argument;
			argument.type = type;
			argument.value = 0;
			switch (type) {
			case 1:
			case 3:
				if (!canRead(cursor, 2))
					return false;
				argument.value = readUint16(cursor);
				cursor += 2;
				break;

			case 2:
			case 4:
			case 7: {
				if (!canRead(cursor, 2))
					return false;
				const uint16 length = readUint16(cursor);
				cursor += 2;
				if (!canRead(cursor, length))
					return false;
				argument.data.resize(length);
				for (uint dataIndex = 0; dataIndex < length; ++dataIndex) {
					byte value = _data[cursor + dataIndex];
					if (decodeText && type == 7 && value != 0 && value != '\n')
						value ^= 0xff;
					argument.data[dataIndex] = value;
				}
				cursor += length;
				break;
			}

			case 5:
				if (!canRead(cursor, 4))
					return false;
				argument.value = readUint32(cursor);
				cursor += 4;
				if (argument.value >= _data.size())
					return false;
				break;

			case 6:
			case 8:
			case 9:
				if (!canRead(cursor, 4))
					return false;
				argument.value = readUint32(cursor);
				cursor += 4;
				break;

			default:
				warning("Ripper: script '%s' uses unknown argument type %u for opcode 0x%02x at 0x%x",
					_memberName.c_str(), type, command.opcode, command.offset);
				return false;
			}
			command.arguments.push_back(argument);
		}

		debugC(3, kDebugScripts,
			"Ripper: script='%s' callback=0x%x opcode=0x%02x selector=%u arguments=%u next=0x%x",
			_memberName.c_str(), command.offset, command.opcode, command.selector,
			command.arguments.size(), cursor);
		commands.push_back(command);
	}

	warning("Ripper: unterminated callback at 0x%x in '%s'", offset, _memberName.c_str());
	return false;
}

bool CompiledScript::validateCallbacks() const {
	Common::Array<ScriptCommand> commands;
	for (uint i = 0; i < _frames.size(); ++i) {
		if (!decodeCallback(_frames[i].enterCallbackOffset, true, commands) ||
			!decodeCallback(_frames[i].idleCallbackOffset, true, commands) ||
			!decodeCallback(_frames[i].exitCallbackOffset, true, commands)) {
			warning("Ripper: invalid callback in frame %u of '%s'", i, _memberName.c_str());
			return false;
		}
	}
	for (uint i = 0; i < _interactions.size(); ++i) {
		if (!decodeCallback(_interactions[i].callbackOffset, true, commands)) {
			warning("Ripper: invalid callback in interaction %u of '%s'", i, _memberName.c_str());
			return false;
		}
	}
	return true;
}

ScriptManager::ScriptManager(RipperEngine *engine) : _engine(engine), _activeBa0Frame(0),
		_hoveredBa0Interaction(-1),
		_awaitingBa0Interaction(false), _resumeLoadedPresentation(false),
		_clearPreservedAudioOnTransition(false), _cyberActive(false),
		_cyberExitRequested(false), _cyberKeyboardCommand(0), _sceneCallbackFrame(0),
		_activeIdleMediaCallback(nullptr), _chooserTemplateMode(0) {
	_briefing = new BriefingManager(engine);
	_dialogue = new DialogueChooser();
}

bool ScriptManager::canSaveGame() const {
	return !_cyberActive && _awaitingBa0Interaction && _pendingSceneMember.empty() &&
		_activeBa0Frame < _ba0.getFrames().size();
}

void ScriptManager::requestCyberExit(const char *source) {
	if (!_cyberActive)
		return;
	_cyberExitRequested = true;
	debugC(1, kDebugCyber, "Ripper: Cyber nested runtime exit requested source=%s", source);
}

void ScriptManager::logRuntimeFailure(const char *reason) const {
	Common::String frameLabel;
	if (_activeBa0Frame < _ba0.getFrames().size())
		frameLabel = _ba0.getString(_ba0.getFrames()[_activeBa0Frame].labelOffset);
	warning("Ripper: %s activeScript='%s' frame=%u/%u label='%s' awaitingInteraction=%d "
		"concurrentScript='%s' concurrentEntry='%s' pendingScript='%s' pendingEntry='%s'",
		reason, _ba0.getMemberName().c_str(), _activeBa0Frame, _ba0.getFrames().size(),
		frameLabel.c_str(), _awaitingBa0Interaction,
		_concurrent.getMemberName().c_str(), _concurrentEntryLabel.c_str(),
		_pendingSceneMember.c_str(), _pendingSceneEntryLabel.c_str());
}

bool ScriptManager::syncGame(Common::Serializer &serializer) {
	Common::String ba0Member = serializer.isSaving() ? _ba0.getMemberName() : Common::String();
	Common::String concurrentMember = serializer.isSaving() ? _concurrent.getMemberName() : Common::String();
	Common::String concurrentEntry = serializer.isSaving() ? _concurrentEntryLabel : Common::String();
	Common::String previousFrame = serializer.isSaving() ? _previousBa0FrameLabel : Common::String();
	uint32 activeFrame = _activeBa0Frame;
	byte awaitingInteraction = _awaitingBa0Interaction ? 1 : 0;
	byte briefingArmed = _briefing->isArmed() ? 1 : 0;
	uint32 briefingSelector = _briefing->getSelector();

	serializer.syncString(ba0Member);
	serializer.syncAsUint32LE(activeFrame);
	serializer.syncString(previousFrame);
	serializer.syncString(concurrentMember);
	serializer.syncString(concurrentEntry);
	serializer.syncAsByte(awaitingInteraction);
	serializer.syncAsByte(briefingArmed);
	serializer.syncAsUint32LE(briefingSelector);
	if (serializer.isLoading() && (ba0Member.empty() || ba0Member.size() > 128 ||
			concurrentMember.size() > 128 || concurrentEntry.size() > 128 ||
			previousFrame.size() > 128))
		return false;

	if (!_engine->getMilestones()->syncGame(serializer))
		return false;

	uint32 playedSceneCount = _playedScenes.size();
	serializer.syncAsUint32LE(playedSceneCount);
	if (serializer.isLoading()) {
		if (playedSceneCount > 4096)
			return false;
		_playedScenes.clear();
		_playedScenes.resize(playedSceneCount);
	}
	for (uint i = 0; i < playedSceneCount; ++i) {
		serializer.syncString(_playedScenes[i]);
		if (serializer.isLoading() && _playedScenes[i].size() > 128)
			return false;
	}

	uint32 interactionCount = _activeBa0InteractionEnabled.size();
	serializer.syncAsUint32LE(interactionCount);
	if (serializer.isLoading()) {
		if (interactionCount > 255)
			return false;
		_activeBa0InteractionEnabled.clear();
		_activeBa0InteractionEnabled.resize(interactionCount);
	}
	for (uint i = 0; i < interactionCount; ++i) {
		byte enabled = _activeBa0InteractionEnabled[i] ? 1 : 0;
		serializer.syncAsByte(enabled);
		if (serializer.isLoading())
			_activeBa0InteractionEnabled[i] = enabled != 0;
	}

	if (!_dialogue->syncGame(serializer) || serializer.err())
		return false;
	if (serializer.isSaving())
		return true;

	CompiledScript restoredBa0;
	if (!restoredBa0.load(_engine->getResources()->scripts(), ba0Member))
		return false;
	if (activeFrame >= restoredBa0.getFrames().size() ||
			interactionCount != restoredBa0.getFrames()[activeFrame].interactionCount)
		return false;

	CompiledScript restoredConcurrent;
	if (!concurrentMember.empty() &&
			!restoredConcurrent.load(_engine->getResources()->scripts(), concurrentMember))
		return false;

	_ba0 = Common::move(restoredBa0);
	_concurrent = Common::move(restoredConcurrent);
	_concurrentEntryLabel = concurrentEntry;
	_previousBa0FrameLabel = previousFrame;
	_activeBa0Frame = activeFrame;
	_awaitingBa0Interaction = awaitingInteraction != 0;
	_briefing->restore(briefingArmed != 0, briefingSelector);
	_pendingSceneMember.clear();
	_pendingSceneEntryLabel.clear();
	_clearPreservedAudioOnTransition = false;
	_hoveredBa0Interaction = -1;
	_resumeLoadedPresentation = !_dialogue->isPending() &&
		_ba0.getFrames()[_activeBa0Frame].presentationType == 1;
	_engine->getToolbar()->leave();
	_engine->getCursor()->setVisible(false);
	debugC(1, kDebugSaveLoad,
		"Ripper: restored script state member='%s' frame=%u label='%s' concurrent='%s' "
		"entry='%s' flags=%u playedScenes=%u interactions=%u dialogue=%d",
		_ba0.getMemberName().c_str(), _activeBa0Frame,
		_ba0.getString(_ba0.getFrames()[_activeBa0Frame].labelOffset).c_str(),
		_concurrent.getMemberName().c_str(), _concurrentEntryLabel.c_str(),
		Milestones::kFlagCount, _playedScenes.size(),
		_activeBa0InteractionEnabled.size(), _dialogue->isPending());
	return true;
}

ScriptManager::~ScriptManager() {
	delete _dialogue;
	delete _briefing;
}

bool ScriptManager::hasActivePrompt() const {
	// DispatchFrontEndAction at 0x190b7 selects contextual help while
	// SceneRuntime+0x189 bit 0x20 is set. HandleSceneEntryChoiceListLifecycle
	// sets that bit for the choice list currently modeled by DialogueChooser.
	return _dialogue->isPending();
}

bool ScriptManager::showHelp(const char *source) {
	// PollInteractionAndResolveSelection at 0x13c8d and DispatchFrontEndAction
	// at 0x190b7 use the same prompt-state branch for F1 and toolbar Help.
	const bool promptActive = hasActivePrompt();
	const uint resourceId = _cyberActive ? kCyberHelpResource :
		(promptActive ? kPromptHelpResource : kGeneralHelpResource);
	debugC(1, kDebugScene,
		"Ripper: opening scene help source=%s resource=%u promptActive=%d cyberActive=%d",
		source, resourceId, promptActive, _cyberActive);
	_engine->getCursor()->setVisible(true);
	// PollInteractionAndResolveSelection at 0x13c8d sends Cyber help resource
	// 0x1a4 through RunModalTextDialog without replacing the active palette.
	// The MENUB pixels therefore retain the colors of the Cyber presentation.
	const ModalDialogManager::PaletteBehavior paletteBehavior = _cyberActive ?
		ModalDialogManager::kPreserveActivePalette :
		ModalDialogManager::kApplyModalPalette;
	return _engine->getModalDialog()->run(resourceId, true,
		ModalDialogManager::kMenubPresentation, paletteBehavior);
}

bool ScriptManager::initialize(ResourceManager &resources) {
	return _briefing->initialize(resources) &&
		_dialogue->initialize(resources) &&
		_startup.load(resources.scripts(), "ripper.run") &&
		_ba0.load(resources.scripts(), "ba0.run");
}

bool ScriptManager::openWorldMap() {
	Common::String targetScript;
	if (!_engine->getWorldMap()->run(targetScript))
		return false;
	if (!targetScript.empty()) {
		_pendingSceneMember = targetScript;
		_pendingSceneEntryLabel.clear();
		_clearPreservedAudioOnTransition = true;
		debugC(2, kDebugScene,
			"Ripper: queued world map scene transition target='%s'",
			_pendingSceneMember.c_str());
	}
	return true;
}

Common::String ScriptManager::argumentString(const ScriptArgument &argument) {
	Common::String value;
	for (uint i = 0; i < argument.data.size() && argument.data[i] != 0; ++i)
		value += (char)argument.data[i];
	return value;
}

bool ScriptManager::textAnswersMatch(const Common::String &entered,
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

bool ScriptManager::isScenePlayed(const Common::String &scene) const {
	for (uint i = 0; i < _playedScenes.size(); ++i) {
		if (_playedScenes[i].equalsIgnoreCase(scene))
			return true;
	}
	return false;
}

void ScriptManager::markScenePlayed(const Common::String &scene) {
	if (scene.empty() || isScenePlayed(scene))
		return;
	_playedScenes.push_back(scene);
	debugC(2, kDebugScene, "Ripper: marked scene played '%s'", scene.c_str());
}

bool ScriptManager::findFrameByLabel(const CompiledScript &script, const Common::String &label,
		uint &frameIndex) const {
	for (uint i = 0; i < script.getFrames().size(); ++i) {
		if (script.getString(script.getFrames()[i].labelOffset).equalsIgnoreCase(label)) {
			frameIndex = i;
			return true;
		}
	}
	return false;
}

void ScriptManager::beginBa0InteractionWait(const Common::String &frameLabel,
		uint interactionCount) {
	// ExecuteSceneFrameAndInteractions at 0x13277 services the new controls once after
	// frame media, discarding that result before it starts the blocking chooser loop.
	if (_engine->getInput()->pollEvents())
		_engine->quitGame();
	_engine->getInput()->discardMouseTransitions();
	_awaitingBa0Interaction = true;
	debugC(1, kDebugScene,
		"Ripper: active scene script='%s' frame='%s' awaiting %u interactions",
		_ba0.getMemberName().c_str(), frameLabel.c_str(), interactionCount);
}

void ScriptManager::initializeBa0InteractionState(const ScriptFrame &frame) {
	_activeBa0InteractionEnabled.clear();
	_activeBa0InteractionEnabled.resize(frame.interactionCount);
	for (uint i = 0; i < frame.interactionCount; ++i) {
		const ScriptInteraction &interaction =
			_ba0.getInteractions()[frame.firstInteractionIndex + i];
		_activeBa0InteractionEnabled[i] = (interaction.flags & 2) == 0;
		const Common::Rect bounds = interactionBounds(interaction);
		debugC(3, kDebugScene,
			"Ripper: active scene interaction proxy script='%s' frame=%u interaction=%u "
			"label='%s' raw=%d,%d,%d,%d screen=%d,%d,%d,%d key=0x%04x cursor=%u enabled=%d",
			_ba0.getMemberName().c_str(), _activeBa0Frame,
			frame.firstInteractionIndex + i, interaction.label.c_str(),
			interaction.x, interaction.y, interaction.width, interaction.height,
			bounds.left, bounds.top, bounds.width(), bounds.height(),
			interaction.keyboardCommand, interaction.initialSelection,
			_activeBa0InteractionEnabled[i]);
	}
	debugC(3, kDebugScene,
		"Ripper: initialized active scene interaction proxies script='%s' frame=%u count=%u",
		_ba0.getMemberName().c_str(), _activeBa0Frame, frame.interactionCount);
}

void ScriptManager::bindBa0Frame(uint frameIndex) {
	// BindSceneRuntimeCurrentFrame at 0x145d6 preserves the outgoing frame label
	// at SceneRuntime+0x177 before changing the current frame record.
	if (_activeBa0Frame < _ba0.getFrames().size())
		_previousBa0FrameLabel = _ba0.getString(_ba0.getFrames()[_activeBa0Frame].labelOffset);
	else
		_previousBa0FrameLabel.clear();
	_activeBa0Frame = frameIndex;
	debugC(2, kDebugScripts,
		"Ripper: bound active scene script='%s' frame=%u previous='%s'",
		_ba0.getMemberName().c_str(), _activeBa0Frame, _previousBa0FrameLabel.c_str());
}

bool ScriptManager::executeCallback(CompiledScript &script, uint32 callbackOffset, int &result,
		uint *nextFrame, uint commandStart) {
	Common::Array<ScriptCommand> commands;
	result = 0;
	if (nextFrame)
		*nextFrame = 0;
	if (!script.decodeCallback(callbackOffset, true, commands))
		return false;
	debugC(2, kDebugScripts,
		"Ripper: decoded callback script='%s' offset=0x%x commands=%u",
		script.getMemberName().c_str(), callbackOffset, commands.size());

	uint commandIndex = commandStart;
	bool branchTaken = false;
	while (commandIndex < commands.size()) {
		const ScriptCommand &command = commands[commandIndex];
		debugC(2, kDebugScripts,
			"Ripper: execute script='%s' offset=0x%x opcode=0x%02x name='%s' selector=%u arguments=%u",
			script.getMemberName().c_str(), command.offset, command.opcode,
			scriptOpcodeName(command.opcode), command.selector, command.arguments.size());

		switch (command.opcode) {
		case kMilestoneCondition:
		case kScenePlayedCondition: {
			if (command.arguments.size() < 3)
				return false;
			const bool expected = command.arguments[0].value != 0;
			bool actual = false;
			if (command.opcode == kMilestoneCondition) {
				const uint flag = command.arguments[1].value;
				actual = _engine->getMilestones()->isSet(flag);
				debugC(3, kDebugMilestones,
					"Ripper: milestone gate flag=%u label='%s' domain='%s' expected=%d actual=%d "
					"target=0x%x script='%s' offset=0x%x",
					flag, _engine->getMilestones()->label(flag).c_str(), Milestones::domain(flag),
					expected, actual,
					command.arguments[2].value, script.getMemberName().c_str(), command.offset);
			} else {
				const Common::String scene = argumentString(command.arguments[1]);
				actual = isScenePlayed(scene);
				debugC(3, kDebugScripts,
					"Ripper: played condition scene='%s' expected=%d actual=%d target=0x%x",
					scene.c_str(), expected, actual, command.arguments[2].value);
			}
			if (actual != expected) {
				branchTaken = true;
				uint targetIndex = 0;
				while (targetIndex < commands.size() &&
					commands[targetIndex].offset != command.arguments[2].value)
					++targetIndex;
				if (targetIndex == commands.size()) {
					warning("Ripper: callback branch target 0x%x is not a command boundary in '%s'",
						command.arguments[2].value, script.getMemberName().c_str());
					return false;
				}
				commandIndex = targetIndex;
				continue;
			}
			break;
		}

		case kDialogueChoiceCondition: {
			if (command.arguments.size() < 2)
				return false;
			const bool choicesAvailable = _dialogue->hasChoices();
			const bool takeBranch = command.arguments[0].value == 0 ?
				choicesAvailable : !choicesAvailable;
			debugC(2, kDebugDialogue,
				"Ripper: dialogue availability condition choices=%u invert=%u target=0x%x selected=%d",
				choicesAvailable, command.arguments[0].value != 0,
				command.arguments[1].value, takeBranch);
			if (takeBranch) {
				branchTaken = true;
				uint targetIndex = 0;
				while (targetIndex < commands.size() &&
					commands[targetIndex].offset != command.arguments[1].value)
					++targetIndex;
				if (targetIndex == commands.size()) {
					warning("Ripper: dialogue branch target 0x%x is not a command boundary in '%s'",
						command.arguments[1].value, script.getMemberName().c_str());
					return false;
				}
				commandIndex = targetIndex;
				continue;
			}
			break;
		}

		case kPreviousSceneCondition: {
			if (command.arguments.size() < 3 ||
				command.arguments[1].value >= script.getFrames().size())
				return false;
			if (&script != &_ba0) {
				warning("Ripper: previous-scene condition has no tracked runtime in '%s' at 0x%x",
					script.getMemberName().c_str(), command.offset);
				return false;
			}

			const bool expected = command.arguments[0].value != 0;
			const ScriptFrame &comparedFrame = script.getFrames()[command.arguments[1].value];
			const Common::String comparedLabel = script.getString(comparedFrame.labelOffset);
			const bool actual = !_previousBa0FrameLabel.empty() &&
				_previousBa0FrameLabel.equalsIgnoreCase(comparedLabel);
			debugC(3, kDebugScripts,
				"Ripper: previous-scene condition frame=%u label='%s' previous='%s' "
				"expected=%d actual=%d target=0x%x",
				command.arguments[1].value, comparedLabel.c_str(),
				_previousBa0FrameLabel.c_str(), expected, actual,
				command.arguments[2].value);
			if (actual != expected) {
				branchTaken = true;
				uint targetIndex = 0;
				while (targetIndex < commands.size() &&
					commands[targetIndex].offset != command.arguments[2].value)
					++targetIndex;
				if (targetIndex == commands.size()) {
					warning("Ripper: callback branch target 0x%x is not a command boundary in '%s'",
						command.arguments[2].value, script.getMemberName().c_str());
					return false;
				}
				commandIndex = targetIndex;
				continue;
			}
			break;
		}

		case kDefaultBranch: {
			if (command.arguments.size() < 1)
				return false;
			const bool useDefaultBranch = !branchTaken;
			branchTaken = false;
			debugC(3, kDebugScripts, "Ripper: default branch target=0x%x selected=%d",
				command.arguments[0].value, useDefaultBranch);
			if (useDefaultBranch) {
				uint targetIndex = 0;
				while (targetIndex < commands.size() &&
					commands[targetIndex].offset != command.arguments[0].value)
					++targetIndex;
				if (targetIndex == commands.size()) {
					warning("Ripper: callback branch target 0x%x is not a command boundary in '%s'",
						command.arguments[0].value, script.getMemberName().c_str());
					return false;
				}
				commandIndex = targetIndex;
				continue;
			}
			break;
		}

		case kClearBranchFlag:
			branchTaken = false;
			debugC(3, kDebugScripts, "Ripper: cleared callback branch state");
			break;

		case kSetMilestoneFlag:
		case kClearMilestoneFlag: {
			if (command.arguments.size() < 1)
				return false;
			const bool value = command.opcode == kSetMilestoneFlag;
			if (!_engine->getMilestones()->set(command.arguments[0].value, value,
				value ? "set milestone flag" : "clear milestone flag"))
				return false;
			break;
		}

		case kDisableInteraction: {
			if (command.arguments.size() < 1)
				return false;
			if (&script != &_ba0 || _activeBa0Frame >= _ba0.getFrames().size()) {
				warning("Ripper: interaction disable opcode has no active scene frame in '%s' at 0x%x",
					script.getMemberName().c_str(), command.offset);
				return false;
			}
			const uint relativeIndex = command.arguments[0].value & 0xffff;
			const ScriptFrame &frame = _ba0.getFrames()[_activeBa0Frame];
			if (relativeIndex < _activeBa0InteractionEnabled.size()) {
				const uint interactionIndex = frame.firstInteractionIndex + relativeIndex;
				const ScriptInteraction &interaction = _ba0.getInteractions()[interactionIndex];
				_activeBa0InteractionEnabled[relativeIndex] = false;
				debugC(2, kDebugScene,
					"Ripper: disabled frame interaction relative=%u global=%u label='%s'",
					relativeIndex, interactionIndex, interaction.label.c_str());
			} else {
				debugC(2, kDebugScene,
					"Ripper: ignored out-of-range interaction disable relative=%u count=%u",
					relativeIndex, frame.interactionCount);
			}
			break;
		}

		case kPlayMedia: {
			if (command.arguments.size() < 5)
				return false;
			const Common::String mediaPath = script.getString(command.arguments[0].value);
			const bool allowEscSpace = command.arguments[2].value == 0;
			// ExecutePresentationEntry at 0x1652a deactivates the UI selection
			// presentation before media playback. The following frame activation
			// restores the cursor after the callback finishes.
			_engine->getCursor()->setVisible(false);
			if (!_engine->getMedia()->play(mediaPath, allowEscSpace,
				(int32)command.arguments[3].value, (int32)command.arguments[4].value, true))
				return false;
			// HandleSceneEntryMediaAndSetBasenameFlag at 0x159e1 marks the
			// presentation basename only after ExecutePresentationEntry returns.
			const size_t extension = mediaPath.findFirstOf('.');
			const Common::String playedKey = mediaPath.substr(0, extension);
			markScenePlayed(playedKey);
			debugC(2, kDebugScripts,
				"Ripper: completed media command '%s' controls=%d playedKey='%s'",
				mediaPath.c_str(), allowEscSpace, playedKey.c_str());
			break;
		}

		case kAddDialogueChoice:
		case kAddConditionalDialogueChoice:
		case kStartDialogue: {
			bool includeChoice = true;
			if (command.opcode == kAddConditionalDialogueChoice) {
				if (command.arguments.size() < 2 ||
					command.arguments[1].value >= _ba0.getFrames().size())
					return false;
				const ScriptFrame &responseFrame = _ba0.getFrames()[command.arguments[1].value];
				const Common::String responseLabel = _ba0.getString(responseFrame.labelOffset);
				includeChoice = !isScenePlayed(responseLabel);
				debugC(2, kDebugDialogue,
					"Ripper: dialogue response frame=%u label='%s' played=%d",
					command.arguments[1].value, responseLabel.c_str(), !includeChoice);
			}
			if (!_dialogue->execute(script, command, includeChoice))
				return false;
			if (command.opcode == kStartDialogue)
				debugC(2, kDebugAudio,
					"Ripper: dialogue chooser retained scene audio active=%d",
					_engine->getMedia()->isSceneAudioActive());
			break;
		}

		case kStartSceneRuntime: {
			if (command.arguments.size() < 3)
				return false;
			const Common::String target = argumentString(command.arguments[0]);
			const Common::String entryLabel = argumentString(command.arguments[1]);
			// HandleSceneEntryAndStartConcurrentSceneRuntime at 0x15cd3 always
			// turns opcode 0x1d into a scene handoff when the command belongs to
			// the concurrent runtime. The third argument only selects this path
			// when the active scene issues the command.
			if (&script == &_concurrent || command.arguments[2].value == 0) {
				_pendingSceneMember = compiledScriptMemberName(target);
				_pendingSceneEntryLabel = entryLabel;
				if (&script == &_concurrent)
					_clearPreservedAudioOnTransition = true;
				debugC(1, kDebugScene,
					"Ripper: queued script transition target='%s' entry='%s' source='%s'",
					_pendingSceneMember.c_str(), _pendingSceneEntryLabel.c_str(),
					script.getMemberName().c_str());
				result = -3;
				return true;
			}
			const Common::String memberName = compiledScriptMemberName(target);
			if (!_concurrent.load(_engine->getResources()->scripts(), memberName))
				return false;
			_concurrentEntryLabel = entryLabel;
			debugC(1, kDebugScene, "Ripper: created concurrent script='%s' entry='%s'",
				target.c_str(), entryLabel.c_str());
			break;
		}

		case kLoadAudio: {
			if (command.arguments.size() < 2)
				return false;
			const Common::String audioPath = script.getString(command.arguments[0].value);
			const bool preserve = (command.arguments[1].value & 1) != 0;
			if (!_engine->getMedia()->loadAudio(audioPath, preserve))
				return false;
			break;
		}

		case kConfigureAudio: {
			if (command.arguments.size() < 4)
				return false;
			const Common::String key = argumentString(command.arguments[0]);
			const uint volume = command.arguments[1].value == 0 ? 100 : command.arguments[1].value & 0xff;
			const uint trigger = command.arguments[2].value & 0xffff;
			const uint control = command.arguments[3].value & 0xff;
			if (!_engine->getMedia()->configureAudio(key, volume, trigger, control))
				return false;
			break;
		}

		case kClearAudio:
		case kStopAudio: {
			if (command.arguments.size() < 1)
				return false;
			const Common::String key = argumentString(command.arguments[0]);
			if (command.opcode == kClearAudio)
				_engine->getMedia()->clearAudio(key);
			else
				_engine->getMedia()->stopAudio(key);
			break;
		}

		case kSetAudioVolume: {
			if (command.arguments.size() < 4)
				return false;
			const Common::String key = argumentString(command.arguments[0]);
			_engine->getMedia()->setAudioVolume(key,
				command.arguments[1].value & 0xff,
				command.arguments[2].value & 0xffff,
				command.arguments[3].value & 0xffff);
			break;
		}

		case kWaitForFrameCounter: {
			if (command.arguments.size() < 1)
				return false;
			const uint targetFrame = command.arguments[0].value;
			// HandleSceneEntryWaitForSceneFrameCounter at 0x1633e suspends the
			// idle command stream until RunMediaSequence publishes this one-based
			// frame. IdleMediaCallback enters the stream only after that boundary.
			if (_sceneCallbackFrame < targetFrame) {
				warning("Ripper: scene callback frame=%u did not reach wait target=%u script='%s' offset=0x%x",
					_sceneCallbackFrame, targetFrame, script.getMemberName().c_str(),
					command.offset);
				return false;
			}
			debugC(2, kDebugScene,
				"Ripper: scene idle callback reached media frame=%u target=%u script='%s' offset=0x%x",
				_sceneCallbackFrame, targetFrame, script.getMemberName().c_str(),
				command.offset);
			break;
		}

		case kRequestTextInput: {
			if (command.arguments.size() < 4 || !nextFrame ||
					!_activeIdleMediaCallback)
				return false;
			const Common::String prompt = argumentString(command.arguments[0]);
			const Common::String expected = argumentString(command.arguments[1]);
			const uint failureFrame = command.arguments[2].value;
			const uint layoutVariant = command.arguments[3].value;
			if (layoutVariant != 2) {
				warning("Ripper: unsupported scene text request layout=%u script='%s' offset=0x%x",
					layoutVariant, script.getMemberName().c_str(), command.offset);
				return false;
			}

			// DispatchSceneEntryAction 30 captures the active presentation as the
			// following chooser's one-shot template. The original Cyber scripts
			// pass zero, which maps to drawing over ScummVM's active framebuffer.
			const byte templateMode = _chooserTemplateMode;
			_chooserTemplateMode = 0;
			if (!_activeIdleMediaCallback->beginTextRequest(prompt, expected,
					failureFrame, command.selector, commandIndex + 1))
				return false;
			debugC(2, kDebugCyber,
				"Ripper: suspended scene callback for text request script='%s' selector=%u layout=%u templateMode=%u resumeCommand=%u",
				script.getMemberName().c_str(), command.selector, layoutVariant,
				templateMode, commandIndex + 1);
			result = kCallbackSuspendedForText;
			return true;
		}

		case kPreviewMedia: {
			if (command.arguments.size() < 4)
				return false;
			const Common::String mediaPath = script.getString(command.arguments[0].value);
			if (!_engine->getMedia()->playScene(mediaPath, (int32)command.arguments[1].value,
				(int32)command.arguments[2].value, true))
				return false;
			debugC(2, kDebugScripts,
				"Ripper: retained first frame for scene media command '%s'",
				mediaPath.c_str());
			break;
		}

		case kSelectFrame:
			if (command.arguments.size() < 1)
				return false;
			if (nextFrame)
				*nextFrame = command.arguments[0].value;
			result = -2;
			debugC(2, kDebugScripts, "Ripper: callback selected frame=%u control=%d",
				command.arguments[0].value, result);
			return true;

		case kDispatchSceneAction: {
			if (command.arguments.size() < 2)
				return false;
			const uint action = command.arguments[0].value;
			const uint argument = command.arguments[1].value;
			debugC(2, kDebugScene,
				"Ripper: dispatch scene action=%u name='%s' argument=%u script='%s' offset=0x%x",
				action, sceneActionName(action), argument,
				script.getMemberName().c_str(), command.offset);
			if (action == kSceneActionWorldMap) {
				if (!openWorldMap())
					return false;
				if (!_pendingSceneMember.empty()) {
					result = -3;
					return true;
				}
				break;
			}
			if (action == kSceneActionCalculatorPuzzle) {
				CalculatorPuzzle puzzle(_engine);
				const CalculatorPuzzle::Result puzzleResult = puzzle.run(argument);
				debugC(1, kDebugPuzzles,
					"Ripper: calculator puzzle scene action completed result=%d milestone=%u",
					puzzleResult, argument);
				if (puzzleResult == CalculatorPuzzle::kLoadFailed)
					return false;
				break;
			}
			if (action == kSceneActionRolodexPuzzle) {
				RolodexPuzzle puzzle(_engine);
				const RolodexPuzzle::Result puzzleResult = puzzle.run(argument);
				debugC(1, kDebugPuzzles,
					"Ripper: rolodex puzzle scene action completed result=%d milestone=%u",
					puzzleResult, argument);
				if (puzzleResult == RolodexPuzzle::kLoadFailed)
					return false;
				break;
			}
			if (action == kSceneActionCyberMenu) {
				const CyberManager::Result cyberResult = _engine->getCyber()->run();
				debugC(1, kDebugCyber,
					"Ripper: Cyber menu scene action completed result=%d",
					cyberResult);
				if (cyberResult == CyberManager::kLoadFailed)
					return false;
				break;
			}
			if (action == kSceneActionTableGatePuzzle) {
				TableGatePuzzle puzzle(_engine);
				const TableGatePuzzle::Result puzzleResult = puzzle.run(argument);
				debugC(1, kDebugPuzzles,
					"Ripper: table gate puzzle scene action completed result=%d milestone=%u",
					puzzleResult, argument);
				if (puzzleResult == TableGatePuzzle::kLoadFailed)
					return false;
				break;
			}
			if (action == kSceneActionTubeSwitchScene) {
				TubeScene scene(_engine);
				const TubeScene::Result sceneResult = scene.run(argument);
				debugC(1, kDebugScene,
					"Ripper: tube switch scene action completed result=%d milestone=%u",
					sceneResult, argument);
				if (sceneResult == TubeScene::kLoadFailed)
					return false;
				break;
			}
			if (action == kSceneActionCrystalPuzzle) {
				CrystalPuzzle puzzle(_engine);
				const CrystalPuzzle::Result puzzleResult = puzzle.run(argument);
				debugC(1, kDebugPuzzles,
					"Ripper: crystal puzzle scene action completed result=%d milestone=%u",
					puzzleResult, argument);
				if (puzzleResult == CrystalPuzzle::kLoadFailed)
					return false;
				break;
			}
			if (action == kSceneActionSetChooserTemplateMode) {
				// DispatchSceneEntryAction at 0x36892 captures the active media
				// presentation for the following chooser. ScummVM presents that media
				// on one framebuffer, represented by template mode zero. The script
				// argument is not consumed by the original handler.
				_chooserTemplateMode = 0;
				debugC(2, kDebugScene,
					"Ripper: scene action 30 captured active presentation templateMode=%u script='%s' offset=0x%x",
					_chooserTemplateMode, script.getMemberName().c_str(), command.offset);
				break;
			}
			if (action == kSceneActionNoOp) {
				// DispatchSceneEntryAction at 0x36892 has an explicit action-31
				// branch which returns without changing the active runtime.
				debugC(3, kDebugCyber,
					"Ripper: Cyber scene action 31 completed as original no-op");
				break;
			}
			if (action == kSceneActionClearDisplay) {
				// DispatchSceneEntryAction at 0x36892 deactivates the selection
				// presentation and sends display command 0x14, whose table entry is
				// ClearGenericVideoLogicalPage at 0x45ed8, before reactivating it.
				g_system->fillScreen(0);
				g_system->updateScreen();
				debugC(2, kDebugScene,
					"Ripper: cleared active scene display from scene action 32");
				break;
			}
			if (action == kSceneActionSetUiSelectionIndex) {
				_engine->getCursor()->setSelectionIndex(argument);
				debugC(2, kDebugCursor,
					"Ripper: scene action 35 stored UI selection index=%u script='%s' offset=0x%x",
					argument, script.getMemberName().c_str(), command.offset);
				break;
			}
			if (action == kSceneActionDispatchUiSelection) {
				_engine->getCursor()->dispatchSelectionIndexChange(argument);
				debugC(2, kDebugCursor,
					"Ripper: scene action 36 dispatched UI selection index=%u script='%s' offset=0x%x",
					argument, script.getMemberName().c_str(), command.offset);
				break;
			}
			if (action == kSceneActionKaDialogue) {
				// DispatchKSceneActionBand at 0x36e84 preserves the Cyber menu
				// runtime around RunKaDialogueScene at 0x2aef5, just as it does
				// around the sibling K-scene script loops.
				debugC(1, kDebugCyber,
					"Ripper: dispatching Cyber dialogue action=%u name='%s' argument=%u activeScript='%s' frame=%u",
					action, sceneActionName(action), argument,
					_ba0.getMemberName().c_str(), _activeBa0Frame);
				const CyberManager::Result cyberResult =
					_engine->getCyber()->runProgram(action, "ka", argument);
				debugC(cyberResult == CyberManager::kExited ? 1 : 2, kDebugCyber,
					"Ripper: Cyber dialogue action=%u completed result=%d restoredScript='%s' frame=%u",
					action, cyberResult, _ba0.getMemberName().c_str(), _activeBa0Frame);
				if (cyberResult == CyberManager::kLoadFailed)
					return false;
				break;
			}
			if (action >= kSceneActionKbProgram && action <= kSceneActionKrProgram &&
					action != 44) {
				const char *program = nullptr;
				uint chapter = 0;
				for (uint flag = kMilestoneCompletedAct3; flag != 0; --flag) {
					if (_engine->getMilestones()->isSet(flag)) {
						chapter = flag;
						break;
					}
				}
				switch (action) {
				case kSceneActionKbProgram: program = "kb"; break;
				case kSceneActionKcOrWoffordProgram:
					if (chapter < 3) {
						warning("Ripper: Cyber action 42 requires the Wofford interactive media path before chapter 3");
						return false;
					}
					program = "kc";
					break;
				case kSceneActionKdProgram: program = "kd"; break;
				case kSceneActionKfProgram: program = "kf"; break;
				case kSceneActionKgProgram:
					program = chapter >= 3 ? "kg3" : "kg";
					break;
				case kSceneActionKhProgram: program = "kh"; break;
				case kSceneActionKiProgram: program = "ki"; break;
				case kSceneActionKjProgram:
					program = chapter >= 3 ? "kj3" : "kj";
					break;
				case kSceneActionKkProgram: program = "kk"; break;
				case kSceneActionKlProgram: program = "kl"; break;
				case kSceneActionKmProgram: program = "km"; break;
				case kSceneActionKnProgram: program = "kn"; break;
				case kSceneActionKpProgram: program = "kp"; break;
				case kSceneActionKqProgram: program = "kq"; break;
				case kSceneActionKrProgram: program = "kr"; break;
				default: break;
				}
				if (!program)
					return false;
				// DispatchKSceneActionBand at 0x36e84 preserves the Cyber menu's
				// palette, UI controls, chooser registry, and audio table around
				// each sibling K scene-script loop.
				debugC(1, kDebugCyber,
					"Ripper: dispatching Cyber program action=%u name='%s' script='%s.run' argument=%u activeScript='%s' frame=%u",
					action, sceneActionName(action), program, argument,
					_ba0.getMemberName().c_str(), _activeBa0Frame);
				const CyberManager::Result cyberResult =
					_engine->getCyber()->runProgram(action, program, argument);
				debugC(cyberResult == CyberManager::kExited ? 1 : 2, kDebugCyber,
					"Ripper: Cyber program action=%u script='%s.run' completed result=%d restoredScript='%s' frame=%u",
					action, program, cyberResult, _ba0.getMemberName().c_str(),
					_activeBa0Frame);
				if (cyberResult == CyberManager::kLoadFailed)
					return false;
				break;
			}
			if (action == 44) {
				debugC(3, kDebugCyber,
					"Ripper: Cyber scene action 44 completed as original no-op");
				break;
			}
			if (action == kSceneActionTerminateRuntime) {
				if (!_cyberActive)
					return false;
				requestCyberExit("scene-action-9999");
				result = -4;
				return true;
			}
			if (action != kSceneActionBriefing) {
				warning("Ripper: unsupported scene action %u ('%s') in '%s' at 0x%x",
					action, sceneActionName(action), script.getMemberName().c_str(), command.offset);
				return false;
			}
			if (!_briefing->arm(argument))
				return false;
			break;
		}

		case kFadePalette: {
			if (command.arguments.size() < 2)
				return false;
			const bool fadeIn = command.arguments[0].value != 0;
			const int32 configuredSteps = (int32)command.arguments[1].value;
			const uint stepCount = configuredSteps > 0 ? configuredSteps : kDefaultPaletteFadeSteps;
			debugC(2, kDebugVideo, "Ripper: fading palette %s steps=%u",
				fadeIn ? "in" : "out", stepCount);
			debugC(3, kDebugScripts,
				"Ripper: palette transition script='%s' offset=0x%x mode=%u configuredSteps=%d effectiveSteps=%u",
				script.getMemberName().c_str(), command.offset, command.arguments[0].value,
				configuredSteps, stepCount);
			_engine->getMedia()->fadePalette(fadeIn, stepCount);
			break;
		}

		default:
			warning("Ripper: unsupported opcode 0x%02x in '%s' at 0x%x",
				command.opcode, script.getMemberName().c_str(), command.offset);
			return false;
		}
		++commandIndex;
	}
	return true;
}

bool ScriptManager::runStartupPath() {
	if (_startup.getFrames().empty() || _ba0.getFrames().empty())
		return false;

	int result = 0;
	const ScriptFrame &startupFrame = _startup.getFrames()[0];
	if (!executeCallback(_startup, startupFrame.exitCallbackOffset, result) || result != -3) {
		warning("Ripper: startup script did not request the BA0 transition (result=%d)", result);
		return false;
	}

	uint ba0StartFrame = 0xffffffff;
	for (uint i = 0; i < _ba0.getFrames().size(); ++i) {
		if (_ba0.getString(_ba0.getFrames()[i].labelOffset).equalsIgnoreCase("start")) {
			ba0StartFrame = i;
			break;
		}
	}
	if (ba0StartFrame == 0xffffffff) {
		warning("Ripper: BA0 does not contain the start frame");
		return false;
	}

	// InitializeSceneDisplayModeAndContext at 0x1e28a switches to a newly cleared scene page.
	// ScummVM retains one framebuffer across presentations, so clear the previous cinematic here.
	g_system->fillScreen(0);
	g_system->updateScreen();
	debugC(1, kDebugScene, "Ripper: initialized cleared scene display for BA0");

	debugC(1, kDebugScene, "Ripper: entering BA0 frame=%u label='start'", ba0StartFrame);
	bindBa0Frame(ba0StartFrame);
	initializeBa0InteractionState(_ba0.getFrames()[ba0StartFrame]);
	result = 0;
	if (!executeCallback(_ba0, _ba0.getFrames()[ba0StartFrame].enterCallbackOffset, result) || result != 0)
		return false;
	beginBa0InteractionWait("start", _ba0.getFrames()[ba0StartFrame].interactionCount);
	return true;
}

bool ScriptManager::executeConcurrentFrame() {
	if (_concurrent.getFrames().empty())
		return true;

	uint frameIndex = 0;
	if (!findFrameByLabel(_concurrent, _concurrentEntryLabel, frameIndex)) {
		warning("Ripper: concurrent script '%s' has no frame '%s'",
			_concurrent.getMemberName().c_str(), _concurrentEntryLabel.c_str());
		return false;
	}

	const ScriptFrame &frame = _concurrent.getFrames()[frameIndex];
	debugC(2, kDebugScene, "Ripper: servicing concurrent script='%s' frame=%u label='%s'",
		_concurrent.getMemberName().c_str(), frameIndex, _concurrentEntryLabel.c_str());
	int result = 0;
	uint nextFrame = frameIndex;
	if (!executeCallback(_concurrent, frame.enterCallbackOffset, result, &nextFrame))
		return false;
	if (result == -3 && !_pendingSceneMember.empty()) {
		debugC(1, kDebugScene,
			"Ripper: concurrent script requested transition target='%s' entry='%s'",
			_pendingSceneMember.c_str(), _pendingSceneEntryLabel.c_str());
		_concurrent = CompiledScript();
		_concurrentEntryLabel.clear();
		return true;
	}
	if (result != 0)
		return false;
	if (!executeCallback(_concurrent, frame.exitCallbackOffset, result, &nextFrame))
		return false;
	if (result == -3 && !_pendingSceneMember.empty()) {
		debugC(1, kDebugScene,
			"Ripper: concurrent script requested transition target='%s' entry='%s'",
			_pendingSceneMember.c_str(), _pendingSceneEntryLabel.c_str());
		_concurrent = CompiledScript();
		_concurrentEntryLabel.clear();
		return true;
	}
	if (result != -2) {
		warning("Ripper: concurrent frame '%s' returned unexpected result %d",
			_concurrentEntryLabel.c_str(), result);
		return false;
	}
	debugC(2, kDebugScene,
		"Ripper: concurrent frame yielded to active scene script='%s' nextFrame=%u",
		_ba0.getMemberName().c_str(), nextFrame);
	return true;
}

bool ScriptManager::performPendingSceneTransition() {
	if (_pendingSceneMember.empty())
		return true;

	const Common::String memberName = _pendingSceneMember;
	const Common::String entryLabel = _pendingSceneEntryLabel;
	_pendingSceneMember.clear();
	_pendingSceneEntryLabel.clear();
	debugC(1, kDebugScene,
		"Ripper: applying scene transition target='%s' entry='%s' concurrent='%s' clearPreservedAudio=%d",
		memberName.c_str(), entryLabel.c_str(), _concurrent.getMemberName().c_str(),
		_clearPreservedAudioOnTransition);
	// RunSceneScriptLoop at 0x124e9 retires non-preserved slots at every scene
	// handoff. Concurrent and world-map handoffs clear preserve bits first.
	_engine->getMedia()->clearSceneAudio(_clearPreservedAudioOnTransition);
	_clearPreservedAudioOnTransition = false;
	_engine->getToolbar()->leave();
	_dialogue->dismissForSceneTransition("scene-runtime-transition");
	if (!_ba0.load(_engine->getResources()->scripts(), memberName))
		return false;
	uint startFrame = 0;
	if (!entryLabel.empty() && !findFrameByLabel(_ba0, entryLabel, startFrame)) {
		warning("Ripper: scene transition target '%s' has no entry '%s'",
			memberName.c_str(), entryLabel.c_str());
		return false;
	}

	_previousBa0FrameLabel.clear();
	_activeBa0Frame = startFrame;
	_awaitingBa0Interaction = false;
	_hoveredBa0Interaction = -1;
	_engine->getCursor()->setVisible(false);
	g_system->fillScreen(0);
	g_system->updateScreen();
	return advanceBa0ToFrame(startFrame);
}

bool ScriptManager::advanceBa0ToFrame(uint nextFrame) {
	_engine->getCursor()->setVisible(false);
	_hoveredBa0Interaction = -1;
	// RunSceneScriptLoop at 0x124e9 has no frame-count bound here. A -2 result
	// re-enters the loop so the concurrent runtime is serviced before the next
	// active frame, even when opcode 0x14 selected that same frame again.
	while (!_engine->shouldQuit()) {
		if (!executeConcurrentFrame())
			return false;
		if (!_pendingSceneMember.empty())
			return performPendingSceneTransition();
		if (nextFrame >= _ba0.getFrames().size()) {
			warning("Ripper: active scene script='%s' requested invalid frame %u count=%u",
				_ba0.getMemberName().c_str(), nextFrame, _ba0.getFrames().size());
			return false;
		}

		bindBa0Frame(nextFrame);
		const ScriptFrame &frame = _ba0.getFrames()[_activeBa0Frame];
		const Common::String label = _ba0.getString(frame.labelOffset);
		debugC(1, kDebugScene,
			"Ripper: entering active scene script='%s' frame=%u label='%s' type=%u interactions=%u",
			_ba0.getMemberName().c_str(), _activeBa0Frame, label.c_str(),
			frame.presentationType, frame.interactionCount);
		initializeBa0InteractionState(frame);

		int result = 0;
		uint callbackFrame = _activeBa0Frame;
		if (!executeCallback(_ba0, frame.enterCallbackOffset, result, &callbackFrame))
			return false;
		if (result == -3 && !_pendingSceneMember.empty())
			return performPendingSceneTransition();
		if (result == -2) {
			_engine->getMedia()->resetSceneAudioTriggers();
			nextFrame = callbackFrame;
			continue;
		}
		if (result != 0)
			return false;

		if (frame.presentationType == 0 || frame.presentationType == 1) {
			const Common::String mediaPath = _ba0.getString(frame.mediaNameOffset);
			debugC(2, kDebugScene,
				"Ripper: frame presentation label='%s' media='%s' origin=%d,%d",
				label.c_str(), mediaPath.c_str(), frame.x, frame.y);
			markScenePlayed(label);
			if (frame.interactionCount != 0)
				beginBa0InteractionWait(label, frame.interactionCount);
			if (frame.idleCallbackOffset != 0 && _dialogue->hasChoices()) {
				int idleResult = 0;
				if (!executeCallback(_ba0, frame.idleCallbackOffset, idleResult) || idleResult != 0) {
					warning("Ripper: dialogue idle callback for frame '%s' returned %d",
						label.c_str(), idleResult);
					return false;
				}
				debugC(1, kDebugDialogue,
					"Ripper: activated frame dialogue chooser before media label='%s' idle=0x%x",
					label.c_str(), frame.idleCallbackOffset);
			}

			uint idleWaitFrame = 0;
			bool idleTextRequest = false;
			if (frame.idleCallbackOffset != 0 && !_dialogue->hasChoices()) {
				Common::Array<ScriptCommand> idleCommands;
				if (!_ba0.decodeCallback(frame.idleCallbackOffset, true, idleCommands))
					return false;
				for (uint commandIndex = 0; commandIndex < idleCommands.size(); ++commandIndex) {
					const ScriptCommand &idleCommand = idleCommands[commandIndex];
					if (idleCommand.opcode == kWaitForFrameCounter &&
							!idleCommand.arguments.empty())
						idleWaitFrame = idleCommand.arguments[0].value;
					else if (idleCommand.opcode == kRequestTextInput)
						idleTextRequest = true;
				}
				if (idleTextRequest && idleWaitFrame == 0) {
					warning("Ripper: scene text request frame='%s' has no media-frame wait",
						label.c_str());
					return false;
				}
			}
			const bool loopUntilInput = frame.presentationType == 1 && frame.interactionCount != 0;
			const bool allowEscSpace = frame.presentationType == 0;
			debugC(2, kDebugVideo,
				"Ripper: frame presentation controls label='%s' keyboard=%d mouse=%d idleTextRequest=%d waitFrame=%u",
				label.c_str(), allowEscSpace, loopUntilInput, idleTextRequest,
				idleWaitFrame);
			IdleMediaCallback idleCallback(this, _ba0, frame.idleCallbackOffset,
				idleWaitFrame, _activeBa0Frame);
			uint16 idleCommand = 0;
			if (!_engine->getMedia()->playScene(mediaPath, frame.x, frame.y, false,
				loopUntilInput, allowEscSpace,
				idleTextRequest ? &idleCallback : nullptr, &idleCommand))
				return false;
			if (idleTextRequest) {
				if (!idleCallback.serviced()) {
					warning("Ripper: scene text request frame='%s' media='%s' ended before target=%u",
						label.c_str(), mediaPath.c_str(), idleCallback.targetFrame());
					return false;
				}
				if (!idleCallback.succeeded())
					return false;
				result = idleCallback.result();
				callbackFrame = idleCallback.nextFrame();
				debugC(2, kDebugScene,
					"Ripper: scene text idle callback completed frame='%s' result=%d nextFrame=%u mediaCommand=0x%04x",
					label.c_str(), result, callbackFrame, idleCommand);
				if (result == -2) {
					_engine->getMedia()->resetSceneAudioTriggers();
					nextFrame = callbackFrame;
					continue;
				}
				if (result != 0)
					return false;
			}
			if (!_pendingSceneMember.empty()) {
				debugC(1, kDebugScene,
					"Ripper: scene presentation returned transition target='%s' entry='%s'",
					_pendingSceneMember.c_str(), _pendingSceneEntryLabel.c_str());
				return performPendingSceneTransition();
			}
			if (allowEscSpace)
				_dialogue->rebuildPresentationBands("controlled-media-complete");
			if (frame.interactionCount != 0)
				return true;
		}

		if (frame.interactionCount != 0 && !_awaitingBa0Interaction) {
			beginBa0InteractionWait(label, frame.interactionCount);
			return true;
		}

		callbackFrame = _activeBa0Frame;
		if (!executeCallback(_ba0, frame.exitCallbackOffset, result, &callbackFrame))
			return false;
		if (result == -3 && !_pendingSceneMember.empty())
			return performPendingSceneTransition();
		if (result != -2) {
			warning("Ripper: automatic active scene script='%s' frame='%s' returned unexpected result %d",
				_ba0.getMemberName().c_str(), label.c_str(), result);
			return false;
		}
		_engine->getMedia()->resetSceneAudioTriggers();
		nextFrame = callbackFrame;
		if (nextFrame == _activeBa0Frame) {
			debugC(2, kDebugScene,
				"Ripper: active scene script='%s' frame='%s' retained frame=%u; servicing concurrent='%s'",
				_ba0.getMemberName().c_str(), label.c_str(), nextFrame,
				_concurrent.getMemberName().c_str());
		}
	}
	return true;
}

bool ScriptManager::captureCyberKeyboardCommand() {
	if (!_cyberActive || !_engine->getInput()->hasPendingKey())
		return false;
	const uint16 command = _engine->getInput()->peekKey();
	if (command != kCyberLeftCommand && command != kCyberRightCommand &&
			command != kCyberChooseCommand && command != kCyberEscapeCommand)
		return false;
	_cyberKeyboardCommand = _engine->getInput()->consumeKey();
	debugC(3, kDebugCyber,
		"Ripper: captured Cyber keyboard command=0x%04x script='%s' frame=%u",
		_cyberKeyboardCommand, _ba0.getMemberName().c_str(), _activeBa0Frame);
	return true;
}

bool ScriptManager::serviceCyberKeyboardCommand() {
	const uint16 command = _cyberKeyboardCommand;
	_cyberKeyboardCommand = 0;
	if (!_cyberActive || command == 0)
		return true;
	if (!_awaitingBa0Interaction || _activeBa0Frame >= _ba0.getFrames().size())
		return false;

	const ScriptFrame &frame = _ba0.getFrames()[_activeBa0Frame];
	const ScriptInteraction *interaction = nullptr;
	uint interactionIndex = 0;
	for (uint relativeIndex = 0; relativeIndex < frame.interactionCount; ++relativeIndex) {
		const uint candidateIndex = frame.firstInteractionIndex + relativeIndex;
		if (relativeIndex < _activeBa0InteractionEnabled.size() &&
				_activeBa0InteractionEnabled[relativeIndex] &&
				candidateIndex < _ba0.getInteractions().size() &&
				_ba0.getInteractions()[candidateIndex].keyboardCommand == command) {
			interaction = &_ba0.getInteractions()[candidateIndex];
			interactionIndex = candidateIndex;
			break;
		}
	}
	if (!interaction || interaction->callbackOffset == 0) {
		warning("Ripper: Cyber keyboard command 0x%04x has no bound interaction in script='%s' frame=%u",
			command, _ba0.getMemberName().c_str(), _activeBa0Frame);
		return false;
	}

	int result = 0;
	uint nextFrame = _activeBa0Frame;
	debugC(2, kDebugCyber,
		"Ripper: Cyber keyboard command=0x%04x interaction=%u label='%s' callback=0x%x",
		command, interactionIndex, interaction->label.c_str(), interaction->callbackOffset);
	if (!executeCallback(_ba0, interaction->callbackOffset, result, &nextFrame))
		return false;
	_awaitingBa0Interaction = false;
	_engine->getCursor()->setVisible(false);
	_hoveredBa0Interaction = -1;
	if (result == -4 && _cyberExitRequested)
		return true;
	if (result != -2) {
		warning("Ripper: Cyber keyboard interaction=%u returned unexpected result %d",
			interactionIndex, result);
		return false;
	}
	_engine->getMedia()->resetSceneAudioTriggers();
	return advanceBa0ToFrame(nextFrame);
}

bool ScriptManager::serviceScene() {
	if (_resumeLoadedPresentation) {
		_resumeLoadedPresentation = false;
		const ScriptFrame &frame = _ba0.getFrames()[_activeBa0Frame];
		const Common::String label = _ba0.getString(frame.labelOffset);
		const Common::String mediaPath = _ba0.getString(frame.mediaNameOffset);
		debugC(1, kDebugSaveLoad,
			"Ripper: resuming loaded interactive presentation frame=%u label='%s' media='%s'",
			_activeBa0Frame, label.c_str(), mediaPath.c_str());
		if (!_engine->getMedia()->playScene(mediaPath, frame.x, frame.y, false, true, false))
			return false;
		if (!_pendingSceneMember.empty())
			return performPendingSceneTransition();
	}
	const MouseState mouse = _engine->getInput()->publishMouseState();
	if (_cyberExitRequested)
		return true;
	captureCyberKeyboardCommand();
	if (_cyberKeyboardCommand != 0)
		return serviceCyberKeyboardCommand();
	if (_engine->getInput()->peekKey() == kHelpCommand) {
		_engine->getInput()->consumeKey();
		if (!showHelp("scene"))
			return false;
		return true;
	}
	const BriefingServiceResult briefingResult = _briefing->service(mouse);
	if (briefingResult == kBriefingFailed)
		return false;
	if (briefingResult != kBriefingIdle)
		return true;
	const bool dialoguePending = _dialogue->isPending();
	if (dialoguePending) {
		const uint16 dialogueCommand = _engine->getInput()->peekKey();
		if (dialogueCommand == kDialogueEnterCommand ||
				dialogueCommand == kDialogueUpCommand ||
				dialogueCommand == kDialogueDownCommand) {
			_engine->getInput()->consumeKey();
			uint dialogueFrame = 0;
			if (_dialogue->serviceKeyboard(dialogueCommand, dialogueFrame)) {
				_awaitingBa0Interaction = false;
				_engine->getCursor()->setVisible(false);
				debugC(1, kDebugDialogue,
					"Ripper: dialogue keyboard chooser returned control=-2 nextFrame=%u",
					dialogueFrame);
				_engine->getMedia()->resetSceneAudioTriggers();
				return advanceBa0ToFrame(dialogueFrame);
			}
		}
		if (!_cyberActive && _engine->getToolbar()->service(mouse)) {
			if (!_pendingSceneMember.empty())
				return performPendingSceneTransition();
			_engine->getCursor()->setVisible(true);
			_engine->getCursor()->update(kToolbarCursor);
			return true;
		}
		uint dialogueFrame = 0;
		if (_dialogue->service(mouse, dialogueFrame)) {
			_awaitingBa0Interaction = false;
			_engine->getCursor()->setVisible(false);
			debugC(1, kDebugDialogue,
				"Ripper: dialogue chooser returned control=-2 nextFrame=%u", dialogueFrame);
			_engine->getMedia()->resetSceneAudioTriggers();
			return advanceBa0ToFrame(dialogueFrame);
		}
		_dialogue->draw();
		debugC(3, kDebugDialogue,
			"Ripper: dialogue chooser pending point=%d,%d buttons=0x%02x",
			mouse.position.x, mouse.position.y, mouse.buttons);
		if (_dialogue->contains(mouse.position)) {
			_engine->getCursor()->setVisible(true);
			_engine->getCursor()->update(kDialogueCursor);
			return true;
		}
	}
	if (!_awaitingBa0Interaction) {
		_engine->getToolbar()->leave();
		_engine->getCursor()->setVisible(false);
		_hoveredBa0Interaction = -1;
		return true;
	}
	if (!_cyberActive && !dialoguePending && _engine->getToolbar()->service(mouse)) {
		if (!_pendingSceneMember.empty())
			return performPendingSceneTransition();
		_engine->getCursor()->update(kToolbarCursor);
		return true;
	}

	const ScriptFrame &frame = _ba0.getFrames()[_activeBa0Frame];
	uint hoveredInteractionIndex = 0;
	const ScriptInteraction *hoveredInteraction =
		findBa0Interaction(mouse.position, &hoveredInteractionIndex);

	const uint cursorIndex = hoveredInteraction ?
		(hoveredInteraction->conditionOffset != 0 ? 8 : hoveredInteraction->initialSelection) :
		(dialoguePending && _dialogue->contains(mouse.position) ? kDialogueCursor :
			_engine->getCursor()->getSelectionIndex());
	_engine->getCursor()->update(cursorIndex);
	const int hoveredIndex = hoveredInteraction ? (int)hoveredInteractionIndex : -1;
	if (hoveredIndex != _hoveredBa0Interaction) {
		_hoveredBa0Interaction = hoveredIndex;
		if (hoveredInteraction) {
			const Common::Rect bounds = interactionBounds(*hoveredInteraction);
			debugC(2, kDebugScene,
				"Ripper: active scene hover script='%s' interaction=%u label='%s' cursor=%u "
				"point=%d,%d rect=%d,%d,%d,%d",
				_ba0.getMemberName().c_str(), hoveredInteractionIndex,
				hoveredInteraction->label.c_str(), cursorIndex,
				mouse.position.x, mouse.position.y, bounds.left, bounds.top,
				bounds.width(), bounds.height());
		} else {
			debugC(2, kDebugScene,
				"Ripper: active scene hover cleared script='%s' cursor=%u point=%d,%d",
				_ba0.getMemberName().c_str(), cursorIndex,
				mouse.position.x, mouse.position.y);
		}
	}

	if ((mouse.pressed & kMouseButtonLeft) == 0)
		return true;
	if (hoveredInteraction) {
		const Common::Rect bounds = interactionBounds(*hoveredInteraction);
		if (dialoguePending) {
			_dialogue->dismissForSceneTransition("scene-interaction");
			debugC(1, kDebugDialogue,
				"Ripper: dialogue chooser closed by scene interaction=%u label='%s'",
				hoveredInteractionIndex, hoveredInteraction->label.c_str());
		}
		debugC(1, kDebugInput,
			"Ripper: hotspot click frame=%u label='%s' interaction=%u action='%s' "
				"point=%d,%d rect=%d,%d,%d,%d cursor=%u condition=0x%x callback=0x%x flags=0x%02x",
			_activeBa0Frame, _ba0.getString(frame.labelOffset).c_str(), hoveredInteractionIndex,
			hoveredInteraction->label.c_str(), mouse.position.x, mouse.position.y,
			bounds.left, bounds.top, bounds.width(), bounds.height(), cursorIndex,
			hoveredInteraction->conditionOffset,
			hoveredInteraction->callbackOffset, hoveredInteraction->flags);
		debugC(2, kDebugScene,
			"Ripper: active scene chooser selected script='%s' interaction=%u label='%s' point=%d,%d rect=%d,%d,%d,%d",
			_ba0.getMemberName().c_str(), hoveredInteractionIndex,
			hoveredInteraction->label.c_str(), mouse.position.x, mouse.position.y,
			bounds.left, bounds.top, bounds.width(), bounds.height());
		int result = 0;
		uint nextFrame = _activeBa0Frame;
		if (!executeCallback(_ba0, hoveredInteraction->callbackOffset, result, &nextFrame))
			return false;
		debugC(1, kDebugScripts,
			"Ripper: hotspot action script='%s' callback=0x%x result=%d nextFrame=%u",
			_ba0.getMemberName().c_str(), hoveredInteraction->callbackOffset, result, nextFrame);
		if (result == -3 && !_pendingSceneMember.empty()) {
			_awaitingBa0Interaction = false;
			_engine->getCursor()->setVisible(false);
			_hoveredBa0Interaction = -1;
			return performPendingSceneTransition();
		}
		if (result == -4 && _cyberActive && _cyberExitRequested) {
			_awaitingBa0Interaction = false;
			_engine->getCursor()->setVisible(false);
			_hoveredBa0Interaction = -1;
			return true;
		}
		if (result != -2) {
			warning("Ripper: active scene script='%s' interaction=%u returned unexpected result %d",
				_ba0.getMemberName().c_str(), hoveredInteractionIndex, result);
			return false;
		}
		_awaitingBa0Interaction = false;
		_engine->getCursor()->setVisible(false);
		_hoveredBa0Interaction = -1;
		debugC(1, kDebugScene,
			"Ripper: active scene script='%s' yielded to concurrent script='%s' entry='%s' nextFrame=%u",
			_ba0.getMemberName().c_str(), _concurrent.getMemberName().c_str(),
			_concurrentEntryLabel.c_str(), nextFrame);
		_engine->getMedia()->resetSceneAudioTriggers();
		return advanceBa0ToFrame(nextFrame);
	}

	debugC(3, kDebugScene,
		"Ripper: active scene primary press missed chooser script='%s' point=%d,%d",
		_ba0.getMemberName().c_str(), mouse.position.x, mouse.position.y);
	return true;
}

void ScriptManager::drawDialogueOverlay(bool captureBacking) {
	_dialogue->draw(captureBacking);
}

void ScriptManager::drawBriefingOverlay() {
	_briefing->draw();
}

bool ScriptManager::updateInteractiveCursor(const Common::Point &point, bool *failed) {
	if (captureCyberKeyboardCommand())
		return false;
	const BriefingServiceResult briefingResult =
		_briefing->service(_engine->getInput()->peekMouseState());
	if (failed)
		*failed = briefingResult == kBriefingFailed;
	if (briefingResult != kBriefingIdle)
		return false;
	if (!_awaitingBa0Interaction || _activeBa0Frame >= _ba0.getFrames().size())
		return false;
	if (!_cyberActive && _engine->getToolbar()->service(_engine->getInput()->peekMouseState())) {
		_engine->getCursor()->setVisible(true);
		_engine->getCursor()->update(kToolbarCursor);
		return true;
	}
	if (_dialogue->isPending()) {
		_dialogue->updateHover(point);
		const ScriptInteraction *interaction = findBa0Interaction(point);
		const uint cursorIndex = _dialogue->contains(point) ? kDialogueCursor :
			(interaction ? (interaction->conditionOffset != 0 ? 8 : interaction->initialSelection) :
				_engine->getCursor()->getSelectionIndex());
		_engine->getCursor()->setVisible(true);
		_engine->getCursor()->update(cursorIndex);
		return false;
	}
	const ScriptInteraction *interaction = findBa0Interaction(point);
	const uint cursorIndex = interaction ?
		(interaction->conditionOffset != 0 ? 8 : interaction->initialSelection) :
		_engine->getCursor()->getSelectionIndex();
	_engine->getCursor()->setVisible(true);
	_engine->getCursor()->update(cursorIndex);
	return false;
}

void ScriptManager::updateModalSceneCursor(const Common::Point &point) {
	// RunModalSelectionTableDialogWithRestore at 0x1f7f8 retains the scene
	// selection-state list while the modal chooser is active. Outside the modal
	// rectangle, ProcessChooserControlInput at 0x57372 services that retained
	// list so hotspot cursors continue to animate without dispatching actions.
	const ScriptInteraction *interaction = _awaitingBa0Interaction ?
		findBa0Interaction(point) : nullptr;
	uint cursorIndex = interaction ?
		(interaction->conditionOffset != 0 ? 8 : interaction->initialSelection) :
		_engine->getCursor()->getSelectionIndex();
	if (_dialogue->isPending() && _dialogue->contains(point))
		cursorIndex = kDialogueCursor;
	_engine->getCursor()->setVisible(true);
	_engine->getCursor()->update(cursorIndex);
}

Common::Rect ScriptManager::interactionBounds(const ScriptInteraction &interaction) const {
	// ExecuteSceneFrameAndInteractions at 0x13277 transposes the compiled
	// interaction axes and adds the active 0x32 presentation origin to the
	// first logical coordinate before registering the physical UI rectangle.
	return Common::Rect(interaction.y, interaction.x + kSceneInteractionOriginY,
		interaction.y + interaction.height,
		interaction.x + kSceneInteractionOriginY + interaction.width);
}

const ScriptInteraction *ScriptManager::findBa0Interaction(const Common::Point &point,
		uint *interactionIndex) const {
	if (_activeBa0Frame >= _ba0.getFrames().size() || point.y >= 400)
		return nullptr;
	const ScriptFrame &frame = _ba0.getFrames()[_activeBa0Frame];
	for (uint i = 0; i < frame.interactionCount; ++i) {
		const uint index = frame.firstInteractionIndex + i;
		const ScriptInteraction &interaction = _ba0.getInteractions()[index];
		if (i >= _activeBa0InteractionEnabled.size() || !_activeBa0InteractionEnabled[i] ||
			(interaction.flags & 2) != 0 || interaction.width <= 0 || interaction.height <= 0)
			continue;
		if (!interactionBounds(interaction).contains(point))
			continue;
		if (interactionIndex)
			*interactionIndex = index;
		return &interaction;
	}
	return nullptr;
}

} // End of namespace Ripper
