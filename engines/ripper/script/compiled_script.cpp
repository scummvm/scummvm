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
#include "ripper/resources.h"

namespace Ripper {

namespace {

static const uint32 kScriptHeaderSize = 0xe9;
static const uint32 kFrameRecordSize = 0x22;
static const uint32 kInteractionRecordSize = 0x25;
static const byte kCallbackTerminator = 'c';

} // End of anonymous namespace

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

Common::String CompiledScript::getPresentationText(uint32 offset) const {
	if (offset >= _data.size())
		return Common::String();

	Common::String value;
	while (offset < _data.size() && _data[offset] != 0) {
		byte character = _data[offset++];
		// ExecutePresentationEntry at 0x1754b receives the frame's auxiliary
		// string pointer. RIPPER.LE SCR compilation stores those bytes inverted,
		// preserving line feeds just like inline type-7 command arguments.
		if (character != '\n')
			character ^= 0xff;
		value += (char)character;
	}
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
		_interactions.push_back(Common::move(interaction));
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
			command.arguments.push_back(Common::move(argument));
		}

		debugC(3, kDebugScripts,
			"Ripper: script='%s' callback=0x%x opcode=0x%02x selector=%u arguments=%u next=0x%x",
			_memberName.c_str(), command.offset, command.opcode, command.selector,
			command.arguments.size(), cursor);
		commands.push_back(Common::move(command));
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

} // End of namespace Ripper
