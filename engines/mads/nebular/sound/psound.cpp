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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "audio/fmopl.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/func.h"
#include "common/md5.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "mads/nebular/sound/psound.h"

namespace MADS {
namespace RexNebular {
namespace Sound {

namespace {

/*
 * Rex PSOUND is rendered as PAS16/OPL3. The two native logical destinations
 * map directly to the low and high OPL3 register banks. A write to both
 * destinations is still emitted twice because OPL3 has no broadcast register.
 */

/*
 * The executable, not the PSOUND descriptor's nominal value 100, owns the
 * driver cadence. Its PIT channel-0 handler retains every second raw interrupt,
 * calls export 4 on each retained service while enabled, and calls export 3
 * every fifth service. The OPL callback is only a time source close to that raw
 * interrupt grid; NativeSoundTimer reconstructs the exact long-term rates with
 * rational arithmetic.
 */
constexpr int kHostCallbackRateHz =
		NativeSoundTimer::kPitClockHz / NativeSoundTimer::kHostTimerDivisor;

struct PSoundFileSignature {
	int section;
	uint32 fileSize;
	const char *md5;
};

/*
 * These signatures cover the first 8192 bytes of each verified PSOUND module,
 * matching the validation used by the other native sound overlays. Keep the
 * retail and demo tables separate because they reuse filenames while containing
 * different binaries and, for demo PSOUND.009, a different table layout.
 */
static const PSoundFileSignature kRetailSignatures[] = {
	{ 1, 0x3786, "49045141642eddc7aa8652554c87d7ca" },
	{ 2, 0x61a0, "a48086be285c23abd64a73cffb6de9a1" },
	{ 3, 0x6942, "0c0907533a6dfe5e49973eba845e006b" },
	{ 4, 0x404f, "988b96242bdd29cb8d52d15a97a5a05c" },
	{ 5, 0x38e2, "64cfeab1fce320f8941ee5e2808656c4" },
	{ 6, 0x3a18, "4675c3f172fa7c8c73e1ff3b903f9af6" },
	{ 7, 0x4339, "3e9c54dccc93c01524ab3ec38b17ae0c" },
	{ 8, 0x325a, "4721aee44bd56857a8fb01d1245f5e23" },
	{ 9, 0x936c, "5bfac799abc1149cdfcf73a4a1583a39" }
};

static const PSoundFileSignature kDemoSignatures[] = {
	{ 1, 0x449a, "92890dee6887466ee0bbb835934fcc4e" },
	{ 9, 0x6676, "fa366b53123001e29ba9be9537c44ae2" }
};

static void validateInitializedRange(uint32 initializedSize, uint32 offset,
		uint32 length, const char *description) {
	if (offset > initializedSize || length > initializedSize - offset)
		error("PSOUND %s outside initialized data: offset 0x%04x, length %u, size 0x%04x",
				description, (uint)offset, (uint)length, (uint)initializedSize);
}

} // namespace

static int clampLevel(int value) {
	return CLIP(value, 0, 63);
}

static byte panningBits(byte panning) {
	if (panning < 0x2b)
		return 0x10;
	if (panning < 0x55)
		return 0x30;
	return 0x20;
}

void PSound::validate(bool isDemo) {
	const PSoundFileSignature *signatures = isDemo ?
			kDemoSignatures : kRetailSignatures;
	const uint count = isDemo ? ARRAYSIZE(kDemoSignatures) :
			ARRAYSIZE(kRetailSignatures);

	for (uint index = 0; index < count; ++index) {
		const PSoundFileSignature &signature = signatures[index];
		const Common::Path filename(Common::String::format("PSOUND.00%d",
				signature.section));
		Common::File file;
		if (!file.open(filename))
			error("Could not process - %s", filename.toString().c_str());
		if ((uint32)file.size() != signature.fileSize)
			error("Invalid sound file size - %s", filename.toString().c_str());

		file.seek(0);
		const Common::String md5 = Common::computeStreamMD5AsString(file,
				8192);
		file.close();
		if (md5 != signature.md5)
			error("Invalid sound file - %s", filename.toString().c_str());
	}
}

void PSound::Channel::reset() {
	activeCount = 0;
	pitchBend = 0;
	volumeFadeStep = 0;
	panningFadeStep = 0;
	note = 0;
	patch = 0;
	volume = 0;
	noteOffset = 0;
	keyOnDelay = 0;
	volumeFadeCounter = 0;
	volumeFadeReload = 0xff;
	panningFadeCounter = 0;
	panningFadeReload = 0;
	panning = 0x40;
	volumeOffset = 0;
	mode = 0;
	memset(operatorTotalLevel, 0, sizeof(operatorTotalLevel));
	sequenceStart = 0;
	position = 0;
	innerLoopStart = 0;
	outerLoopStart = 0;
	innerLoopCount = 0;
	outerLoopCount = 0;
	originalSequence = 0;
	transpose = 0;
	noteTranspose = 0;
	pendingStop = 0;
	patchAttenuation = 0;
}

void PSound::Channel::load(uint16 sequenceOffset) {
	sequenceStart = sequenceOffset;
	position = sequenceOffset;
	innerLoopStart = sequenceOffset;
	outerLoopStart = sequenceOffset;
	originalSequence = sequenceOffset;
	volumeFadeReload = 0xff;
	pitchBend = 0;
	volumeFadeStep = 0;
	panningFadeStep = 0;
	panning = 0x40;
	transpose = 0;
	volumeOffset = 0;
	volume = 0;
	volumeFadeCounter = 0;
	panningFadeCounter = 0;
	pendingStop = 0;
	noteTranspose = 0;
	innerLoopCount = 0;
	outerLoopCount = 0;
	noteOffset = 0;
	activeCount = 1;
}

void PSound::ChannelData::reset() {
	noiseMode = 0;
	frequencyMask = 0;
	frequencyBase = 0;
	frequencyStep = 0;
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
	_musicChannelCount = driverData.musicChannelCount;
	_commandParam = 0;
	_updatesEnabled = false;
	_noiseServiceEnabled = false;
	_noiseState = false;

	if (_soundData.size() != (uint32)driverData.initializedDataSize)
		error("PSOUND initialized data has unexpected size %u (expected %d)",
				(uint)_soundData.size(), driverData.initializedDataSize);
	if (driverData.totalDataSize < driverData.initializedDataSize ||
			driverData.totalDataSize > 0xffff)
		error("PSOUND has invalid mutable data size %d", driverData.totalDataSize);

	const uint32 initializedSize = _soundData.size();
	_soundData.resize(driverData.totalDataSize);
	if (_soundData.size() > initializedSize)
		memset(&_soundData[initializedSize], 0, _soundData.size() - initializedSize);

	validateDataLayout(driverData);

	memset(_noiseTicks, 0, sizeof(_noiseTicks));
	memset(_savedNoiseTicks, 0, sizeof(_savedNoiseTicks));
	memset(_noiseChannel, 0, sizeof(_noiseChannel));
	memset(_noiseMask, 0, sizeof(_noiseMask));
	memset(_noiseBase, 0, sizeof(_noiseBase));
	memset(_noiseStep, 0, sizeof(_noiseStep));
	for (uint i = 0; i < kChannelCount; ++i) {
		_channels[i].reset();
		_channelData[i].reset();
	}

	_randomSeed = readDataUint16(0x58);
	memset(_registerCache, 0, sizeof(_registerCache));

	_opl = OPL::Config::create(OPL::Config::kOpl3);
	if (!_opl || !_opl->init()) {
		delete _opl;
		_opl = nullptr;
		return;
	}
	resetDriver();
	_updatesEnabled = true;
	/*
	 * The original host calls PSOUND export 4 at the retained service rate
	 * (about 304.383 Hz) and export 3 every fifth service (about 60.877 Hz).
	 * Starting at 608 Hz mirrors the source grid used by the recovered host;
	 * NativeSoundTimer corrects the fractional difference from the physical
	 * PIT rate.
	 *
	 * Emulated OPL backends schedule these callbacks from generated audio
	 * samples and are the timing reference. Audio::RealChip deliberately caps
	 * operating-system timer requests at 100 Hz, then invokes a requested
	 * high-rate callback several times per timer event. Counts, ordering and
	 * long-term phase therefore remain correct, but export-4 modulation writes
	 * reach physical OPL backends in roughly 10 ms bursts instead of being
	 * spaced about 3.3 ms apart. That can subtly change noise-like effects.
	 * Lowering this request to 100 Hz would also degrade emulated playback, so
	 * fixing physical-chip spacing belongs in the shared RealChip scheduler.
	 */
	_opl->start(new Common::Functor0Mem<void, PSound>(this, &PSound::onTimer),
			kHostCallbackRateHz);
}

PSound::~PSound() {
	if (_opl) {
		_opl->stop();
		delete _opl;
	}
}

bool PSound::isDataRangeValid(uint32 offset, uint32 length) const {
	return offset <= _soundData.size() && length <= _soundData.size() - offset;
}

void PSound::requireDataRange(uint32 offset, uint32 length,
		const char *operation) const {
	if (!isDataRangeValid(offset, length))
		error("PSOUND %s outside data image: offset 0x%04x, length %u, size 0x%04x",
				operation, (uint)offset, (uint)length, (uint)_soundData.size());
}

const byte *PSound::getDataPointer(uint32 offset, uint32 length,
		const char *operation) const {
	requireDataRange(offset, length, operation);
	return &_soundData[offset];
}

byte *PSound::getDataPointer(uint32 offset, uint32 length,
		const char *operation) {
	requireDataRange(offset, length, operation);
	return &_soundData[offset];
}

void PSound::validateDataLayout(const PSoundDriverData &driverData) const {
	const uint32 initializedSize = driverData.initializedDataSize;

	if (!driverData.patchCount || driverData.musicChannelCount > kChannelCount)
		error("PSOUND has invalid patch/channel counts");
	validateInitializedRange(initializedSize, 0x58, 2, "random seed");
	validateInitializedRange(initializedSize, driverData.nullSequenceOffset, 1,
			"null sequence");
	validateInitializedRange(initializedSize, driverData.patchTableOffset,
			(uint32)driverData.patchCount * kPatchSize, "patch table");
	validateInitializedRange(initializedSize, driverData.tables.panning, 0x80,
			"panning table");
	validateInitializedRange(initializedSize, driverData.tables.frequency, 12 * 2,
			"frequency table");
	validateInitializedRange(initializedSize, driverData.tables.bank, kChannelCount,
			"bank table");
	validateInitializedRange(initializedSize, driverData.tables.channel, kChannelCount,
			"channel table");
	validateInitializedRange(initializedSize, driverData.tables.operators,
			kChannelCount * 4, "operator table");

	for (uint channel = 0; channel < kChannelCount; ++channel) {
		const byte bank = readDataByte(driverData.tables.bank + channel);
		const byte oplChannel = readDataByte(driverData.tables.channel + channel);
		if (!bank || (bank & ~kBothBanks) || oplChannel > 8)
			error("PSOUND has invalid OPL routing for channel %u", channel);
		const uint operatorCount = channel < 6 ? 4 : 2;
		for (uint op = 0; op < operatorCount; ++op) {
			if (readDataByte(driverData.tables.operators + channel * 4 + op) > 0x15)
				error("PSOUND has invalid operator routing for channel %u", channel);
		}
	}
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
	assert(channel < kChannelCount);
	return readDataByte(_tableLayout.bank + channel);
}

byte PSound::getOplChannel(uint channel) const {
	assert(channel < kChannelCount);
	return readDataByte(_tableLayout.channel + channel);
}

byte PSound::getOperatorOffset(uint channel, uint operatorIndex) const {
	assert(channel < kChannelCount && operatorIndex < 4);
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

void PSound::resetDriver() {
	const bool wasEnabled = _updatesEnabled;
	_updatesEnabled = false;
	for (uint i = 0; i < kChannelCount; ++i) {
		_channels[i].activeCount = 0;
		_channels[i].pitchBend = 0;
		_channels[i].volumeFadeStep = 0;
		_channels[i].panningFadeStep = 0;
		_channels[i].pendingStop = 0;
		_channelData[i].reset();
	}
	memset(_noiseTicks, 0, sizeof(_noiseTicks));
	memset(_savedNoiseTicks, 0, sizeof(_savedNoiseTicks));
	memset(_noiseMask, 0, sizeof(_noiseMask));
	memset(_noiseBase, 0, sizeof(_noiseBase));
	memset(_noiseStep, 0, sizeof(_noiseStep));
	_resultFlag = 0;
	_pollResult = 0;

	/*
	 * Preserve the PAS16 initialization literally: secondary-bank registers
	 * 0x05 and 0x04 are OPL3 registers 0x105 (New mode) and 0x104 (enable all
	 * six four-operator pairs). These writes must precede ordinary bank setup.
	 */
	writeRegister(kSecondBank, 0x05, 0x01);
	writeRegister(kSecondBank, 0x04, 0x3f);

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
	assert(firstChannel <= endChannel && endChannel <= kChannelCount);
	for (uint i = firstChannel; i < endChannel; ++i) {
		if (_channels[i].activeCount) {
			_channels[i].pendingStop = 0xff;
			_channels[i].originalSequence = 0xffff;
		}
	}
}

void PSound::setCurrentSequence(uint firstChannel, uint endChannel, uint16 sequenceOffset) {
	assert(firstChannel <= endChannel && endChannel <= kChannelCount);
	requireDataRange(sequenceOffset, 1, "sequence position");
	for (uint i = firstChannel; i < endChannel; ++i)
		_channels[i].position = sequenceOffset;
}

void PSound::loadChannel(uint channel, uint16 sequenceOffset) {
	assert(channel < kChannelCount);
	requireDataRange(sequenceOffset, 1, "sequence start");
	_channels[channel].load(sequenceOffset);
}

void PSound::playSound(uint16 sequenceOffset) {
	for (uint i = _musicChannelCount; i < kChannelCount; ++i) {
		if (!_channels[i].activeCount) {
			loadChannel(i, sequenceOffset);
			return;
		}
	}
	for (uint i = _musicChannelCount; i < kChannelCount; ++i) {
		if (_channels[i].pendingStop == 0xff) {
			loadChannel(i, sequenceOffset);
			return;
		}
	}
}

void PSound::playSoundAny(uint16 sequenceOffset) {
	for (uint i = 0; i < _musicChannelCount; ++i) {
		if (!_channels[i].activeCount) {
			loadChannel(i, sequenceOffset);
			return;
		}
	}
	for (uint i = 0; i < _musicChannelCount; ++i) {
		if (_channels[i].pendingStop == 0xff) {
			loadChannel(i, sequenceOffset);
			return;
		}
	}
}

bool PSound::isSoundActive(uint16 sequenceOffset) const {
	for (uint i = 0; i < kChannelCount; ++i) {
		if (_channels[i].activeCount && _channels[i].originalSequence == sequenceOffset)
			return true;
	}
	return false;
}

uint16 PSound::nextRandom() {
	const uint16 value = 0x9248 + _randomSeed;
	_randomSeed = (value >> 3) | (value << 13);
	return _randomSeed;
}

byte PSound::scaledCommandParameter(int param) const {
	const byte value = param;
	return value > 0x1e ? value - 0x1e : 0;
}

int PSound::command0() {
	resetDriver();
	return 0;
}

int PSound::command1() {
	requestStop(0, kChannelCount);
	return 0;
}

int PSound::command2() {
	setCurrentSequence(0, _musicChannelCount, _nullSequenceOffset);
	return 0;
}

int PSound::command3() {
	requestStop(0, _musicChannelCount);
	return 0;
}

int PSound::command4() {
	setCurrentSequence(_musicChannelCount, kChannelCount, _nullSequenceOffset);
	return 0;
}

int PSound::command5() {
	requestStop(_musicChannelCount, kChannelCount);
	return 0;
}

int PSound::command6() {
	_savedNoiseTicks[0] = _noiseTicks[0];
	_savedNoiseTicks[1] = _noiseTicks[1];
	_noiseTicks[0] = _noiseTicks[1] = 0;
	for (uint i = 0; i < kChannelCount; ++i)
		keyOff(i);
	_updatesEnabled = false;
	return 0;
}

int PSound::command7() {
	_noiseTicks[0] = _savedNoiseTicks[0];
	_noiseTicks[1] = _savedNoiseTicks[1];
	_updatesEnabled = true;
	for (uint i = 0; i < kChannelCount; ++i) {
		if (_channels[i].activeCount) {
			updateChannelLevels(i);
			updateChannelFrequency(i, true);
		}
	}
	if (_noiseTicks[0] != _noiseTicks[1])
		resultCheck();
	return _savedNoiseTicks[1];
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
		/*
		 * The executable calls export 4 before export 3. A nonzero result
		 * from export 3 consequently changes the export-4 gate beginning on
		 * the following retained service tick.
		 */
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
	const uint16 random = nextRandom();
	if (_noiseTicks[0])
		setNoiseFrequency(_noiseChannel[0],
				((~random) & _noiseMask[0]) + _noiseBase[0]);
	if (_noiseTicks[1])
		setNoiseFrequency(_noiseChannel[1],
				(random & _noiseMask[1]) + _noiseBase[1]);
}

void PSound::update() {
	/*
	 * Native export 3 advances the shared random state before testing the
	 * driver's disabled sentinel. Export 4 uses the same seed, so moving this
	 * call after the guard changes later noise modulation.
	 */
	nextRandom();
	if (!_updatesEnabled)
		return;

	tickCallback();
	++_frameCounter;
	for (uint i = 0; i < kChannelCount; ++i)
		updateChannel(i);
	checkPendingStops();
	updateNoise();

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
			if (value <= 0xf0) {
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
					channel.keyOnDelay = (byte)(channel.activeCount - channel.noteOffset);
					updateChannelFrequency(channelIndex, true);
				}
				break;
			}

			levelsDirty = false;
			if (!executeOpcode(channelIndex, value, levelsDirty)) {
				finishChannel(channelIndex);
				break;
			}
		}

