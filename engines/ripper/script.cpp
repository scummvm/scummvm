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

#include "common/debug.h"
#include "common/ptr.h"
#include "common/stream.h"

#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/media.h"
#include "ripper/resources.h"
#include "ripper/ripper.h"

namespace Ripper {

static const uint32 kScriptHeaderSize = 0xe9;
static const uint32 kFrameRecordSize = 0x22;
static const uint32 kInteractionRecordSize = 0x25;

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
		for (uint labelIndex = 0; labelIndex < 0xd && _data[offset + labelIndex] != 0; ++labelIndex)
			interaction.label += (char)_data[offset + labelIndex];
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
		command.opcode = _data[cursor++];
		if (command.opcode == 'c')
			return true;
		if (command.opcode >= _argumentLayoutCount || !canRead(cursor, 2)) {
			warning("Ripper: script '%s' has invalid opcode 0x%02x at 0x%x",
				_memberName.c_str(), command.opcode, command.offset);
			return false;
		}

		command.selector = readUint16(cursor);
		cursor += 2;
		const uint32 layout = _argumentLayoutOffset + command.opcode * _argumentLayoutStride;
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
		_awaitingBa0Interaction(false) {
}

bool ScriptManager::initialize(ResourceManager &resources) {
	return _startup.load(resources.scripts(), "ripper.run") &&
		_ba0.load(resources.scripts(), "ba0.run");
}

Common::String ScriptManager::argumentString(const ScriptArgument &argument) {
	Common::String value;
	for (uint i = 0; i < argument.data.size() && argument.data[i] != 0; ++i)
		value += (char)argument.data[i];
	return value;
}

bool ScriptManager::executeCallback(CompiledScript &script, uint32 callbackOffset, int &result) {
	Common::Array<ScriptCommand> commands;
	result = 0;
	if (!script.decodeCallback(callbackOffset, true, commands))
		return false;

	for (uint commandIndex = 0; commandIndex < commands.size(); ++commandIndex) {
		const ScriptCommand &command = commands[commandIndex];
		debugC(2, kDebugScripts,
			"Ripper: execute script='%s' offset=0x%x opcode=0x%02x selector=%u arguments=%u",
			script.getMemberName().c_str(), command.offset, command.opcode, command.selector,
			command.arguments.size());

		switch (command.opcode) {
		case 0x0e: {
			if (command.arguments.size() < 1)
				return false;
			const uint32 flag = command.arguments[0].value;
			if (_milestoneFlags.size() <= flag)
				_milestoneFlags.resize(flag + 1);
			_milestoneFlags[flag] = true;
			debugC(2, kDebugScripts, "Ripper: set milestone flag %u", flag);
			break;
		}

		case 0x1a: {
			if (command.arguments.size() < 5)
				return false;
			const Common::String mediaPath = script.getString(command.arguments[0].value);
			const bool allowEscSpace = command.arguments[2].value == 0;
			if (!_engine->getMedia()->play(mediaPath, allowEscSpace,
				(int32)command.arguments[3].value, (int32)command.arguments[4].value))
				return false;
			debugC(2, kDebugScripts, "Ripper: completed media command '%s' controls=%d",
				mediaPath.c_str(), allowEscSpace);
			break;
		}

		case 0x1d: {
			if (command.arguments.size() < 3)
				return false;
			const Common::String target = argumentString(command.arguments[0]);
			const Common::String entryLabel = argumentString(command.arguments[1]);
			if (command.arguments[2].value == 0) {
				debugC(1, kDebugScene, "Ripper: transition script='%s' entry='%s'",
					target.c_str(), entryLabel.c_str());
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

		case 0x1f: {
			if (command.arguments.size() < 2)
				return false;
			const Common::String audioPath = script.getString(command.arguments[0].value);
			if (!_engine->getMedia()->loadAudio(audioPath))
				return false;
			break;
		}

		case 0x20: {
			if (command.arguments.size() < 4)
				return false;
			const Common::String key = argumentString(command.arguments[0]);
			const uint volume = command.arguments[1].value == 0 ? 100 : command.arguments[1].value & 0xff;
			const uint trigger = command.arguments[2].value & 0xffff;
			if (trigger == 0 && !_engine->getMedia()->startLoadedAudio(key, volume))
				return false;
			debugC(2, kDebugScripts,
				"Ripper: configured audio key='%s' volume=%u trigger=%u control=%u",
				key.c_str(), volume, trigger, command.arguments[3].value & 0xff);
			break;
		}

		case 0x1b: {
			if (command.arguments.size() < 4)
				return false;
			const Common::String mediaPath = script.getString(command.arguments[0].value);
			if (!_engine->getMedia()->play(mediaPath, false, (int32)command.arguments[1].value,
				(int32)command.arguments[2].value))
				return false;
			break;
		}

		case 0x14:
			if (command.arguments.size() < 1)
				return false;
			result = -2;
			debugC(2, kDebugScripts, "Ripper: callback result index=%u control=%d",
				command.arguments[0].value, result);
			return true;

		default:
			warning("Ripper: unsupported opcode 0x%02x in '%s' at 0x%x",
				command.opcode, script.getMemberName().c_str(), command.offset);
			return false;
		}
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

	debugC(1, kDebugScene, "Ripper: entering BA0 frame=%u label='start'", ba0StartFrame);
	result = 0;
	if (!executeCallback(_ba0, _ba0.getFrames()[ba0StartFrame].enterCallbackOffset, result) || result != 0)
		return false;
	_activeBa0Frame = ba0StartFrame;
	_awaitingBa0Interaction = true;
	debugC(1, kDebugScene, "Ripper: BA0 start frame initialized; awaiting chooser interaction");
	return true;
}

bool ScriptManager::serviceScene() {
	const MouseState mouse = _engine->getInput()->publishMouseState();
	if (!_awaitingBa0Interaction || (mouse.pressed & kMouseButtonLeft) == 0)
		return true;

	const ScriptFrame &frame = _ba0.getFrames()[_activeBa0Frame];
	for (uint i = 0; i < frame.interactionCount; ++i) {
		const uint interactionIndex = frame.firstInteractionIndex + i;
		const ScriptInteraction &interaction = _ba0.getInteractions()[interactionIndex];
		if ((interaction.flags & 2) != 0 || interaction.width <= 0 || interaction.height <= 0)
			continue;
		if (mouse.position.x < 0 || mouse.position.x >= interaction.width ||
			mouse.position.y < interaction.y || mouse.position.y >= interaction.y + interaction.height)
			continue;

		debugC(2, kDebugScene,
			"Ripper: BA0 chooser selected interaction=%u label='%s' point=%d,%d rect=0,%d,%d,%d",
			interactionIndex, interaction.label.c_str(), mouse.position.x, mouse.position.y,
			interaction.y, interaction.width, interaction.height);
		int result = 0;
		if (!executeCallback(_ba0, interaction.callbackOffset, result))
			return false;
		if (result != -2) {
			warning("Ripper: BA0 interaction %u returned unexpected result %d", interactionIndex, result);
			return false;
		}
		_awaitingBa0Interaction = false;
		debugC(1, kDebugScene, "Ripper: handed BA0 control to concurrent script='%s' entry='%s'",
			_concurrent.getMemberName().c_str(), _concurrentEntryLabel.c_str());
		return true;
	}

	debugC(3, kDebugScene, "Ripper: BA0 primary press missed chooser point=%d,%d",
		mouse.position.x, mouse.position.y);
	return true;
}

} // End of namespace Ripper
