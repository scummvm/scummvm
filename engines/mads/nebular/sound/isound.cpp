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
 *
 */

#include "common/endian.h"
#include "common/file.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "mads/nebular/sound/isound.h"

namespace MADS {
namespace RexNebular {
namespace Sound {

namespace {

const uint32 kMinimumDataSegmentSize =
ISound::kFrequencyTableOffset +
ISound::kFrequencyTableEntries * 2;

} // namespace

ISound::OverlayLayout ISound::readOverlayLayout(
	const Common::Path &filename) {
	Common::File file;
	if (!file.open(filename))
		error("Could not open file - %s", filename.toString().c_str());

	const int64 fileSize = file.size();
	if (fileSize < 0x200)
		error("ISOUND overlay is too small - %s",
			filename.toString().c_str());

	file.seek(0);
	if (file.readUint16LE() != 0x5a4d)
		error("ISOUND overlay is not an MZ executable - %s",
			filename.toString().c_str());

	const uint16 bytesOnLastPage = file.readUint16LE();
	const uint16 pageCount = file.readUint16LE();
	if (!pageCount || bytesOnLastPage >= 512)
		error("Invalid ISOUND MZ file-size fields - %s",
			filename.toString().c_str());

	const uint32 declaredFileSize = (pageCount - 1) * 512U +
		(bytesOnLastPage ? bytesOnLastPage : 512U);
	if (declaredFileSize != (uint64)fileSize)
		error("ISOUND MZ file-size fields do not match - %s",
			filename.toString().c_str());

	file.seek(8);
	const uint32 imageOffset = file.readUint16LE() * 16U;
	if (imageOffset < 0x1c || imageOffset + 0x48 >(uint64)fileSize)
		error("Invalid ISOUND MZ header - %s",
			filename.toString().c_str());

	file.seek(imageOffset + 0x2a);
	const uint32 dataSegmentOffset = file.readUint16LE() * 16U;
	const uint16 dataSegmentSize = file.readUint16LE();
	const uint16 timerHz = file.readUint16LE();
	const uint16 exportCount = file.readUint16LE();
	const uint32 dataOffset = imageOffset + dataSegmentOffset;

	if (dataOffset >= (uint64)fileSize)
		error("Invalid ISOUND data-segment offset - %s",
			filename.toString().c_str());

	OverlayLayout result;
	result.dataOffset = dataOffset;
	result.initializedDataSize = (uint32)(fileSize - dataOffset);
	result.dataSegmentSize = dataSegmentSize;

	if (result.dataSegmentSize < result.initializedDataSize ||
		result.dataSegmentSize < kMinimumDataSegmentSize ||
		timerHz != 100 || exportCount != 11)
		error("Unsupported ISOUND overlay layout - %s",
			filename.toString().c_str());

	return result;
}

ISound::ISound(Audio::Mixer *mixer, const Common::Path &filename) :
	ISound(mixer, filename, readOverlayLayout(filename)) {
}

ISound::ISound(Audio::Mixer *mixer, const Common::Path &filename, const OverlayLayout &layout) :
		SoundDriver(mixer, filename, (int)layout.dataOffset, (int)layout.initializedDataSize),
	_speakerGate(false),
	_noiseEnabled(false),
	_updatesEnabled(false),
	_masterVolume(255),
	_outputRate(mixer->getOutputRate()),
	_sequenceAccumulator(0),
	_noiseAccumulator(0),
	_oscillatorPhase(0),
	_frameCounter(0),
	_randomSeed(0),
	_commandParam(0),
	_pollResult(0),
	_resultState(0),
	_priority(0),
	_sequenceStart(0),
	_position(0),
	_innerLoopStart(0),
	_outerLoopStart(0),
	_restartOverride(0),
	_innerLoopCount(0),
	_outerLoopCount(0),
	_note(0),
	_activeTicks(0),
	_releaseCounter(0),
	_gateOffset(0),
	_transpose(0),
	_fineOffset(0),
	_noiseMask(0),
	_currentDivisor(0),
	_lastOutputDivisor(0),
	_pendingDivisor(0),
	_hasPendingDivisor(false),
	_speakerHigh(true),
	_pitchStep(0),
	_directDivisor(0),
	_alternationReload(0),
	_alternationOffset(0),
	_alternationCounter(0),
	_alternationToggle(false),
	_sweepInitialized(false),
	_sweepUpper(0),
	_sweepLower(0),
	_sweepDirection(1) {
	// The descriptor size includes a zero-initialized tail absent from the
	// executable image. Preserve the original data/BSS layout.
	_soundData.resize(layout.dataSegmentSize);
	_randomSeed = readSequenceUint16(0x00f9);

	initializeDriver();
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_speakerHandle,
		this, -1, Audio::Mixer::kMaxChannelVolume, 0,
		DisposeAfterUse::NO, true);
}

ISound::~ISound() {
	_mixer->stopHandle(_speakerHandle);
}

byte ISound::readSequenceByte(uint16 offset) const {
	if (offset >= _soundData.size())
		error("ISOUND sequence offset 0x%04x is out of range", offset);
	return _soundData[offset];
}

uint16 ISound::readSequenceUint16(uint16 offset) const {
	if ((uint32)offset + 1 >= _soundData.size())
		error("ISOUND word offset 0x%04x is out of range", offset);
	return READ_LE_UINT16(&_soundData[offset]);
}

void ISound::writeSequenceByte(uint16 offset, byte value) {
	if (offset >= _soundData.size())
		error("ISOUND mutation offset 0x%04x is out of range", offset);
	_soundData[offset] = value;
}

void ISound::initializeDriver() {
	// Export 0 resets the driver, installs the null sequence at DS:00F0 with
	// priority 0x64, and only then enables update processing.
	resetDriver();
	playSequence(kInitialNullSequenceOffset, 0x64);
	_updatesEnabled = true;
}

void ISound::resetDriver() {
	// The native reset at image 0x0146 is intentionally partial.
	_sweepInitialized = false;
	_alternationCounter = 0;
	_priority = 0;
	_innerLoopCount = 0;
	_outerLoopCount = 0;
	_noiseMask = 0;
	_currentDivisor = 0;
	_pendingDivisor = 0;
	_hasPendingDivisor = false;
	_pitchStep = 0;
	_gateOffset = 0;
	_fineOffset = 0;
	_activeTicks = 0;
	_alternationReload = 0;
	_alternationOffset = 0;

	stopSpeaker();
}

void ISound::beginCommand(int param) {
	// The dispatcher zeroes AH and stores only the parameter's low byte.
	_commandParam = (byte)param;
}

int ISound::executeCommonCommand(int commandId) {
	switch (commandId) {
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		resetDriver();
		return 0;
	case 6:
		_updatesEnabled = false;
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

	// Native code uses CMP followed by JS. Preserve the sign bit of the
	// wrapped eight-bit subtraction rather than using a wider comparison.
	if ((int8)(byte)(priority - _priority) < 0) {
		_updatesEnabled = wasEnabled;
		return;
	}

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
	_directDivisor = 0;
	_gateOffset = 0;
	_fineOffset = 0;
	_transpose = 0;
	_alternationReload = 0;
	_alternationOffset = 0;
	_alternationCounter = 0;
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
	const uint16 commandPosition = _position;

	if (!_innerLoopCount) {
		const byte count = readSequenceByte((uint16)(commandPosition + 1));
		if (!count) {
			_position = (uint16)(commandPosition + 2);
			_innerLoopStart = _position;
			_innerLoopCount = 0;
		} else {
			_innerLoopCount = count;
			_position = _innerLoopStart;
		}
	} else if (--_innerLoopCount) {
		_position = _innerLoopStart;
	} else {
		_position = (uint16)(commandPosition + 2);
		_innerLoopStart = _position;
	}
}

void ISound::processOuterLoop() {
	const uint16 commandPosition = _position;

	if (!_outerLoopCount) {
		const byte count = readSequenceByte((uint16)(commandPosition + 1));
		if (!count) {
			_position = (uint16)(commandPosition + 2);
			_innerLoopStart = _position;
			_outerLoopStart = _position;
			_innerLoopCount = 0;
			_outerLoopCount = 0;
		} else {
			_outerLoopCount = count;
			_position = _outerLoopStart;
		}
	} else if (--_outerLoopCount) {
		// The original outer-loop repeat uses the current inner-loop anchor.
		_position = _innerLoopStart;
	} else {
		_position = (uint16)(commandPosition + 2);
		_outerLoopStart = _position;
		_innerLoopStart = _position;
	}
}

void ISound::processRestart() {
	if (!_restartOverride) {
		_position = _sequenceStart;
		return;
	}

	_sequenceStart = _restartOverride;
	_position = _restartOverride;
	_innerLoopStart = _restartOverride;
	_outerLoopStart = _restartOverride;
}

void ISound::processRandomMutation() {
	const uint16 commandPosition = _position;
	const byte count = readSequenceByte((uint16)(commandPosition + 1));
	if (!count)
		error("Invalid zero-length ISOUND F8 mutation at 0x%04x",
			commandPosition);

	const uint16 choices = (uint16)(commandPosition + 2);
	const byte index = (byte)(nextRandom() & (byte)(count - 1));
	const byte selected = readSequenceByte((uint16)(choices + index));
	const byte destination = readSequenceByte((uint16)(choices + count));

	// The target expression is evaluated in AL, so the relative byte wraps
	// before it is added to the 16-bit choice-table address.
	const byte relative = (byte)(
		destination + (byte)(count - 1) + 2);
	writeSequenceByte((uint16)(choices + relative), selected);

	_position = (uint16)(commandPosition + count + 3);
}

uint16 ISound::calculateNoteDivisor(byte note) const {
	const byte tableIndex = (byte)(note + _transpose);
	if (tableIndex >= kFrequencyTableEntries)
		error("ISOUND note-table index %u is out of range", tableIndex);

	const uint32 tableOffset =
		kFrequencyTableOffset + (uint32)tableIndex * 2;
	const uint16 divisor = READ_LE_UINT16(&_soundData[tableOffset]);
	return (uint16)(divisor + _fineOffset);
}

byte ISound::outputVolume() const {
	return (byte)((kDefaultOutputVolume * _masterVolume) / 255);
}

void ISound::outputDivisor(uint16 divisor) {
	// Reprogramming an 8254 counter in mode 3 does not restart the current
	// half-cycle. The new count is loaded at the next output transition.
	// This is especially important while the noise service rewrites the
	// divisor: restarting the waveform on every write creates false tones.
	if (_speakerGate && _lastOutputDivisor) {
		_pendingDivisor = divisor;
		_hasPendingDivisor = true;
	} else {
		_lastOutputDivisor = divisor;
		_pendingDivisor = 0;
		_hasPendingDivisor = false;
		_oscillatorPhase = 0;
		_speakerHigh = true;
	}
}

void ISound::startSpeaker() {
	_currentDivisor = _directDivisor ?
		_directDivisor : calculateNoteDivisor(_note);
	_directDivisor = 0;

	_sweepInitialized = false;
	_alternationToggle = false;
	outputDivisor(_currentDivisor);
	_speakerGate = true;
}

void ISound::stopSpeaker() {
	_speakerGate = false;
	_sweepInitialized = false;
	_hasPendingDivisor = false;
	_oscillatorPhase = 0;
	_speakerHigh = true;
}

void ISound::processOrdinaryEvent() {
	_note = readSequenceByte(_position);
	_activeTicks = readSequenceByte((uint16)(_position + 1));
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

	// Note zero with nonzero duration is a timed rest.
	if (!_note)
		return;

	_releaseCounter = (byte)(_activeTicks - _gateOffset);
	startSpeaker();
}

void ISound::processSequenceTick() {
	if (!_activeTicks)
		return;

	if (_releaseCounter && !--_releaseCounter)
		stopSpeaker();

	if (--_activeTicks)
		return;

	for (uint operation = 0; operation < kMaxOperationsPerTick; ++operation) {
		const byte opcode = readSequenceByte(_position);
		if (opcode < 0xf0) {
			processOrdinaryEvent();
			return;
		}

		switch (opcode) {
		case 0xff:
			processInnerLoop();
			break;
		case 0xfe:
			processOuterLoop();
			break;
		case 0xfd:
			processRestart();
			break;
		case 0xfc:
			_noiseMask = readSequenceUint16((uint16)(_position + 1));
			setResultState(_noiseMask ? 1 : -1);
			_pitchStep = readSequenceUint16((uint16)(_position + 3));
			_position = (uint16)(_position + 5);
			break;
		case 0xfb:
			_gateOffset = readSequenceByte((uint16)(_position + 1));
			_position = (uint16)(_position + 2);
			break;
		case 0xfa:
			_pitchStep = readSequenceUint16((uint16)(_position + 1));
			_position = (uint16)(_position + 3);
			break;
		case 0xf9:
			_fineOffset = (int8)readSequenceByte(
				(uint16)(_position + 1));
			_position = (uint16)(_position + 2);
			break;
		case 0xf8:
			processRandomMutation();
			break;
		case 0xf7:
			_transpose = readSequenceByte((uint16)(_position + 1));
			_position = (uint16)(_position + 2);
			break;
		case 0xf6:
			_alternationReload = readSequenceByte(
				(uint16)(_position + 1));
			_alternationCounter = _alternationReload;
			_alternationToggle = false;
			_alternationOffset = readSequenceByte(
				(uint16)(_position + 2));
			_position = (uint16)(_position + 3);
			break;
		case 0xf5:
			_directDivisor = readSequenceUint16(
				(uint16)(_position + 1));
			_position = (uint16)(_position + 3);
			break;
		case 0xf4:
		case 0xf3:
		case 0xf1:
			_position = (uint16)(_position + 2);
			break;
		case 0xf2:
		case 0xf0:
			_position = (uint16)(_position + 3);
			break;
		}
	}

	error("ISOUND sequence exceeded %u operations in one tick",
		kMaxOperationsPerTick);
}

void ISound::updateAlternation() {
	_alternationCounter = _alternationReload;

	byte alternatingOffset = 0;
	if (!_alternationToggle) {
		_alternationToggle = true;
		alternatingOffset = _alternationOffset;
	} else {
		_alternationToggle = false;
	}

	const byte tableIndex = (byte)(
		_note + _transpose + alternatingOffset);
	if (tableIndex >= kFrequencyTableEntries)
		error("ISOUND alternating note-table index %u is out of range",
			tableIndex);

	const uint32 tableOffset =
		kFrequencyTableOffset + (uint32)tableIndex * 2;
	_currentDivisor = (uint16)(
		READ_LE_UINT16(&_soundData[tableOffset]) + _fineOffset);
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
		const uint16 range = _pitchStep & 0x00ff;
		const uint16 step = (_pitchStep >> 8) & 0x000f;

		if (!_sweepInitialized) {
			_sweepUpper = (uint16)(_currentDivisor + range);
			_sweepLower = (uint16)(_currentDivisor - range);
			_sweepInitialized = true;
		}

		if ((int16)_currentDivisor > (int16)_sweepUpper)
			_sweepDirection = -1;
		else if ((int16)_currentDivisor < (int16)_sweepLower)
			_sweepDirection = 1;

		_currentDivisor = (uint16)(
			_currentDivisor + step * _sweepDirection);
	} else {
		_currentDivisor = (uint16)(_currentDivisor + _pitchStep);
	}

	outputDivisor(_currentDivisor);
}

void ISound::update() {
	++_frameCounter;
	nextRandom();

	// Command 6 freezes both stream and pitch/gate processing. The frame
	// counter and random state continue to advance.
	if (!_updatesEnabled)
		return;

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

	// Export 4 changes the emitted divisor without replacing DS:00E0.
	const uint16 divisor = (uint16)(
		(nextRandom() & _noiseMask) + _currentDivisor);
	outputDivisor(divisor);
}

int ISound::poll() {
	// Playback advances from the audio stream at the overlay's 100 Hz rate.
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

int16 ISound::generateSample() {
	if (!_speakerGate)
		return 0;

	// A PIT divisor of zero represents 65536.
	_oscillatorPhase += kPitClockHz;

	for (;;) {
		const uint32 effectiveDivisor =
			_lastOutputDivisor ? _lastOutputDivisor : 0x10000;
		const uint32 halfCount = _speakerHigh ?
			(effectiveDivisor + 1) / 2 : effectiveDivisor / 2;
		const uint64 halfPeriod = (uint64)MAX<uint32>(halfCount, 1) *
			_outputRate;
		if (_oscillatorPhase < halfPeriod)
			break;

		_oscillatorPhase -= halfPeriod;
		_speakerHigh = !_speakerHigh;
		if (_hasPendingDivisor) {
			_lastOutputDivisor = _pendingDivisor;
			_pendingDivisor = 0;
			_hasPendingDivisor = false;
		}
	}

	if (!_masterVolume)
		return 0;

	const int amplitude = 127 * outputVolume();
	return _speakerHigh ? amplitude : -amplitude;
}

int ISound::readBuffer(int16 *buffer, int numSamples) {
	Common::StackLock lock(_driverMutex);

	for (int sample = 0; sample < numSamples; ++sample) {
		_sequenceAccumulator += kSequenceRateHz;
		if (_sequenceAccumulator >= (uint32)_outputRate) {
			_sequenceAccumulator -= _outputRate;
			timerTick();
		}

		_noiseAccumulator += kNoiseRateHz;
		if (_noiseAccumulator >= (uint32)_outputRate) {
			_noiseAccumulator -= _outputRate;

			// The overlay proves that noise is serviced independently from
			// its 100 Hz sequencer, but not the host interval. The original
			// capture is consistent with the surviving MADS 60 Hz service
			// cadence; the 600 Hz counter is a timing clock.
			if (_noiseEnabled)
				noiseTick();
		}

		buffer[sample] = generateSample();
	}

	return numSamples;
}

} // namespace Sound
} // namespace RexNebular
} // namespace MADS
