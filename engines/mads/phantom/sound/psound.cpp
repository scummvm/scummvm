/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT file.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "audio/fmopl.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/func.h"
#include "common/md5.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "mads/phantom/sound/psound.h"

namespace MADS {
namespace Phantom {
namespace Sound {

namespace {

const int kHostCallbackRateHz =
		NativeSoundTimer::kPitClockHz / NativeSoundTimer::kHostTimerDivisor;

int clampLevel(int value) {
	return CLIP(value, 0, 63);
}

byte panningBits(byte panning) {
	if (panning < 0x2b)
		return 0x10;
	if (panning < 0x55)
		return 0x30;
	return 0x20;
}

} // namespace

void PSound::Channel::reset() {
	memset(this, 0, sizeof(*this));
	volumeFadeReload = 0xff;
	panning = 0x40;
}

void PSound::Channel::load(uint16 sequenceOffset) {
	reset();
	loopStart = sequenceOffset;
	position = sequenceOffset;
	innerLoopStart = sequenceOffset;
	outerLoopStart = sequenceOffset;
	originalSequence = sequenceOffset;
	activeCount = 1;
}

bool PSound::validateFile(const PSoundDriverData &driverData,
						  const char *first8192Md5, Common::String *reason) {
	Common::File file;
	if (!file.open(driverData.filename)) {
		if (reason)
			*reason = "file is missing";
		return false;
	}
	const uint32 minimumSize = driverData.dataOffset +
			driverData.initializedDataSize;
	if ((uint32)file.size() < minimumSize) {
		if (reason)
			*reason = "initialized data is truncated";
		return false;
	}

	file.seek(0);
	const Common::String md5 = Common::computeStreamMD5AsString(file, 8192);
	if (md5 != first8192Md5) {
		if (reason)
			*reason = "first-8192-byte signature does not match";
		return false;
	}
	return true;
}

PSound::PSound(Audio::Mixer *mixer, const PSoundDriverData &driverData) :
		SoundDriver(mixer, driverData.filename, driverData.dataOffset,
				driverData.initializedDataSize), _opl(nullptr) {
	_masterVolume = 255;
	_randomSeed = 0;
	_frameCounter = 0;
	_pollResult = 0;
	_resultFlag = 0;
	_nullSequenceOffset = driverData.nullSequenceOffset;
	_patchTableOffset = driverData.patchTableOffset;
	_tableLayout = driverData.tables;
	_patchCount = driverData.patchCount;
	_updatesEnabled = false;
	_noiseServiceEnabled = false;
	_tickEnabled = 1;
	_tickCounter = 0;
	_tempoReload = 0;
	_tempoTarget = 0;
	_tempoShift = 0;
	_tempoBase = 0xa0;
	_tempoCurrent = 0x28;
	_tempoScale = 0x0a;
	_frameNumber2 = 0;
	if (_soundData.size() != (uint32)driverData.initializedDataSize)
		error("Phantom PSOUND initialized data has unexpected size %u (expected %d)",
			  (uint)_soundData.size(), driverData.initializedDataSize);
	if (driverData.totalDataSize < driverData.initializedDataSize ||
		driverData.totalDataSize > 0xffff)
		error("Phantom PSOUND has invalid mutable data size %d",
			  driverData.totalDataSize);

	const uint32 initializedSize = _soundData.size();
	_soundData.resize(driverData.totalDataSize);
	if (_soundData.size() > initializedSize)
		memset(&_soundData[initializedSize], 0,
			   _soundData.size() - initializedSize);

	if (!_patchCount ||
		_patchTableOffset + (uint32)_patchCount * kPatchSize > initializedSize)
		error("Phantom PSOUND patch table is outside initialized data");
	if ((uint32)driverData.randomSeedOffset + 2 > initializedSize)
		error("Phantom PSOUND random seed is outside initialized data");
	if ((uint32)_tableLayout.panning + 0x80 > initializedSize ||
		(uint32)_tableLayout.frequency + 24 > initializedSize ||
		(uint32)_tableLayout.bank + kChannelCount > initializedSize ||
		(uint32)_tableLayout.channel + kChannelCount > initializedSize ||
		(uint32)_tableLayout.operators + kChannelCount * 4 > initializedSize)
		error("Phantom PSOUND lookup table is outside initialized data");
	if (_nullSequenceOffset >= initializedSize)
		error("Phantom PSOUND null sequence is outside initialized data");

	for (uint channel = 0; channel < kChannelCount; ++channel) {
		const byte banks = getBankMask(channel);
		if (!banks || (banks & ~kBothBanks))
			error("Phantom PSOUND has invalid bank routing for channel %u", channel);
		if (getOplChannel(channel) > 8)
			error("Phantom PSOUND has invalid OPL routing for channel %u", channel);
		for (uint op = 0; op < 4; ++op) {
			if (getOperatorOffset(channel, op) > 0x15)
				error("Phantom PSOUND has invalid operator routing for channel %u",
					  channel);
		}
		_channels[channel].reset();
	}
	memset(_scriptVars, 0, sizeof(_scriptVars));
	memset(_registerCache, 0, sizeof(_registerCache));
	_randomSeed = readDataUint16(driverData.randomSeedOffset);

	// The original overlays select this path only for PAS16 card types
	// 20h-22h. ScummVM intentionally implements that native branch and not the
	// older dual-OPL2 branch in the same binaries.
	_opl = OPL::Config::create(OPL::Config::kOpl3);
	if (!_opl || !_opl->init()) {
		delete _opl;
		_opl = nullptr;
		return;
	}
	initializePas16();
	resetDriver();
	_updatesEnabled = true;
	_opl->start(new Common::Functor0Mem<void, PSound>(this,
													  &PSound::onTimer),
				kHostCallbackRateHz);
}

PSound::~PSound() {
	if (_opl) {
		_opl->stop();
		shutdownPas16();
		delete _opl;
	}
}

bool PSound::isDataRangeValid(uint32 offset, uint32 length) const {
	return offset <= _soundData.size() && length <= _soundData.size() - offset;
}

const byte *PSound::getDataPointer(uint32 offset, uint32 length,
								   const char *operation) const {
	if (!isDataRangeValid(offset, length))
		error("Phantom PSOUND %s outside data image: offset 0x%04x, length %u",
			  operation, (uint)offset, (uint)length);
	return &_soundData[offset];
}

byte *PSound::getDataPointer(uint32 offset, uint32 length,
							 const char *operation) {
	return const_cast<byte *>(static_cast<const PSound *>(this)->getDataPointer(offset, length, operation));
}

byte PSound::readDataByte(uint32 offset) const {
	return *getDataPointer(offset, 1, "byte read");
}

uint16 PSound::readDataUint16(uint32 offset) const {
	return READ_LE_UINT16(getDataPointer(offset, 2, "word read"));
}

void PSound::writeDataByte(uint32 offset, byte value) {
	*getDataPointer(offset, 1, "byte write") = value;
}

void PSound::writeDataUint16(uint32 offset, uint16 value) {
	WRITE_LE_UINT16(getDataPointer(offset, 2, "word write"), value);
}

byte PSound::getBankMask(uint channel) const {
	return readDataByte(_tableLayout.bank + channel);
}

byte PSound::getOplChannel(uint channel) const {
	return readDataByte(_tableLayout.channel + channel);
}

byte PSound::getOperatorOffset(uint channel, uint operatorIndex) const {
	return readDataByte(_tableLayout.operators + channel * 4 + operatorIndex);
}

const byte *PSound::getPatch(uint patchIndex) const {
	if (patchIndex >= _patchCount)
		patchIndex = 0;
	return getDataPointer(_patchTableOffset + (uint32)patchIndex * kPatchSize,
						  kPatchSize, "patch");
}

byte PSound::getPanningAttenuation(byte panning) const {
	return readDataByte(_tableLayout.panning + (panning & 0x7f));
}

uint16 PSound::getFrequencyNumber(byte semitone) const {
	return readDataUint16(_tableLayout.frequency + (semitone % 12) * 2);
}

void PSound::writeRegister(byte banks, byte reg, byte value) {
	assert(_opl);
	assert(banks && !(banks & ~kBothBanks));
	if (banks & kFirstBank) {
		_registerCache[0][reg] = value;
		_opl->writeReg(reg, value);
	}
	if (banks & kSecondBank) {
		_registerCache[1][reg] = value;
		_opl->writeReg(0x100 | reg, value);
	}
}

byte PSound::getCachedRegister(byte banks, byte reg) const {
	assert(banks && !(banks & ~kBothBanks));
	return _registerCache[(banks & kFirstBank) ? 0 : 1][reg];
}

void PSound::initializePas16() {
	writeRegister(kSecondBank, 0x05, 0x01);
	writeRegister(kSecondBank, 0x04, 0x3f);
}

void PSound::shutdownPas16() {
	writeRegister(kSecondBank, 0x04, 0);
	writeRegister(kSecondBank, 0x05, 0);
}

void PSound::resetDriver() {
	const bool wasEnabled = _updatesEnabled;
	_updatesEnabled = false;
	for (uint i = 0; i < kChannelCount; ++i)
		_channels[i].reset();
	memset(_scriptVars, 0, sizeof(_scriptVars));
	_resultFlag = 0;
	_pollResult = 0;

	// OPL3 mode is established only at driver initialization, matching the
	// original card-selection path rather than being re-enabled by command 0.
	for (int reg = 0x56; reg >= 0x40; --reg)
		writeRegister(kBothBanks, reg, 0x3f);
	for (int reg = 0xff; reg >= 0x60; --reg)
		writeRegister(kBothBanks, reg, 0);
	for (int reg = 0x3f; reg >= 0x20; --reg)
		writeRegister(kBothBanks, reg, 0);
	writeRegister(kBothBanks, 0x01, 0x20);
	writeRegister(kBothBanks, 0xbd, 0xc0);

	_updatesEnabled = wasEnabled;
}

void PSound::requestStop(uint firstChannel, uint endChannel) {
	for (uint i = firstChannel; i < endChannel; ++i) {
		if (_channels[i].activeCount) {
			_channels[i].pendingStop = 0xff;
			_channels[i].originalSequence = 0xffff;
		}
	}
}

void PSound::setCurrentSequence(uint firstChannel, uint endChannel,
								uint16 sequenceOffset) {
	getDataPointer(sequenceOffset, 1, "sequence position");
	for (uint i = firstChannel; i < endChannel; ++i)
		_channels[i].position = sequenceOffset;
}

void PSound::loadChannel(uint channel, uint16 sequenceOffset) {
	assert(channel < kChannelCount);
	getDataPointer(sequenceOffset, 1, "sequence start");
	_channels[channel].load(sequenceOffset);
}

void PSound::playSound(uint16 sequenceOffset) {
	// Native effect allocator: free 7,8,6; then pending-stop 6,7,8.
	static const byte freeOrder[] = {7, 8, 6};
	static const byte pendingOrder[] = {6, 7, 8};
	for (uint i = 0; i < ARRAYSIZE(freeOrder); ++i) {
		const byte channel = freeOrder[i];
		if (!_channels[channel].activeCount) {
			loadChannel(channel, sequenceOffset);
			return;
		}
	}
	for (uint i = 0; i < ARRAYSIZE(pendingOrder); ++i) {
		const byte channel = pendingOrder[i];
		if (_channels[channel].pendingStop == 0xff) {
			loadChannel(channel, sequenceOffset);
			return;
		}
	}
}

void PSound::playMusicAny(uint16 sequenceOffset) {
	// This native section helper is music-only despite its generic role in the
	// original code: it never considers effect channels 6-8.
	for (uint i = 0; i < kMusicChannelCount; ++i) {
		if (!_channels[i].activeCount) {
			loadChannel(i, sequenceOffset);
			return;
		}
	}
	for (uint i = 0; i < kMusicChannelCount; ++i) {
		if (_channels[i].pendingStop == 0xff) {
			loadChannel(i, sequenceOffset);
			return;
		}
	}
}

bool PSound::isSoundActive(uint16 sequenceOffset) const {
	for (uint i = 0; i < kChannelCount; ++i) {
		if (_channels[i].activeCount &&
			_channels[i].originalSequence == sequenceOffset)
			return true;
	}
	return false;
}

uint16 PSound::nextRandom() {
	const uint16 value = 0x9248 + _randomSeed;
	_randomSeed = (value >> 3) | (value << 13);
	return _randomSeed;
}

int PSound::command0() {
	resetDriver();
	return 0;
}

int PSound::command1() {
	command3();
	command5();
	return 0;
}

int PSound::command2() {
	setCurrentSequence(0, kMusicChannelCount, _nullSequenceOffset);
	return 0;
}

int PSound::command3() {
	requestStop(0, kMusicChannelCount);
	return 0;
}

int PSound::command4() {
	setCurrentSequence(kMusicChannelCount, kChannelCount, _nullSequenceOffset);
	return 0;
}

int PSound::command5() {
	requestStop(kMusicChannelCount, kChannelCount);
	return 0;
}

int PSound::command6() {
	for (uint i = 0; i < kChannelCount; ++i) {
		_channels[i].savedNoiseTicks = _channels[i].noiseTicks;
		_channels[i].noiseTicks = 0;
		keyOff(i);
	}
	_updatesEnabled = false;
	return 0;
}

int PSound::command7() {
	_updatesEnabled = true;
	for (uint i = 0; i < kChannelCount; ++i) {
		Channel &channel = _channels[i];
		channel.noiseTicks = channel.savedNoiseTicks;
		if (channel.activeCount) {
			updateChannelLevels(i);
			updateChannelFrequency(i, true);
		}
	}
	bool anyNoise = false;
	for (uint i = 0; i < kChannelCount; ++i)
		anyNoise |= _channels[i].noiseTicks != 0;
	if (anyNoise)
		resultCheck();
	return _channels[kChannelCount - 1].savedNoiseTicks;
}

int PSound::command8() {
	int result = 0;
	for (uint i = 0; i < kChannelCount; ++i)
		result |= _channels[i].activeCount;
	return result;
}

void PSound::onTimer() {
	Common::StackLock lock(_driverMutex);
	uint32 serviceTicks = _hostTimer.advance(1, kHostCallbackRateHz);
	while (serviceTicks--) {
		if (_noiseServiceEnabled)
			serviceNoise();
		if (_hostTimer.pollDue()) {
			const int result = serviceUpdate();
			if (result)
				_noiseServiceEnabled = result > 0;
		}
	}
}

int PSound::serviceUpdate() {
	update();
	const int result = _pollResult;
	_pollResult = 0;
	return result;
}

void PSound::serviceNoise() {
	for (int i = kChannelCount - 1; i >= 0; --i) {
		Channel &channel = _channels[i];
		if (channel.noiseTicks) {
			const uint16 random = nextRandom();
			setNoiseFrequency(i,
							  (random & channel.noiseMask) + channel.noiseBase);
		}
	}
}

void PSound::update() {
	if (!_updatesEnabled)
		return;
	nextRandom();
	++_frameNumber2;
	++_frameCounter;
	for (uint i = 0; i < kChannelCount; ++i)
		updateChannel(i);
	checkPendingStops();

	bool anyNoise = false;
	for (int i = kChannelCount - 1; i >= 0; --i) {
		Channel &channel = _channels[i];
		if (!channel.noiseTicks)
			continue;
		anyNoise = true;
		channel.noiseBase += channel.noiseStep;
		if (!--channel.noiseTicks)
			keyOff(i);
	}
	if (!anyNoise && _resultFlag != -1) {
		_resultFlag = -1;
		_pollResult = -1;
	}
}

void PSound::updateChannel(uint channelIndex) {
	Channel &channel = _channels[channelIndex];
	if (!channel.activeCount)
		return;

	if (channel.keyOnDelay && --channel.keyOnDelay == 0)
		keyOff(channelIndex);

	if (--channel.activeCount == 0) {
		bool levelsDirty = false;
		int budget = kOpcodeBudgetPerTick;
		while (budget-- > 0) {
			if (!isDataRangeValid(channel.position, 1)) {
				finishChannel(channelIndex);
				break;
			}
			const byte value = readDataByte(channel.position);
			if (value <= 0xbd) {
				if (!isDataRangeValid(channel.position, 2)) {
					finishChannel(channelIndex);
					break;
				}
				if (levelsDirty)
					updateChannelLevels(channelIndex);
				channel.note = value;
				channel.activeCount = readDataByte(channel.position + 1);
				channel.position += 2;
				if (!channel.note || !channel.activeCount) {
					keyOff(channelIndex);
					if (!channel.activeCount)
						finishChannel(channelIndex);
				} else {
					channel.keyOnDelay = channel.durationOverride ? channel.durationOverride : (byte)(channel.activeCount - channel.noteOffset);
					updateChannelFrequency(channelIndex, true);
				}
				break;
			}

			if (!executeOpcode(channelIndex, value, levelsDirty)) {
				finishChannel(channelIndex);
				break;
			}
		}
		if (budget < 0 && !channel.activeCount)
			finishChannel(channelIndex);
	}

	if (channel.pitchBend)
		updatePitchBend(channelIndex);

	bool levelsDirty = false;
	if (channel.volumeFadeCounter || channel.panningFadeCounter) {
		if (--channel.volumeFadeCounter == 0) {
			channel.volumeFadeCounter = channel.volumeFadeReload;
			if (channel.volumeFadeStep) {
				channel.volumeOffset += channel.volumeFadeStep;
				levelsDirty = true;
			}
		}
		if (--channel.panningFadeCounter == 0) {
			channel.panningFadeCounter = channel.panningFadeReload;
			if (channel.panningFadeStep) {
				channel.panning += channel.panningFadeStep;
				updatePanning(channelIndex);
				levelsDirty = true;
			}
		}
	}
	if (levelsDirty)
		updateChannelLevels(channelIndex);
}

bool PSound::isOpcodeDataValid(uint16 position, uint32 length) const {
	return isDataRangeValid(position, length);
}

byte PSound::readOpcodeByte(uint16 position, uint16 delta) const {
	return readDataByte((uint16)(position + delta));
}

uint16 PSound::readOpcodeWord(uint16 position, uint16 delta) const {
	return readDataUint16((uint16)(position + delta));
}

bool PSound::isScriptVariableValid(byte index) const {
	return index < kScriptVarCount;
}

bool PSound::transferOpcode(Channel &channel, uint16 position, bool take,
		bool isCall) {
	if (!isOpcodeDataValid(position, 5))
		return false;
	if (!take) {
		channel.position = position + 5;
		return true;
	}
	const uint16 target = readOpcodeWord(position, 3);
	if (!isDataRangeValid(target, 1))
		return false;
	if (isCall)
		channel.branchReturn = position + 5;
	channel.position = target;
	return true;
}

bool PSound::executeOpcode(uint channelIndex, byte opcode, bool &levelsDirty) {
	Channel &channel = _channels[channelIndex];
	const uint16 position = channel.position;

	switch (opcode) {
	case 0xff: {
		if (!isOpcodeDataValid(position, 2))
			return false;
		const uint16 count = (uint16)(int16)(int8)
				readOpcodeByte(position, 1);
		if (!channel.innerLoopCount) {
			if (!count) {
				channel.position = position + 2;
				channel.innerLoopStart = channel.position;
			} else {
				channel.innerLoopCount = count;
				channel.position = channel.innerLoopStart;
			}
		} else if (--channel.innerLoopCount) {
			channel.position = channel.innerLoopStart;
		} else {
			channel.position = position + 2;
			channel.innerLoopStart = channel.position;
		}
		break;
	}
	case 0xfe: {
		if (!isOpcodeDataValid(position, 2))
			return false;
		const uint16 count = (uint16)(int16)(int8)
				readOpcodeByte(position, 1);
		if (!channel.outerLoopCount) {
			if (!count) {
				channel.position = position + 2;
				channel.outerLoopStart = channel.position;
				channel.innerLoopStart = channel.position;
				channel.innerLoopCount = 0;
			} else {
				channel.outerLoopCount = count;
				channel.position = channel.outerLoopStart;
				channel.innerLoopStart = channel.outerLoopStart;
			}
		} else if (--channel.outerLoopCount) {
			channel.position = channel.outerLoopStart;
			channel.innerLoopStart = channel.outerLoopStart;
		} else {
			channel.position = position + 2;
			channel.outerLoopStart = channel.position;
			channel.innerLoopStart = channel.position;
		}
		break;
	}
	case 0xfd:
		channel.loopStart = channel.originalSequence;
		channel.position = channel.originalSequence;
		channel.innerLoopStart = channel.originalSequence;
		channel.outerLoopStart = channel.originalSequence;
		channel.pitchBend = 0;
		channel.volumeFadeStep = 0;
		channel.panningFadeStep = 0;
		channel.transpose = 0;
		channel.volumeOffset = 0;
		channel.volume = 0;
		channel.volumeFadeCounter = 0;
		channel.panningFadeCounter = 0;
		channel.innerLoopCount = 0;
		channel.outerLoopCount = 0;
		channel.noteOffset = 0;
		break;
	case 0xfc: {
		if (!isOpcodeDataValid(position, 3))
			return false;
		const uint16 target = readOpcodeWord(position, 1);
		if (!isDataRangeValid(target, 1))
			return false;
		channel.loopStart = channel.position = channel.innerLoopStart =
			channel.outerLoopStart = channel.originalSequence = target;
		break;
	}
	case 0xfb: {
		if (!isOpcodeDataValid(position, 3))
			return false;
		const uint16 target = readOpcodeWord(position, 1);
		if (!isDataRangeValid(target, 1))
			return false;
		channel.position = target;
		break;
	}
	case 0xfa: {
		if (!isOpcodeDataValid(position, 3))
			return false;
		const uint16 target = readOpcodeWord(position, 1);
		if (!isDataRangeValid(target, 1))
			return false;
		channel.branchReturn = position + 3;
		channel.position = target;
		break;
	}
	case 0xf9:
		if (channel.branchReturn) {
			channel.position = channel.branchReturn;
			channel.branchReturn = 0;
		} else {
			++channel.position;
		}
		break;
	case 0xf8:
		if (!isOpcodeDataValid(position, 2))
			return false;
		channel.patch = readOpcodeByte(position, 1);
		channel.position = position + 2;
		loadPatch(channelIndex, channel.patch);
		break;
	case 0xf7:
		if (!isOpcodeDataValid(position, 2))
			return false;
		channel.noteOffset = readOpcodeByte(position, 1);
		channel.durationOverride = 0;
		channel.position = position + 2;
		break;
	case 0xf6:
		if (!isOpcodeDataValid(position, 2))
			return false;
		channel.durationOverride = readOpcodeByte(position, 1);
		channel.noteOffset = 0;
		channel.position = position + 2;
		break;
	case 0xf5:
		if (!isOpcodeDataValid(position, 2))
			return false;
		channel.pitchBend = (int8)readOpcodeByte(position, 1);
		channel.position = position + 2;
		break;
	case 0xf4:
		if (!isOpcodeDataValid(position, 2))
			return false;
		channel.volume = (byte)((int8)readOpcodeByte(position, 1) >> 1);
		channel.position = position + 2;
		levelsDirty = true;
		break;
	case 0xf3:
		if (!isOpcodeDataValid(position, 3))
			return false;
		if (!channel.pendingStop) {
			channel.volumeFadeReload = readOpcodeByte(position, 1);
			channel.volumeFadeStep = (int8)readOpcodeByte(position, 2);
			channel.volumeFadeCounter = 1;
		}
		channel.position = position + 3;
		break;
	case 0xf2:
		if (!isOpcodeDataValid(position, 2))
			return false;
		channel.transpose = (int8)readOpcodeByte(position, 1);
		channel.position = position + 2;
		break;
	case 0xf1: {
		if (!isOpcodeDataValid(position, 2))
			return false;
		const int8 value =
			(int8)(((int8)readOpcodeByte(position, 1) >> 1) - 50);
		if (!channel.pendingStop || value < channel.volumeOffset) {
			channel.volumeOffset = value;
			levelsDirty = true;
		}
		channel.position = position + 2;
		break;
	}
	case 0xf0:
		if (!isOpcodeDataValid(position, 2))
			return false;
		channel.panning = readOpcodeByte(position, 1);
		channel.position = position + 2;
		updatePanning(channelIndex);
		levelsDirty = true;
		break;
	case 0xef:
		if (!isOpcodeDataValid(position, 3))
			return false;
		channel.panningFadeReload = readOpcodeByte(position, 1);
		channel.panningFadeStep = (int8)readOpcodeByte(position, 2);
		channel.panningFadeCounter = 1;
		channel.position = position + 3;
		break;
	case 0xee:
		if (!isOpcodeDataValid(position, 2))
			return false;
		channel.noteTranspose = (int8)readOpcodeByte(position, 1);
		channel.position = position + 2;
		break;
	case 0xed:
		if (!isOpcodeDataValid(position, 2))
			return false;
		channel.position = (uint16)(position +
			(int8)readOpcodeByte(position, 1) + 3);
		break;
	case 0xec: {
		if (!isOpcodeDataValid(position, 2))
			return false;
		const byte count = readOpcodeByte(position, 1);
		if (!count || !isOpcodeDataValid(position, (uint32)count + 3))
			return false;
		const uint16 base = position + 2;
		const byte selected = readDataByte(base +
										   (nextRandom() & 0x7fff) % count);
		const byte target = readDataByte(base + count);
		if (!isDataRangeValid(base + count + target + 1, 1))
			return false;
		writeDataByte(base + count + target + 1, selected);
		channel.position = position + count + 3;
		break;
	}
	case 0xeb: {
		if (!isOpcodeDataValid(position, 4))
			return false;
		const int low = (int8)readOpcodeByte(position, 1);
		const int high = (int8)readOpcodeByte(position, 2);
		const int range = high - low + 1;
		if (range <= 0)
			return false;
		const byte target = readOpcodeByte(position, 3);
		if (!isDataRangeValid(position + 4 + target, 1))
			return false;
		writeDataByte(position + 4 + target,
					  (byte)(low + (nextRandom() & 0x7fff) % range));
		channel.position = position + 4;
		break;
	}
	case 0xea: {
		if (!isOpcodeDataValid(position, 3))
			return false;
		const byte variable = readOpcodeByte(position, 1);
		const byte count = readOpcodeByte(position, 2);
		if (!isScriptVariableValid(variable) ||
				!isOpcodeDataValid(position, (uint32)count + 4))
			return false;
		const uint16 base = position + 3;
		const byte target = readDataByte(base + count);
		if (!isDataRangeValid(base + _scriptVars[variable], 1) ||
			!isDataRangeValid(base + target + 1, 1))
			return false;
		writeDataByte(base + target + 1,
					  readDataByte(base + _scriptVars[variable]));
		channel.position = position + count + 4;
		break;
	}
	case 0xe9:
	case 0xe8:
	case 0xe7:
	case 0xe4:
	case 0xe3:
	case 0xe2:
	case 0xe1:
	case 0xe0:
	case 0xdf:
	case 0xde:
	case 0xdd:
	case 0xdc:
	case 0xdb:
	case 0xda:
	case 0xd9:
	case 0xd8:
	case 0xd7:
	case 0xd6:
	case 0xd5: {
		if (!isOpcodeDataValid(position, 3))
			return false;
		const byte first = readOpcodeByte(position, 1);
		const byte second = readOpcodeByte(position, 2);
		if (!isScriptVariableValid(first))
			return false;
		if (opcode == 0xe9) {
			_scriptVars[first] = second;
		} else if (opcode == 0xe8) {
			if (!isScriptVariableValid(second))
				return false;
			_scriptVars[first] = _scriptVars[second];
		} else if (opcode == 0xe7) {
			if (!isDataRangeValid(position + 3 + second, 1))
				return false;
			writeDataByte(position + 3 + second, _scriptVars[first]);
		} else {
			const bool usesVariable = (opcode & 1) != 0;
			if (usesVariable && !isScriptVariableValid(second))
				return false;
			const byte operand = usesVariable ? _scriptVars[second] : second;
			byte &destination = _scriptVars[first];
			switch (opcode) {
			case 0xe4:
			case 0xe3:
				destination += operand;
				break;
			case 0xe2:
			case 0xe1:
				destination -= operand;
				break;
			case 0xe0:
			case 0xdf:
				destination *= operand;
				break;
			case 0xde:
			case 0xdd:
				if (!operand)
					return false;
				destination /= operand;
				break;
			case 0xdc:
			case 0xdb:
				if (!operand)
					return false;
				destination %= operand;
				break;
			case 0xda:
			case 0xd9:
				destination &= operand;
				break;
			case 0xd8:
			case 0xd7:
				destination |= operand;
				break;
			case 0xd6:
			case 0xd5:
				destination ^= operand;
				break;
			default:
				break;
			}
		}
		channel.position = position + 3;
		break;
	}
	case 0xe6:
	case 0xe5: {
		if (!isOpcodeDataValid(position, 2))
			return false;
		const byte variable = readOpcodeByte(position, 1);
		if (!isScriptVariableValid(variable))
			return false;
		_scriptVars[variable] += opcode == 0xe6 ? 1 : (byte)-1;
		channel.position = position + 2;
		break;
	}
	case 0xd4:
	case 0xd3:
	case 0xd2:
	case 0xd1:
	case 0xd0:
	case 0xcf:
	case 0xce:
	case 0xcd:
	case 0xcc:
	case 0xcb:
	case 0xca:
	case 0xc9:
	case 0xc8:
	case 0xc7:
	case 0xc6:
	case 0xc5: {
		if (!isOpcodeDataValid(position, 5))
			return false;
		const byte first = readOpcodeByte(position, 1);
		const byte second = readOpcodeByte(position, 2);
		if (!isScriptVariableValid(first))
			return false;
		const bool variablePair =
			(opcode >= 0xcd && opcode <= 0xd0) || opcode <= 0xc8;
		if (variablePair && !isScriptVariableValid(second))
			return false;
		const byte lhs = _scriptVars[first];
		const byte rhs = variablePair ? _scriptVars[second] : second;
		bool take = false;
		switch (opcode) {
		case 0xd4:
		case 0xcc:
		case 0xd0:
		case 0xc8:
			take = lhs == rhs;
			break;
		case 0xd3:
		case 0xcb:
		case 0xcf:
		case 0xc7:
			take = lhs != rhs;
			break;
		case 0xd2:
		case 0xca:
		case 0xce:
		case 0xc6:
			take = lhs < rhs;
			break;
		case 0xd1:
		case 0xc9:
		case 0xcd:
		case 0xc5:
			take = lhs > rhs;
			break;
		default:
			break;
		}
		if (!transferOpcode(channel, position, take, opcode <= 0xcc))
			return false;
		break;
	}
	case 0xc4:
		if (!isOpcodeDataValid(position, 3) ||
				!callFunction(readOpcodeWord(position, 1), channel))
			return false;
		channel.position = position + 3;
		break;
	case 0xc3:
		if (!isOpcodeDataValid(position, 2))
			return false;
		channel.position = position + 2;
		break;
	case 0xc2:
		if (!isOpcodeDataValid(position, 4))
			return false;
		channel.position = position + 4;
		break;
	case 0xc1:
		if (!isOpcodeDataValid(position, 2))
			return false;
		_tempoScale = readOpcodeByte(position, 1);
		channel.position = position + 2;
		break;
	case 0xc0:
		if (!isOpcodeDataValid(position, 2))
			return false;
		_tempoReload = readOpcodeByte(position, 1);
		channel.position = position + 2;
		if (!_frameNumber2)
			_tempoCurrent = _tempoReload;
		break;
	case 0xbf:
		if (!isOpcodeDataValid(position, 3))
			return false;
		_tempoTarget = readOpcodeWord(position, 1);
		channel.position = position + 3;
		if (!_frameNumber2)
			_tempoBase = _tempoTarget;
		_tickEnabled = 1;
		_tickCounter = 1;
		break;
	case 0xbe:
		if (!isOpcodeDataValid(position, 2))
			return false;
		_tempoShift = (int8)readOpcodeByte(position, 1);
		channel.position = position + 2;
		break;
	default:
		return false;
	}
	return true;
}

void PSound::finishChannel(uint channelIndex) {
	keyOff(channelIndex);
	_channels[channelIndex].activeCount = 0;
	_channels[channelIndex].keyOnDelay = 0;
}

void PSound::checkPendingStops() {
	for (uint i = 0; i < kChannelCount; ++i) {
		Channel &channel = _channels[i];
		if (!channel.activeCount || !channel.pendingStop)
			continue;
		if ((byte)channel.volumeOffset == 0xd8) {
			channel.position = _nullSequenceOffset;
			channel.pendingStop = 0;
		} else {
			channel.volumeFadeStep = -1;
			channel.volumeFadeReload = 4;
			if (!channel.volumeFadeCounter)
				channel.volumeFadeCounter = 1;
		}
	}
}

void PSound::programOperator(byte banks, uint channelIndex,
							 uint operatorIndex, const byte *operatorData) {
	const byte op = getOperatorOffset(channelIndex, operatorIndex);
	const byte characteristics = (operatorData[9] & 0x0f) |
								 ((operatorData[5] & 1) << 4) | ((operatorData[4] & 1) << 5) |
								 ((operatorData[12] & 1) << 6) | ((operatorData[11] & 1) << 7);
	const byte totalLevel = ((operatorData[7] & 3) << 6) |
							clampLevel(0x3f - (operatorData[6] & 0x3f));
	writeRegister(banks, 0x40 + op, 0x3f);
	writeRegister(banks, 0x20 + op, characteristics);
	writeRegister(banks, 0x60 + op,
				  (operatorData[0] << 4) | (operatorData[1] & 0x0f));
	writeRegister(banks, 0x80 + op,
				  (operatorData[2] << 4) | (operatorData[3] & 0x0f));
	writeRegister(banks, 0xe0 + op, operatorData[8] & 3);
	writeRegister(banks, 0x40 + op, totalLevel);
}

void PSound::loadPatch(uint channelIndex, byte patchIndex) {
	Channel &channel = _channels[channelIndex];
	const byte *patch = getPatch(patchIndex);
	const byte banks = getBankMask(channelIndex);
	const byte oplChannel = getOplChannel(channelIndex);
	const uint operatorCount = channelIndex < kMusicChannelCount ? 4 : 2;

	keyOff(channelIndex);
	channel.mode = patch[0x0d];
	for (uint i = 0; i < 4; ++i)
		channel.operatorTotalLevel[i] = patch[i * 14 + 6];
	for (uint i = 0; i < operatorCount; ++i)
		programOperator(banks, channelIndex, i, patch + i * 14);

	if (channelIndex < kMusicChannelCount) {
		const byte stereo = panningBits(channel.panning);
		const byte feedback = (patch[0x0a] & 7) << 1;
		writeRegister(banks, 0xc0 + oplChannel,
					  stereo | feedback | (channel.mode >> 1));
		writeRegister(banks, 0xc3 + oplChannel,
					  stereo | feedback | (channel.mode & 1));
	} else {
		const byte value = ((patch[0x0a] & 7) << 1) |
						   ((channel.mode & 1) ^ 1);
		writeRegister(kFirstBank, 0xc0 + oplChannel, value | 0x20);
		writeRegister(kSecondBank, 0xc0 + oplChannel, value | 0x10);
	}

	channel.noiseTicks = patch[0x38];
	channel.noiseMask = READ_LE_UINT16(patch + 0x3a);
	channel.noiseBase = READ_LE_UINT16(patch + 0x3c);
	channel.noiseStep = (int16)READ_LE_UINT16(patch + 0x3e);
	if (channel.noiseTicks)
		resultCheck();
	updatePanning(channelIndex);
	updateChannelLevels(channelIndex);
}

void PSound::updatePanning(uint channelIndex) {
	const byte oplChannel = getOplChannel(channelIndex);
	if (channelIndex < kMusicChannelCount) {
		const byte banks = getBankMask(channelIndex);
		const byte stereo = panningBits(_channels[channelIndex].panning);
		const byte first = 0xc0 + oplChannel;
		const byte second = 0xc3 + oplChannel;
		writeRegister(banks, first,
					  (getCachedRegister(banks, first) & 0x0f) | stereo);
		writeRegister(banks, second,
					  (getCachedRegister(banks, second) & 0x0f) | stereo);
	} else {
		const byte reg = 0xc0 + oplChannel;
		writeRegister(kFirstBank, reg,
					  (getCachedRegister(kFirstBank, reg) & 0x0f) | 0x20);
		writeRegister(kSecondBank, reg,
					  (getCachedRegister(kSecondBank, reg) & 0x0f) | 0x10);
	}
}

void PSound::updateChannelLevels(uint channelIndex) {
	Channel &channel = _channels[channelIndex];
	const byte *patch = getPatch(channel.patch);
	int base = 0x7e - channel.volume - channel.volumeOffset -
			   channel.patchAttenuation;
	base += (255 - _masterVolume) * 63 / 255;

	if (channelIndex >= kMusicChannelCount) {
		const uint operators[2] = {1, 0};
		const bool enabled[2] = {true, channel.mode == 0};
		for (uint i = 0; i < 2; ++i) {
			if (!enabled[i])
				continue;
			const uint opIndex = operators[i];
			const byte op = getOperatorOffset(channelIndex, opIndex);
			const byte scaling = (patch[opIndex * 14 + 7] & 3) << 6;
			const int left = clampLevel(base -
										channel.operatorTotalLevel[opIndex] +
										getPanningAttenuation(channel.panning));
			const int right = clampLevel(base -
										 channel.operatorTotalLevel[opIndex] +
										 getPanningAttenuation(0x7f - channel.panning));
			writeRegister(kFirstBank, 0x40 + op, scaling | left);
			writeRegister(kSecondBank, 0x40 + op, scaling | right);
		}
		return;
	}

	if (channel.panning > 0x2a && channel.panning < 0x55)
		base += 6;
	const byte banks = getBankMask(channelIndex);
	const uint operators[4] = {3, 1, 0, 2};
	const bool enabled[4] = {
		true, channel.mode == 1, (channel.mode & 2) != 0, channel.mode == 3};
	for (uint i = 0; i < 4; ++i) {
		if (!enabled[i])
			continue;
		const uint opIndex = operators[i];
		const byte op = getOperatorOffset(channelIndex, opIndex);
		const byte scaling = (patch[opIndex * 14 + 7] & 3) << 6;
		const int level = clampLevel(base - channel.operatorTotalLevel[opIndex]);
		writeRegister(banks, 0x40 + op, scaling | level);
	}
}

void PSound::updateChannelFrequency(uint channelIndex, bool keyOn) {
	Channel &channel = _channels[channelIndex];
	updateChannelLevels(channelIndex);
	const byte note = (byte)(channel.note + channel.noteTranspose);
	const int frequency = getFrequencyNumber(note % 12) + channel.transpose;
	const byte banks = getBankMask(channelIndex);
	const byte oplChannel = getOplChannel(channelIndex);
	writeRegister(banks, 0xa0 + oplChannel, frequency & 0xff);
	byte high = (((note / 12) & 7) << 2) | ((frequency >> 8) & 3);
	if (keyOn)
		high |= 0x20;
	writeRegister(banks, 0xb0 + oplChannel, high);
}

void PSound::updatePitchBend(uint channelIndex) {
	const Channel &channel = _channels[channelIndex];
	const byte banks = getBankMask(channelIndex);
	const byte oplChannel = getOplChannel(channelIndex);
	const byte lowReg = 0xa0 + oplChannel;
	const byte highReg = 0xb0 + oplChannel;
	int frequency = ((getCachedRegister(banks, highReg) & 0x1f) << 8) |
					getCachedRegister(banks, lowReg);
	frequency += channel.pitchBend;
	writeRegister(banks, lowReg, frequency & 0xff);
	writeRegister(banks, highReg,
				  (getCachedRegister(banks, highReg) & 0x20) |
					  ((frequency >> 8) & 0x1f));
}

void PSound::keyOff(uint channelIndex) {
	const byte banks = getBankMask(channelIndex);
	const byte reg = 0xb0 + getOplChannel(channelIndex);
	writeRegister(banks, reg, getCachedRegister(banks, reg) & 0xdf);
}

void PSound::setNoiseFrequency(uint channelIndex, int frequency) {
	const byte banks = getBankMask(channelIndex);
	const byte channel = getOplChannel(channelIndex);
	writeRegister(banks, 0xa0 + channel, frequency & 0xff);
	writeRegister(banks, 0xb0 + channel,
				  ((frequency >> 8) & 0x1f) | 0x20);
}

void PSound::resultCheck() {
	if (_resultFlag != 1) {
		_resultFlag = 1;
		_pollResult = 1;
	}
}

int PSound::stop() {
	Common::StackLock lock(_driverMutex);
	command0();
	const int result = _pollResult;
	_pollResult = 0;
	return result;
}

int PSound::poll() {
	Common::StackLock lock(_driverMutex);
	return serviceUpdate();
}

void PSound::noise() {
	Common::StackLock lock(_driverMutex);
	serviceNoise();
}

void PSound::setVolume(int volume) {
	Common::StackLock lock(_driverMutex);
	_masterVolume = CLIP(volume, 0, 255);
	for (uint i = 0; i < kChannelCount; ++i) {
		if (_channels[i].activeCount)
			updateChannelLevels(i);
	}
}

} // namespace Sound
} // namespace Phantom
} // namespace MADS
