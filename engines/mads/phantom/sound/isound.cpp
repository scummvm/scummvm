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

#include "common/endian.h"
#include "common/file.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "mads/phantom/sound/isound.h"

namespace MADS {
namespace Phantom {
namespace Sound {

namespace {

void setReason(Common::String *reason, const Common::String &value) {
	if (reason)
		*reason = value;
}

} // namespace

bool ISound::readOverlayLayout(const Common::Path &filename,
		OverlayLayout &layout, Common::String *reason) {
	Common::File file;
	if (!file.open(filename)) {
		setReason(reason, Common::String::format("could not open %s",
			filename.toString().c_str()));
		return false;
	}

	const int64 fileSize = file.size();
	if (fileSize < 0x200) {
		setReason(reason, "overlay is smaller than its DOS MZ header");
		return false;
	}

	file.seek(0);
	if (file.readUint16LE() != 0x5a4d) {
		setReason(reason, "overlay does not have an MZ signature");
		return false;
	}

	const uint16 bytesOnLastPage = file.readUint16LE();
	const uint16 pageCount = file.readUint16LE();
	if (!pageCount || bytesOnLastPage >= 512) {
		setReason(reason, "overlay has invalid MZ size fields");
		return false;
	}
	const uint32 declaredSize = (pageCount - 1) * 512U +
		(bytesOnLastPage ? bytesOnLastPage : 512U);
	if (declaredSize != (uint64)fileSize) {
		setReason(reason, "overlay MZ size does not match the file");
		return false;
	}

	file.seek(8);
	const uint32 imageOffset = file.readUint16LE() * 16U;
	if (imageOffset < 0x1c || imageOffset + 0x48 > (uint64)fileSize) {
		setReason(reason, "overlay has an invalid MZ header size");
		return false;
	}

	file.seek(imageOffset + 0x2a);
	const uint32 dataSegmentOffset = file.readUint16LE() * 16U;
	const uint16 dataSegmentSize = file.readUint16LE();
	const uint16 nominalTimerHz = file.readUint16LE();
	const uint16 exportCount = file.readUint16LE();
	const uint32 dataOffset = imageOffset + dataSegmentOffset;
	if (dataOffset >= (uint64)fileSize) {
		setReason(reason, "overlay data segment is outside the file");
		return false;
	}

	layout.dataOffset = dataOffset;
	layout.initializedDataSize = (uint32)(fileSize - dataOffset);
	layout.dataSegmentSize = dataSegmentSize;
	if (dataSegmentSize < layout.initializedDataSize ||
		dataSegmentSize < 0x1a0 || nominalTimerHz != 100 ||
		exportCount != 11) {
		setReason(reason, "overlay descriptor is not the supported Phantom ISOUND ABI");
		return false;
	}

	return true;
}

bool ISound::isOverlaySupported(const Common::Path &filename,
		Common::String *reason) {
	OverlayLayout layout;
	return readOverlayLayout(filename, layout, reason);
}

ISound::ISound(Audio::Mixer *mixer, const Common::Path &filename,
		const OverlaySpec &spec) :
		SoundDriver(mixer),
	_spec(spec),
	_noiseEnabled(false),
	_updatesEnabled(false),
	_streamInvalid(false),
	_masterVolume(255),
	_outputRate(kPCSpeakerSampleRate),
	_hostTimerAccumulator(0),
	_sequenceServiceCountdown(1),
	_pitRenderer(_outputRate, kPitClockHz),
	_frameCounter(0),
	_randomSeed(0),
	_pollResult(0),
	_resultState(0),
	_priority(0),
	_sequenceStart(0),
	_position(0),
	_innerLoopStart(0),
	_outerLoopStart(0),
	_restartOverride(0),
	_branchReturn(0),
	_innerLoopCount(0),
	_outerLoopCount(0),
	_note(0),
	_activeTicks(0),
	_releaseCounter(0),
	_releaseOverride(0),
	_gateOffset(0),
	_transpose(0),
	_fineOffset(0),
	_noiseMask(0),
	_currentDivisor(0),
	_pitchStep(0),
	_directDivisor(0),
	_alternationReload(0),
	_alternationOffset(0),
	_alternationCounter(0),
	_alternationToggle(false),
	_sweepInitialized(false),
	_sweepUpper(0),
	_sweepLower(0),
	_sweepDirection(1),
	_tempoShift(0),
	_tempoTarget(0),
	_tempoReload(0),
	_tempoScale(0) {
	OverlayLayout layout;
	Common::String reason;
	if (!readOverlayLayout(filename, layout, &reason))
		error("Unsupported Phantom ISOUND overlay %s: %s",
			filename.toString().c_str(), reason.c_str());

	Common::File soundFile;
	if (!soundFile.open(filename))
		error("Could not open file - %s", filename.toString().c_str());
	_soundData.resize(layout.dataSegmentSize);
	soundFile.seek(layout.dataOffset);
	soundFile.read(&_soundData[0], layout.initializedDataSize);
	if (layout.dataSegmentSize > layout.initializedDataSize) {
		memset(&_soundData[layout.initializedDataSize], 0,
			layout.dataSegmentSize - layout.initializedDataSize);
	}
	memset(_scriptVariables, 0, sizeof(_scriptVariables));
	if (!readWord(_spec.randomSeedOffset, _randomSeed))
		_randomSeed = 0x4d2;

	initializeDriver();
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_speakerHandle,
		this, -1, Audio::Mixer::kMaxChannelVolume, 0,
		DisposeAfterUse::NO, true);
}

ISound::~ISound() {
	_mixer->stopHandle(_speakerHandle);
}

bool ISound::readByte(uint16 offset, byte &value) {
	if (offset >= _soundData.size()) {
		invalidateStream("byte read outside the mutable data segment", offset);
		return false;
	}
	value = _soundData[offset];
	return true;
}

bool ISound::readWord(uint16 offset, uint16 &value) {
	if ((uint32)offset + 1 >= _soundData.size()) {
		invalidateStream("word read outside the mutable data segment", offset);
		return false;
	}
	value = READ_LE_UINT16(&_soundData[offset]);
	return true;
}

bool ISound::writeByte(uint16 offset, byte value) {
	if (offset >= _soundData.size()) {
		invalidateStream("byte write outside the mutable data segment", offset);
		return false;
	}
	_soundData[offset] = value;
	return true;
}

void ISound::invalidateStream(const char *reason, uint16 offset) {
	if (!_streamInvalid)
		warning("Phantom ISOUND stopped invalid stream at 0x%04x: %s",
			offset, reason);
	_streamInvalid = true;
	_activeTicks = 0;
	_priority = 0;
	_noiseMask = 0;
	_pitchStep = 0;
	_alternationCounter = 0;
	setResultState(-1);
	stopSpeaker();
}

void ISound::initializeDriver() {
	resetDriver();
	_position = _spec.nullSequenceOffset;
	_updatesEnabled = true;
}

void ISound::resetDriver() {
	_streamInvalid = false;
	_sweepInitialized = false;
	_alternationCounter = 0;
	_priority = 0;
	_innerLoopCount = 0;
	_outerLoopCount = 0;
	_noiseMask = 0;
	_currentDivisor = 0;
	_pitchStep = 0;
	_gateOffset = 0;
	_fineOffset = 0;
	_activeTicks = 0;
	_alternationReload = 0;
	_alternationOffset = 0;
	stopSpeaker();
}

int ISound::executeCommonCommand(int commandId) {
	switch (commandId) {
	case 0:
	case 1:
	case 3:
	case 4:
	case 5:
		resetDriver();
		return 0;
	case 2:
		_position = _spec.nullSequenceOffset;
		return 0;
	case 6:
		_updatesEnabled = false;
		stopSpeaker();
		return 0;
	case 7:
		_updatesEnabled = true;
		return 0;
	case 8:
		return _activeTicks;
	default:
		return 0;
	}
}

void ISound::playSequence(uint16 sequenceOffset, byte priority) {
	const bool wasEnabled = _updatesEnabled;
	_updatesEnabled = false;
	if (_priority > priority) {
		_updatesEnabled = wasEnabled;
		return;
	}

	if (sequenceOffset >= _soundData.size()) {
		invalidateStream("command selected an out-of-range sequence", sequenceOffset);
		_updatesEnabled = wasEnabled;
		return;
	}

	_streamInvalid = false;
	_priority = priority;
	_sequenceStart = sequenceOffset;
	_position = sequenceOffset;
	_innerLoopStart = sequenceOffset;
	_outerLoopStart = sequenceOffset;
	_restartOverride = 0;
	_innerLoopCount = 0;
	_outerLoopCount = 0;
	_noiseMask = 0;
	_currentDivisor = 0;
	_pitchStep = 0;
	_gateOffset = 0;
	_fineOffset = 0;
	_transpose = 0;
	_alternationReload = 0;
	_alternationOffset = 0;
	_activeTicks = 1;
	_updatesEnabled = wasEnabled;
}

uint16 ISound::nextRandom() {
	const uint16 value = (uint16)(0x9248 + _randomSeed);
	_randomSeed = (uint16)((value >> 3) | (value << 13));
	return _randomSeed;
}

void ISound::setResultState(int8 state) {
	if (_resultState == state)
		return;
	_resultState = state;
	_pollResult = state;
}

void ISound::processInnerLoop() {
	byte count;
	if (!readByte((uint16)(_position + 1), count))
		return;
	if (!_innerLoopCount) {
		if (!count) {
			_position = (uint16)(_position + 2);
			_innerLoopStart = _position;
		} else {
			_innerLoopCount = count;
			_position = _innerLoopStart;
		}
	} else if (--_innerLoopCount) {
		_position = _innerLoopStart;
	} else {
		_position = (uint16)(_position + 2);
		_innerLoopStart = _position;
	}
}

void ISound::processOuterLoop() {
	byte count;
	if (!readByte((uint16)(_position + 1), count))
		return;
	if (!_outerLoopCount) {
		if (!count) {
			_position = (uint16)(_position + 2);
			_innerLoopStart = _position;
			_outerLoopStart = _position;
			_innerLoopCount = 0;
		} else {
			_outerLoopCount = count;
			_position = _outerLoopStart;
			_innerLoopStart = _outerLoopStart;
		}
	} else if (--_outerLoopCount) {
		_position = _outerLoopStart;
		_innerLoopStart = _outerLoopStart;
	} else {
		_position = (uint16)(_position + 2);
		_outerLoopStart = _position;
		_innerLoopStart = _position;
	}
}

bool ISound::calculateNoteDivisor(byte note, uint16 &divisor) {
	const byte index = (byte)(note + _transpose);
	uint16 tableValue;
	if (!readWord((uint16)(_spec.noteTableOffset + (uint16)index * 2),
			tableValue))
		return false;
	divisor = (uint16)(tableValue + _fineOffset);
	return true;
}

byte ISound::outputVolume() const {
	return (byte)((kDefaultOutputVolume * _masterVolume) / 255);
}

void ISound::outputDivisor(uint16 divisor) {
	_pitRenderer.writeMode3Count(divisor);
}

void ISound::startSpeaker() {
	if (_directDivisor) {
		_currentDivisor = _directDivisor;
	} else if (!calculateNoteDivisor(_note, _currentDivisor)) {
		return;
	}
	_directDivisor = 0;
	_sweepInitialized = false;
	_alternationToggle = false;
	outputDivisor(_currentDivisor);
	_pitRenderer.setControl(true, true);
}

void ISound::stopSpeaker() {
	_pitRenderer.setControl(false, false);
	_sweepInitialized = false;
}

bool ISound::readControlByte(uint16 delta, byte &value) {
	return readByte((uint16)(_position + delta), value);
}

bool ISound::readControlWord(uint16 delta, uint16 &value) {
	return readWord((uint16)(_position + delta), value);
}

bool ISound::isScriptVariableValid(byte index) {
	if (index < kScriptVariableCount)
		return true;
	invalidateStream("script variable index is out of range", _position);
	return false;
}

bool ISound::transferControl(bool take, bool saveReturn) {
	uint16 target;
	if (!readControlWord(3, target))
		return false;
	if (take) {
		if (saveReturn)
			_branchReturn = (uint16)(_position + 5);
		_position = target;
	} else {
		_position = (uint16)(_position + 5);
	}
	return true;
}

void ISound::processOrdinaryEvent() {
	if (!readByte(_position, _note) ||
		!readByte((uint16)(_position + 1), _activeTicks))
		return;
	_position = (uint16)(_position + 2);
	if (!_note || !_activeTicks)
		stopSpeaker();
	if (!_activeTicks) {
		_priority = 0;
		_pitchStep = 0;
		_sweepInitialized = false;
		_alternationCounter = 0;
		setResultState(-1);
		return;
	}
	if (!_note)
		return;
	_releaseCounter = _releaseOverride ? _releaseOverride :
		(byte)(_activeTicks - _gateOffset);
	startSpeaker();
}

bool ISound::processControl(byte opcode) {
	byte a = 0, b = 0;
	uint16 w = 0;

	switch (opcode) {
	case 0xff:
		processInnerLoop();
		break;
	case 0xfe:
		processOuterLoop();
		break;
	case 0xfd:
		if (_restartOverride) {
			_sequenceStart = _restartOverride;
			_position = _restartOverride;
			_innerLoopStart = _restartOverride;
			_outerLoopStart = _restartOverride;
			_restartOverride = 0;
			_innerLoopCount = 0;
			_outerLoopCount = 0;
			_fineOffset = 0;
			_gateOffset = 0;
		} else {
			_position = _sequenceStart;
		}
		break;
	case 0xfc:
		if (!readControlWord(1, w)) return false;
		_sequenceStart = _position = _innerLoopStart = _outerLoopStart =
			_restartOverride = w;
		break;
	case 0xfb:
		if (!readControlWord(1, w)) return false;
		_position = w;
		break;
	case 0xfa:
		if (!readControlWord(1, w)) return false;
		_branchReturn = (uint16)(_position + 3);
		_position = w;
		break;
	case 0xf9:
		if (_branchReturn) {
			_position = _branchReturn;
			_branchReturn = 0;
		} else {
			++_position;
		}
		break;
	case 0xf8:
		if (!readControlWord(1, w)) return false;
		_noiseMask = w;
		setResultState(_noiseMask ? 1 : -1);
		_position = (uint16)(_position + 3);
		break;
	case 0xf7:
		if (!readControlByte(1, a)) return false;
		_gateOffset = a;
		_releaseOverride = 0;
		_position = (uint16)(_position + 2);
		break;
	case 0xf6:
		if (!readControlByte(1, a)) return false;
		_releaseOverride = a;
		_gateOffset = 0;
		_position = (uint16)(_position + 2);
		break;
	case 0xf5:
		if (!readControlWord(1, w)) return false;
		_pitchStep = w;
		_position = (uint16)(_position + 3);
		break;
	case 0xf4:
	case 0xf1:
	case 0xf0:
	case 0xc3:
		_position = (uint16)(_position + 2);
		break;
	case 0xf3:
	case 0xef:
		_position = (uint16)(_position + 3);
		break;
	case 0xf2:
		if (!readControlByte(1, a)) return false;
		_fineOffset = (int8)a;
		_position = (uint16)(_position + 2);
		break;
	case 0xee:
		if (!readControlByte(1, a)) return false;
		_transpose = a;
		_position = (uint16)(_position + 2);
		break;
	case 0xed:
		if (!readControlByte(1, a)) return false;
		_position = (uint16)((int8)a + 3);
		break;
	case 0xec: {
		if (!readControlByte(1, a) || !a) {
			invalidateStream("random table has a zero size", _position);
			return false;
		}
		const uint16 base = (uint16)(_position + 2);
		const uint16 selectedOffset = (uint16)(base +
			(nextRandom() & 0x7fff) % a);
		byte selected, target;
		if (!readByte(selectedOffset, selected) ||
			!readByte((uint16)(base + a), target) ||
			!writeByte((uint16)(base + a + target + 1), selected))
			return false;
		_position = (uint16)(_position + a + 3);
		break;
	}
	case 0xeb: {
		if (!readControlByte(1, a) || !readControlByte(2, b)) return false;
		const int16 range = (int8)b - (int8)a + 1;
		byte target;
		if (range <= 0 || !readByte((uint16)(_position + 3), target)) {
			invalidateStream("random range is invalid", _position);
			return false;
		}
		const byte result = (byte)((nextRandom() & 0x7fff) % range + (int8)a);
		if (!writeByte((uint16)(_position + 4 + target), result)) return false;
		_position = (uint16)(_position + 4);
		break;
	}
	case 0xea: {
		if (!readControlByte(1, a) || !readControlByte(2, b) ||
				!isScriptVariableValid(a)) return false;
		const uint16 base = (uint16)(_position + 3);
		byte selected, target;
		if (!readByte((uint16)(base + _scriptVariables[a]), selected) ||
			!readByte((uint16)(base + b), target) ||
			!writeByte((uint16)(base + target + 1), selected)) return false;
		_position = (uint16)(_position + b + 4);
		break;
	}
	case 0xe9:
		if (!readControlByte(1, a) || !readControlByte(2, b) ||
				!isScriptVariableValid(a)) return false;
		_scriptVariables[a] = b;
		_position = (uint16)(_position + 3);
		break;
	case 0xe8:
		if (!readControlByte(1, a) || !readControlByte(2, b) ||
				!isScriptVariableValid(a) || !isScriptVariableValid(b))
			return false;
		_scriptVariables[a] = _scriptVariables[b];
		_position = (uint16)(_position + 3);
		break;
	case 0xe7:
		if (!readControlByte(1, a) || !readControlByte(2, b) ||
			!isScriptVariableValid(a) ||
			!writeByte((uint16)(_position + 3 + b), _scriptVariables[a])) return false;
		_position = (uint16)(_position + 3);
		break;
	case 0xe6:
	case 0xe5:
		if (!readControlByte(1, a) || !isScriptVariableValid(a)) return false;
		_scriptVariables[a] += opcode == 0xe6 ? 1 : (byte)-1;
		_position = (uint16)(_position + 2);
		break;
	case 0xe4: case 0xe3: case 0xe2: case 0xe1:
	case 0xe0: case 0xdf: case 0xde: case 0xdd:
	case 0xdc: case 0xdb: case 0xda: case 0xd9:
	case 0xd8: case 0xd7: case 0xd6: case 0xd5: {
		if (!readControlByte(1, a) || !readControlByte(2, b) ||
				!isScriptVariableValid(a)) return false;
		const bool usesVariable = (opcode & 1) != 0;
		if (usesVariable && !isScriptVariableValid(b)) return false;
		const byte operand = usesVariable ? _scriptVariables[b] : b;
		byte &destination = _scriptVariables[a];
		switch (opcode) {
		case 0xe4: destination += operand; break;
		case 0xe3: destination += operand; break;
		case 0xe2: destination -= operand; break;
		case 0xe1: destination -= operand; break;
		case 0xe0: destination = (byte)(destination * operand); break;
		case 0xdf: destination = (byte)(destination * operand); break;
		case 0xde: case 0xdd: case 0xdc: case 0xdb:
			if (!destination) {
				invalidateStream("division by zero", _position);
				return false;
			}
			if (opcode == 0xde || opcode == 0xdd)
				destination = (byte)((int8)operand / (int8)destination);
			else
				destination = (byte)((int8)operand % (int8)destination);
			break;
		case 0xda: destination &= operand; break;
		case 0xd9: destination &= operand; break;
		case 0xd8: destination |= operand; break;
		case 0xd7: destination |= operand; break;
		case 0xd6: destination ^= operand; break;
		case 0xd5: destination ^= operand; break;
		default: break;
		}
		_position = (uint16)(_position + 3);
		break;
	}
	case 0xd4: case 0xd3: case 0xd2: case 0xd1:
	case 0xd0: case 0xcf: case 0xce: case 0xcd:
	case 0xcc: case 0xcb: case 0xca: case 0xc9:
	case 0xc8: case 0xc7: case 0xc6: case 0xc5: {
		if (!readControlByte(1, a) || !readControlByte(2, b) ||
				!isScriptVariableValid(a)) return false;
		const bool variablePair =
			(opcode <= 0xd0 && opcode >= 0xcd) || opcode <= 0xc8;
		if (variablePair && !isScriptVariableValid(b)) return false;
		const byte left = variablePair ? _scriptVariables[b] : _scriptVariables[a];
		const byte right = variablePair ? _scriptVariables[a] : b;
		bool take = false;
		switch (opcode) {
		case 0xd4: case 0xd0: case 0xcc: case 0xc8: take = left == right; break;
		case 0xd3: case 0xcf: case 0xcb: case 0xc7: take = left != right; break;
		case 0xd2: case 0xca: take = (int8)left < (int8)right; break;
		case 0xd1: case 0xc9: take = (int8)left > (int8)right; break;
		case 0xce: case 0xc6: take = left > right; break;
		case 0xcd: case 0xc5: take = left < right; break;
		default: break;
		}
		if (!transferControl(take, opcode <= 0xcc)) return false;
		break;
	}
	case 0xc4:
		if (!readControlWord(1, w)) return false;
		warning("Phantom ISOUND ignored native callback 0x%04x", w);
		_position = (uint16)(_position + 3);
		break;
	case 0xc2:
		_position = (uint16)(_position + 4);
		break;
	case 0xc1:
		if (!readControlWord(1, w)) return false;
		_tempoScale = w;
		_position = (uint16)(_position + 2);
		break;
	case 0xc0:
		if (!readControlByte(1, a)) return false;
		_tempoReload = a;
		_position = (uint16)(_position + 2);
		break;
	case 0xbf:
		if (!readControlWord(1, w)) return false;
		_tempoTarget = w;
		_position = (uint16)(_position + 3);
		break;
	case 0xbe:
		if (!readControlByte(1, a)) return false;
		_tempoShift = a;
		_position = (uint16)(_position + 2);
		break;
	case 0xbd:
		if (!readControlByte(1, a) || !readControlByte(2, b)) return false;
		_alternationReload = a;
		_alternationCounter = a;
		_alternationToggle = false;
		_alternationOffset = b;
		_position = (uint16)(_position + 3);
		break;
	case 0xbc:
		if (!readControlWord(1, w)) return false;
		_directDivisor = w;
		_position = (uint16)(_position + 3);
		break;
	case 0xbb:
		warning("Phantom ISOUND ignored unreachable malformed opcode 0xBB");
		invalidateStream("malformed native 0xBB jump-table entry", _position);
		return false;
	default:
		invalidateStream("unknown control opcode", _position);
		return false;
	}

	return !_streamInvalid;
}

void ISound::processSequenceTick() {
	if (!_activeTicks || _streamInvalid)
		return;
	if (_releaseCounter && !--_releaseCounter)
		stopSpeaker();
	if (--_activeTicks)
		return;

	for (uint operation = 0; operation < kMaxOperationsPerTick; ++operation) {
		byte opcode;
		if (!readByte(_position, opcode))
			return;
		if (opcode <= 0xba) {
			processOrdinaryEvent();
			return;
		}
		if (!processControl(opcode))
			return;
	}

	invalidateStream("operation limit exceeded", _position);
}

void ISound::updateAlternation() {
	_alternationCounter = _alternationReload;
	byte offset = 0;
	if (!_alternationToggle) {
		_alternationToggle = true;
		offset = _alternationOffset;
	} else {
		_alternationToggle = false;
	}
	calculateNoteDivisor((byte)(_note + offset), _currentDivisor);
}

void ISound::updatePitch() {
	bool alternationChanged = false;
	if (_alternationCounter && !--_alternationCounter) {
		updateAlternation();
		alternationChanged = true;
	}
	if (!_pitchStep) {
		if (alternationChanged)
			outputDivisor(_currentDivisor);
		return;
	}

	if ((_pitchStep & 0xf000) == 0x8000) {
		const uint16 range = _pitchStep & 0xff;
		const uint16 step = (_pitchStep >> 8) & 0xf;
		if (!_sweepInitialized) {
			_sweepUpper = (uint16)(_currentDivisor + range);
			_sweepLower = (uint16)(_currentDivisor - range);
			_sweepInitialized = true;
		}
		if ((int16)_currentDivisor > (int16)_sweepUpper)
			_sweepDirection = -1;
		else if ((int16)_currentDivisor < (int16)_sweepLower)
			_sweepDirection = 1;
		_currentDivisor = (uint16)(_currentDivisor + step * _sweepDirection);
	} else {
		_currentDivisor = (uint16)(_currentDivisor - _pitchStep);
	}
	outputDivisor(_currentDivisor);
}

void ISound::update() {
	if (!_updatesEnabled)
		return;
	nextRandom();
	++_frameCounter;
	processSequenceTick();
	updatePitch();
}

void ISound::timerTick() {
	update();
	if (_pollResult) {
		_noiseEnabled = _pollResult > 0;
		_pollResult = 0;
	}
}

void ISound::noiseTick() {
	if (!_noiseMask)
		return;
	outputDivisor((uint16)((nextRandom() & _noiseMask) + _currentDivisor));
}

int ISound::poll() {
	return 0;
}

void ISound::noise() {
	Common::StackLock lock(_driverMutex);
	noiseTick();
}

int ISound::stop() {
	Common::StackLock lock(_driverMutex);
	resetDriver();
	const int result = _pollResult;
	_pollResult = 0;
	return result;
}

void ISound::setVolume(int volume) {
	Common::StackLock lock(_driverMutex);
	_masterVolume = CLIP(volume, 0, 255);
}

int ISound::readBuffer(int16 *buffer, int numSamples) {
	Common::StackLock lock(_driverMutex);
	const uint64 serviceThreshold = (uint64)_outputRate *
		kHostTimerDivisor * kHostServiceDivider;
	assert(serviceThreshold > kPitClockHz);

	for (int sample = 0; sample < numSamples; ++sample) {
		const uint64 previousHostTimerAccumulator = _hostTimerAccumulator;
		_hostTimerAccumulator += kPitClockHz;
		if (_hostTimerAccumulator >= serviceThreshold) {
			// Apply host writes at their fractional position inside this output
			// sample so the shared PIT renderer preserves transition timing.
			const uint64 servicePosition = serviceThreshold -
				previousHostTimerAccumulator;
			assert(servicePosition <= kPitClockHz);
			_pitRenderer.advanceToSampleFraction(
				(uint32)servicePosition, kPitClockHz);
			_hostTimerAccumulator -= serviceThreshold;

			// The native host services noise before polling the sequence VM.
			// A poll result therefore changes noise on the following service.
			if (_noiseEnabled)
				noiseTick();
			if (!--_sequenceServiceCountdown) {
				_sequenceServiceCountdown = kSequenceServiceDivider;
				timerTick();
			}
		}
		buffer[sample] = _pitRenderer.generateSample(outputVolume());
	}
	return numSamples;
}

} // namespace Sound
} // namespace Phantom
} // namespace MADS
