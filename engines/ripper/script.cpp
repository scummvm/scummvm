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

#include "ripper/detection.h"
#include "ripper/cursor.h"
#include "ripper/input.h"
#include "ripper/media.h"
#include "ripper/milestones.h"
#include "ripper/resources.h"
#include "ripper/ripper.h"
#include "ripper/toolbar.h"
#include "ripper/world_map.h"

namespace Ripper {

// RunStartupFrontEndLoop at 0x10778 uses selection cursor 0xe for the white
// menu pointer. PollInteractionAndResolveSelection at 0x13c8d enters the same
// front-end action service for the top 50-pixel band of every scene frame.
static const uint kToolbarCursor = 14;
static const uint kDialogueCursor = 16;

static const uint32 kScriptHeaderSize = 0xe9;
static const uint32 kFrameRecordSize = 0x22;
static const uint32 kInteractionRecordSize = 0x25;
static const byte kCallbackTerminator = 'c';

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
		_awaitingBa0Interaction(false), _resumeLoadedPresentation(false) {
	_briefing = new BriefingManager(engine);
	_dialogue = new DialogueManager();
}

bool ScriptManager::canSaveGame() const {
	return _awaitingBa0Interaction && _pendingSceneMember.empty() &&
		_activeBa0Frame < _ba0.getFrames().size();
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
	debugC(1, kDebugScene, "Ripper: BA0 frame='%s' awaiting %u interactions",
		frameLabel.c_str(), interactionCount);
}

void ScriptManager::initializeBa0InteractionState(const ScriptFrame &frame) {
	_activeBa0InteractionEnabled.clear();
	_activeBa0InteractionEnabled.resize(frame.interactionCount);
	for (uint i = 0; i < frame.interactionCount; ++i) {
		const ScriptInteraction &interaction =
			_ba0.getInteractions()[frame.firstInteractionIndex + i];
		_activeBa0InteractionEnabled[i] = (interaction.flags & 2) == 0;
	}
	debugC(3, kDebugScene, "Ripper: initialized %u BA0 interaction proxies",
		frame.interactionCount);
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
		"Ripper: bound BA0 frame=%u previous='%s'",
		_activeBa0Frame, _previousBa0FrameLabel.c_str());
}