		if (budget < 0 && channel.activeCount == 0)
			finishChannel(channelIndex);
	}

	if (channel.pitchBend)
		updatePitchBend(channelIndex);

	bool levelsDirty = false;
	if (channel.volumeFadeCounter && --channel.volumeFadeCounter == 0) {
		channel.volumeFadeCounter = channel.volumeFadeReload;
		if (channel.volumeFadeStep) {
			channel.volumeOffset += channel.volumeFadeStep;
			levelsDirty = true;
		}
	}

	if (channel.panningFadeCounter && --channel.panningFadeCounter == 0) {
		channel.panningFadeCounter = channel.panningFadeReload;
		if (channel.panningFadeStep) {
			channel.panning += channel.panningFadeStep;
			updatePanning(channelIndex);
			levelsDirty = true;
		}
	}

	if (levelsDirty)
		updateChannelLevels(channelIndex);
}

bool PSound::executeOpcode(uint channelIndex, byte opcode, bool &levelsDirty) {
	Channel &channel = _channels[channelIndex];
	const uint32 position = channel.position;
	if (opcode < 0xf1)
		return false;

	switch (opcode) {
	case 0xff: {
		if (!isDataRangeValid(position, 2))
			return false;
		const byte count = readDataByte(position + 1);
		if (!channel.innerLoopCount) {
			if (!count) {
				channel.position = (uint16)(position + 2);
				channel.innerLoopStart = channel.position;
				channel.innerLoopCount = 0;
			} else {
				if (!isDataRangeValid(channel.innerLoopStart, 1))
					return false;
				channel.innerLoopCount = count;
				channel.position = channel.innerLoopStart;
			}
		} else if (--channel.innerLoopCount) {
			if (!isDataRangeValid(channel.innerLoopStart, 1))
				return false;
			channel.position = channel.innerLoopStart;
		} else {
			channel.position = (uint16)(position + 2);
			channel.innerLoopStart = channel.position;
		}
		break;
	}

	case 0xfe: {
		if (!isDataRangeValid(position, 2))
			return false;
		const byte count = readDataByte(position + 1);
		if (!channel.outerLoopCount) {
			if (!count) {
				channel.position = (uint16)(position + 2);
				channel.outerLoopStart = channel.position;
				channel.innerLoopStart = channel.position;
				channel.innerLoopCount = 0;
				channel.outerLoopCount = 0;
			} else {
				if (!isDataRangeValid(channel.outerLoopStart, 1))
					return false;
				channel.outerLoopCount = count;
				channel.position = channel.outerLoopStart;
				channel.innerLoopStart = channel.outerLoopStart;
			}
		} else if (--channel.outerLoopCount) {
			if (!isDataRangeValid(channel.outerLoopStart, 1))
				return false;
			channel.position = channel.outerLoopStart;
			channel.innerLoopStart = channel.outerLoopStart;
		} else {
			channel.position = (uint16)(position + 2);
			channel.outerLoopStart = channel.position;
			channel.innerLoopStart = channel.position;
		}
		break;
	}

	case 0xfd:
		if (!isDataRangeValid(channel.originalSequence, 1))
			return false;
		channel.sequenceStart = channel.originalSequence;
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

	case 0xfc:
		if (!isDataRangeValid(position, 2))
			return false;
		channel.patch = readDataByte(position + 1);
		channel.position = (uint16)(position + 2);
		loadPatch(channelIndex, channel.patch);
		break;

	case 0xfb:
		if (!isDataRangeValid(position, 2))
			return false;
		channel.noteOffset = readDataByte(position + 1);
		channel.position = (uint16)(position + 2);
		break;

	case 0xfa:
		if (!isDataRangeValid(position, 2))
			return false;
		channel.pitchBend = (int8)readDataByte(position + 1);
		channel.position = (uint16)(position + 2);
		break;

	case 0xf9:
		if (!isDataRangeValid(position, 2))
			return false;
		channel.volume = (byte)((int8)readDataByte(position + 1) >> 1);
		channel.position = (uint16)(position + 2);
		levelsDirty = true;
		break;

	case 0xf8:
		if (!isDataRangeValid(position, 3))
			return false;
		if (!channel.pendingStop) {
			channel.volumeFadeReload = readDataByte(position + 1);
			channel.volumeFadeStep = (int8)readDataByte(position + 2);
			channel.volumeFadeCounter = 1;
		}
		channel.position = (uint16)(position + 3);
		break;

	case 0xf7:
		if (!isDataRangeValid(position, 2))
			return false;
		channel.transpose = (int8)readDataByte(position + 1);
		channel.position = (uint16)(position + 2);
		break;

	case 0xf6: {
		if (!isDataRangeValid(position, 2))
			return false;
		const byte count = readDataByte(position + 1);
		if (!count || !isDataRangeValid(position, (uint32)count + 3))
			return false;
		const uint32 table = position + 2;
		const byte selected = readDataByte(table + ((count - 1) & nextRandom()));
		const byte destination = readDataByte(table + count);
		const uint32 target = table + count + 1 + destination;
		if (!isDataRangeValid(target, 1))
			return false;
		writeDataByte(target, selected);
		channel.position = (uint16)(position + count + 3);
		break;
	}

	case 0xf5: {
		if (!isDataRangeValid(position, 2))
			return false;
		const int8 value = (int8)(((int8)readDataByte(position + 1) >> 1) - 50);
		if (!channel.pendingStop || value < channel.volumeOffset) {
			channel.volumeOffset = value;
			levelsDirty = true;
		}
		channel.position = (uint16)(position + 2);
		break;
	}

	case 0xf4:
		if (!isDataRangeValid(position, 2))
			return false;
		channel.panning = readDataByte(position + 1);
		channel.position = (uint16)(position + 2);
		updatePanning(channelIndex);
		levelsDirty = true;
		break;

	case 0xf3:
		if (!isDataRangeValid(position, 3))
			return false;
		channel.panningFadeReload = readDataByte(position + 1);
		channel.panningFadeStep = (int8)readDataByte(position + 2);
		channel.panningFadeCounter = 1;
		channel.position = (uint16)(position + 3);
		break;

	case 0xf2:
		if (!isDataRangeValid(position, 2))
			return false;
		channel.noteTranspose = (int8)readDataByte(position + 1);
		channel.position = (uint16)(position + 2);
		break;

	case 0xf1:
		if (!isDataRangeValid(position, 2))
			return false;
		channel.position = (uint16)(position + 2);
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
			channel.volumeFadeReload = getStopFadeReload();
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
	const uint operatorCount = channelIndex < 6 ? 4 : 2;

	keyOff(channelIndex);
	channel.mode = patch[0x0d];
	for (uint i = 0; i < 4; ++i)
		channel.operatorTotalLevel[i] = patch[i * 14 + 6];

	for (uint i = 0; i < operatorCount; ++i)
		programOperator(banks, channelIndex, i, patch + i * 14);

	if (channelIndex < 6) {
		const byte stereo = panningBits(channel.panning);
		const byte firstValue = stereo | ((patch[0x0a] & 7) << 1) |
				((patch[0x0d] & 1) ^ 1);
		const byte secondValue = stereo | ((patch[0x26] & 7) << 1) |
				((patch[0x29] & 1) ^ 1);
		writeRegister(banks, 0xc0 + oplChannel, firstValue);
		writeRegister(banks, 0xc3 + oplChannel, secondValue);
	} else {
		const byte value = ((patch[0x0a] & 7) << 1) |
				((channel.mode & 1) ^ 1);
		writeRegister(kFirstBank, 0xc0 + oplChannel, value | 0x20);
		writeRegister(kSecondBank, 0xc0 + oplChannel, value | 0x10);
	}

	_channelData[channelIndex].noiseMode = patch[0x38];
	_channelData[channelIndex].frequencyMask = READ_LE_UINT16(patch + 0x3a);
	_channelData[channelIndex].frequencyBase = READ_LE_UINT16(patch + 0x3c);
	_channelData[channelIndex].frequencyStep =
			(int16)READ_LE_UINT16(patch + 0x3e);

	updatePanning(channelIndex);
	updateChannelLevels(channelIndex);
}

void PSound::updatePanning(uint channelIndex) {
	const byte oplChannel = getOplChannel(channelIndex);
	if (channelIndex < 6) {
		const byte banks = getBankMask(channelIndex);
		const byte stereo = panningBits(_channels[channelIndex].panning);
		const byte reg1 = 0xc0 + oplChannel;
		const byte reg2 = 0xc3 + oplChannel;
		writeRegister(banks, reg1,
				(getCachedRegister(banks, reg1) & 0x0f) | stereo);
		writeRegister(banks, reg2,
				(getCachedRegister(banks, reg2) & 0x0f) | stereo);
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

	if (channelIndex >= 6) {
		const uint carriers[2] = { 1, 0 };
		const bool enabled[2] = { true, channel.mode == 0 };
		for (uint i = 0; i < 2; ++i) {
			if (!enabled[i])
				continue;
			const uint opIndex = carriers[i];
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
	const uint operators[4] = { 3, 1, 0, 2 };
	const bool enabled[4] = {
		true,
		channel.mode == 1,
		(channel.mode & 2) != 0,
		channel.mode == 3
	};
	for (uint i = 0; i < 4; ++i) {
		if (!enabled[i])
			continue;
		const uint opIndex = operators[i];
		const byte op = getOperatorOffset(channelIndex, opIndex);
		const byte scaling = (patch[opIndex * 14 + 7] & 3) << 6;
		const int level = clampLevel(base -
				channel.operatorTotalLevel[opIndex]);
		writeRegister(banks, 0x40 + op, scaling | level);
	}
}

void PSound::updateChannelFrequency(uint channelIndex, bool keyOn) {
	Channel &channel = _channels[channelIndex];
	updateChannelLevels(channelIndex);
	if (_channelData[channelIndex].noiseMode) {
		startNoise(channelIndex);
		return;
	}

	const byte effectiveNote = (byte)(channel.note + channel.noteTranspose);
	const byte semitone = effectiveNote % 12;
	const byte octave = effectiveNote / 12;
	const int frequency = getFrequencyNumber(semitone) + channel.transpose;
	const byte banks = getBankMask(channelIndex);
	const byte oplChannel = getOplChannel(channelIndex);
	const byte lowReg = 0xa0 + oplChannel;
	const byte highReg = 0xb0 + oplChannel;
	writeRegister(banks, lowReg, frequency & 0xff);
	byte high = ((octave & 7) << 2) | ((frequency >> 8) & 3);
	if (keyOn)
		high |= 0x20;
	writeRegister(banks, highReg, high);
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
			(getCachedRegister(banks, highReg) & 0x20) | ((frequency >> 8) & 0x1f));
}

void PSound::keyOff(uint channelIndex) {
	const byte banks = getBankMask(channelIndex);
	const byte highReg = 0xb0 + getOplChannel(channelIndex);
	writeRegister(banks, highReg, getCachedRegister(banks, highReg) & 0xdf);
}

void PSound::startNoise(uint channelIndex) {
	if (_noiseChannel[0] == channelIndex)
		_noiseState = false;
	if (_noiseChannel[1] == channelIndex)
		_noiseState = true;
	const uint slot = _noiseState ? 1 : 0;
	_noiseState = !_noiseState;
	if (_noiseTicks[slot])
		keyOff(_noiseChannel[slot]);
	_noiseChannel[slot] = channelIndex;
	_noiseTicks[slot] = _channelData[channelIndex].noiseMode;
	_noiseMask[slot] = _channelData[channelIndex].frequencyMask;
	_noiseBase[slot] = _channelData[channelIndex].frequencyBase;
	_noiseStep[slot] = _channelData[channelIndex].frequencyStep;
	resultCheck();
}

void PSound::setNoiseFrequency(uint channelIndex, int frequency) {
	const byte banks = getBankMask(channelIndex);
	const byte oplChannel = getOplChannel(channelIndex);
	writeRegister(banks, 0xa0 + oplChannel, frequency & 0xff);
	writeRegister(banks, 0xb0 + oplChannel, ((frequency >> 8) & 0x1f) | 0x20);
}

void PSound::updateNoise() {
	for (uint slot = 0; slot < 2; ++slot) {
		if (!_noiseTicks[slot])
			continue;
		_noiseBase[slot] += _noiseStep[slot];
		if (!--_noiseTicks[slot]) {
			const uint other = slot ^ 1;
			if (!_noiseTicks[other] || _noiseChannel[slot] != _noiseChannel[other])
				keyOff(_noiseChannel[slot]);
		}
	}

	if (_noiseTicks[0] == _noiseTicks[1] && _resultFlag != -1) {
		_resultFlag = -1;
		_pollResult = -1;
	}
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
} // namespace RexNebular
} // namespace MADS