bool ScriptManager::executeCallback(CompiledScript &script, uint32 callbackOffset, int &result,
		uint *nextFrame) {
	Common::Array<ScriptCommand> commands;
	result = 0;
	if (nextFrame)
		*nextFrame = 0;
	if (!script.decodeCallback(callbackOffset, true, commands))
		return false;
	debugC(2, kDebugScripts,
		"Ripper: decoded callback script='%s' offset=0x%x commands=%u",
		script.getMemberName().c_str(), callbackOffset, commands.size());

	uint commandIndex = 0;
	bool branchTaken = false;
	while (commandIndex < commands.size()) {
		const ScriptCommand &command = commands[commandIndex];
		debugC(2, kDebugScripts,
			"Ripper: execute script='%s' offset=0x%x opcode=0x%02x selector=%u arguments=%u",
			script.getMemberName().c_str(), command.offset, command.opcode, command.selector,
			command.arguments.size());

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
				warning("Ripper: interaction disable opcode has no active BA0 frame in '%s' at 0x%x",
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
				(int32)command.arguments[3].value, (int32)command.arguments[4].value))
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
				_pendingSceneMember = target;
				if (!_pendingSceneMember.hasSuffixIgnoreCase(".run"))
					_pendingSceneMember += ".run";
				_pendingSceneEntryLabel = entryLabel;
				debugC(1, kDebugScene,
					"Ripper: queued script transition target='%s' entry='%s' source='%s'",
					_pendingSceneMember.c_str(), _pendingSceneEntryLabel.c_str(),
					script.getMemberName().c_str());
				result = -3;
				return true;
			}
			Common::String memberName = target;
			if (!memberName.hasSuffixIgnoreCase(".run"))
				memberName += ".run";
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
			if (!_engine->getMedia()->loadAudio(audioPath))
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
			const bool loop = (control & 1) != 0;
			if (trigger == 0 && !_engine->getMedia()->startLoadedAudio(key, volume, loop))
				return false;
			debugC(2, kDebugAudio,
				"Ripper: configured audio key='%s' volume=%u trigger=%u control=%u loop=%d",
				key.c_str(), volume, trigger, control, loop);
			break;
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
			if (command.arguments[0].value == 2) {
				if (!openWorldMap())
					return false;
				if (!_pendingSceneMember.empty()) {
					result = -3;
					return true;
				}
				break;
			}
			if (command.arguments[0].value != 300) {
				warning("Ripper: unsupported scene action %u in '%s' at 0x%x",
					command.arguments[0].value,
					script.getMemberName().c_str(), command.offset);
				return false;
			}
			if (!_briefing->arm(command.arguments[1].value))
				return false;
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
	debugC(2, kDebugScene, "Ripper: concurrent frame yielded to BA0 nextFrame=%u", nextFrame);
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
		"Ripper: applying scene transition target='%s' entry='%s' concurrent='%s'",
		memberName.c_str(), entryLabel.c_str(), _concurrent.getMemberName().c_str());
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
	for (uint transitionCount = 0; transitionCount < _ba0.getFrames().size(); ++transitionCount) {
		if (!executeConcurrentFrame())
			return false;
		if (!_pendingSceneMember.empty())
			return performPendingSceneTransition();
		if (nextFrame >= _ba0.getFrames().size()) {
			warning("Ripper: BA0 requested invalid frame %u", nextFrame);
			return false;
		}

		bindBa0Frame(nextFrame);
		const ScriptFrame &frame = _ba0.getFrames()[_activeBa0Frame];
		const Common::String label = _ba0.getString(frame.labelOffset);
		debugC(1, kDebugScene, "Ripper: entering BA0 frame=%u label='%s' type=%u interactions=%u",
			_activeBa0Frame, label.c_str(), frame.presentationType, frame.interactionCount);
		initializeBa0InteractionState(frame);

		int result = 0;
		uint callbackFrame = _activeBa0Frame;
		if (!executeCallback(_ba0, frame.enterCallbackOffset, result, &callbackFrame))
			return false;
		if (result == -3 && !_pendingSceneMember.empty())
			return performPendingSceneTransition();
		if (result == -2) {
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
			const bool loopUntilInput = frame.presentationType == 1 && frame.interactionCount != 0;
			const bool allowEscSpace = frame.presentationType == 0;
			debugC(2, kDebugVideo,
				"Ripper: frame presentation controls label='%s' keyboard=%d mouse=%d",
				label.c_str(), allowEscSpace, loopUntilInput);
			if (!_engine->getMedia()->playScene(mediaPath, frame.x, frame.y, false,
				loopUntilInput, allowEscSpace))
				return false;
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
			warning("Ripper: automatic BA0 frame '%s' returned unexpected result %d",
				label.c_str(), result);
			return false;
		}
		nextFrame = callbackFrame;
	}

	warning("Ripper: BA0 exceeded the automatic transition limit");
	return false;
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
	}
	const MouseState mouse = _engine->getInput()->publishMouseState();
	const BriefingServiceResult briefingResult = _briefing->service(mouse);
	if (briefingResult == kBriefingFailed)
		return false;
	if (briefingResult != kBriefingIdle)
		return true;
	const bool dialoguePending = _dialogue->isPending();
	if (dialoguePending) {
		if (_engine->getToolbar()->service(mouse)) {
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
	if (!dialoguePending && _engine->getToolbar()->service(mouse)) {
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
		(dialoguePending && _dialogue->contains(mouse.position) ? kDialogueCursor : 0);
	_engine->getCursor()->update(cursorIndex);
	const int hoveredIndex = hoveredInteraction ? (int)hoveredInteractionIndex : -1;
	if (hoveredIndex != _hoveredBa0Interaction) {
		_hoveredBa0Interaction = hoveredIndex;
		if (hoveredInteraction) {
			debugC(2, kDebugScene,
				"Ripper: BA0 hover interaction=%u label='%s' cursor=%u point=%d,%d",
				hoveredInteractionIndex, hoveredInteraction->label.c_str(), cursorIndex,
				mouse.position.x, mouse.position.y);
		} else {
			debugC(2, kDebugScene, "Ripper: BA0 hover cleared; cursor=%u point=%d,%d",
				cursorIndex, mouse.position.x, mouse.position.y);
		}
	}

	if ((mouse.pressed & kMouseButtonLeft) == 0)
		return true;
	if (hoveredInteraction) {
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
			hoveredInteraction->y, hoveredInteraction->x, hoveredInteraction->height,
			hoveredInteraction->width, cursorIndex, hoveredInteraction->conditionOffset,
			hoveredInteraction->callbackOffset, hoveredInteraction->flags);
		debugC(2, kDebugScene,
			"Ripper: BA0 chooser selected interaction=%u label='%s' point=%d,%d rect=%d,%d,%d,%d",
			hoveredInteractionIndex, hoveredInteraction->label.c_str(), mouse.position.x, mouse.position.y,
			hoveredInteraction->y, hoveredInteraction->x, hoveredInteraction->height,
			hoveredInteraction->width);
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
		if (result != -2) {
			warning("Ripper: BA0 interaction %u returned unexpected result %d",
				hoveredInteractionIndex, result);
			return false;
		}
		_awaitingBa0Interaction = false;
		_engine->getCursor()->setVisible(false);
		_hoveredBa0Interaction = -1;
		debugC(1, kDebugScene,
			"Ripper: BA0 yielded to concurrent script='%s' entry='%s' nextFrame=%u",
			_concurrent.getMemberName().c_str(), _concurrentEntryLabel.c_str(), nextFrame);
		return advanceBa0ToFrame(nextFrame);
	}

	debugC(3, kDebugScene, "Ripper: BA0 primary press missed chooser point=%d,%d",
		mouse.position.x, mouse.position.y);
	return true;
}

void ScriptManager::drawDialogueOverlay(bool captureBacking) {
	_dialogue->draw(captureBacking);
}

void ScriptManager::drawBriefingOverlay() {
	_briefing->draw();
}

bool ScriptManager::updateInteractiveCursor(const Common::Point &point, bool *failed) {
	const BriefingServiceResult briefingResult =
		_briefing->service(_engine->getInput()->peekMouseState());
	if (failed)
		*failed = briefingResult == kBriefingFailed;
	if (briefingResult != kBriefingIdle)
		return false;
	if (!_awaitingBa0Interaction || _activeBa0Frame >= _ba0.getFrames().size())
		return false;
	if (_engine->getToolbar()->service(_engine->getInput()->peekMouseState())) {
		_engine->getCursor()->setVisible(true);
		_engine->getCursor()->update(kToolbarCursor);
		return true;
	}
	if (_dialogue->isPending()) {
		_dialogue->updateHover(point);
		const ScriptInteraction *interaction = findBa0Interaction(point);
		const uint cursorIndex = _dialogue->contains(point) ? kDialogueCursor :
			(interaction ? (interaction->conditionOffset != 0 ? 8 : interaction->initialSelection) : 0);
		_engine->getCursor()->setVisible(true);
		_engine->getCursor()->update(cursorIndex);
		return false;
	}
	const ScriptInteraction *interaction = findBa0Interaction(point);
	const uint cursorIndex = interaction ?
		(interaction->conditionOffset != 0 ? 8 : interaction->initialSelection) : 0;
	_engine->getCursor()->setVisible(true);
	_engine->getCursor()->update(cursorIndex);
	return false;
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
		if (point.x < interaction.y || point.x >= interaction.y + interaction.height ||
			point.y < interaction.x || point.y >= interaction.x + interaction.width)
			continue;
		if (interactionIndex)
			*interactionIndex = index;
		return &interaction;
	}
	return nullptr;
}

} // End of namespace Ripper
