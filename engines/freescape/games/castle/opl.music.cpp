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

#include "engines/freescape/games/castle/opl.music.h"

#include "common/textconsole.h"
#include "common/util.h"
#include "freescape/freescape.h"
#include "freescape/games/castle/castle.musicdata.h"

using namespace Freescape::CastleMusicData;

namespace Freescape {

struct CastleOPLBasePatch {
	byte modChar;
	byte carChar;
	byte modLevel;
	byte carLevel;
	byte modAD;
	byte carAD;
	byte modSR;
	byte carSR;
	byte modWave;
	byte carWave;
	byte feedbackConnection;
};

// ============================================================================
// Embedded music data (adapted from Castle Master C64 patterns)
// ============================================================================

// OPL2 F-number/block table (95 entries)
// Format: fnum (bits 0-9) | block (bits 10-12)
// Derived from SID frequency table
const uint16 kOPLFreqs[] = {
	0x0000, 0x0168, 0x017D, 0x0194, 0x01AD, 0x01C5,
	0x01E1, 0x01FD, 0x021B, 0x023B, 0x025E, 0x0282,
	0x02A8, 0x02D0, 0x02FB, 0x0328, 0x0358, 0x038B,
	0x03C1, 0x03FA, 0x061B, 0x063C, 0x065E, 0x0682,
	0x06A7, 0x06D0, 0x06FB, 0x0728, 0x0758, 0x078B,
	0x07C1, 0x07FA, 0x0A1B, 0x0A3B, 0x0A5D, 0x0A81,
	0x0AA8, 0x0AD0, 0x0AFB, 0x0B2B, 0x0B58, 0x0B8B,
	0x0BC1, 0x0BFA, 0x0E1B, 0x0E3B, 0x0E5D, 0x0E81,
	0x0EA8, 0x0ED0, 0x0EFB, 0x0F28, 0x0F58, 0x0F8B,
	0x0FC1, 0x0FFA, 0x121B, 0x123B, 0x125D, 0x1281,
	0x12A8, 0x12D0, 0x12FB, 0x1328, 0x1358, 0x138B,
	0x13C1, 0x13FA, 0x161B, 0x163B, 0x1661, 0x1681,
	0x16A8, 0x16D0, 0x16FB, 0x1729, 0x1758, 0x178B,
	0x17C1, 0x17FA, 0x1A1B, 0x1A3B, 0x1A5D, 0x1A81,
	0x1AA8, 0x1AD0, 0x1AFB, 0x1B28, 0x1B58, 0x1B8B,
	0x1BC1, 0x1BFA, 0x1E1B, 0x1E3B, 0x1E5D
};

// ============================================================================
// OPL2 FM base patches (our own creation, but driven by the original SID data)
// ============================================================================

// OPL operator register offsets for channels 0-2
const byte kOPLModOffset[] = { 0x00, 0x01, 0x02 };
const byte kOPLCarOffset[] = { 0x03, 0x04, 0x05 };

const CastleOPLBasePatch kOPLBasePatches[] = {
	{ 0x21, 0x21, 0x22, 0x03, 0xF2, 0xF3, 0x74, 0x56, 0x00, 0x00, 0x04 },
	{ 0x02, 0x01, 0x1E, 0x00, 0xE4, 0xF2, 0x64, 0x46, 0x00, 0x01, 0x06 },
	{ 0x31, 0x21, 0x26, 0x06, 0xC3, 0xE3, 0x64, 0x47, 0x00, 0x00, 0x02 },
	{ 0x01, 0x01, 0x28, 0x04, 0xB2, 0xE4, 0x73, 0x58, 0x00, 0x00, 0x01 },
	{ 0x11, 0x01, 0x18, 0x00, 0xE2, 0xF3, 0x65, 0x47, 0x02, 0x00, 0x0C },
	{ 0x22, 0x21, 0x16, 0x00, 0xF4, 0xF2, 0x55, 0x36, 0x00, 0x00, 0x08 },
	{ 0x22, 0x21, 0x14, 0x00, 0xF4, 0xF3, 0x55, 0x26, 0x00, 0x00, 0x08 },
	{ 0x05, 0x01, 0x10, 0x00, 0xF6, 0xF4, 0x44, 0x34, 0x01, 0x00, 0x0E }
};

const byte kMusicAttenuation = 12;

byte attenuateOPLLevel(byte level) {
	return MIN<byte>((level & 0x3F) + kMusicAttenuation, 0x3F) | (level & 0xC0);
}

uint16 scaleDuration(byte duration) {
	return MAX<uint16>(1, duration);
}

// ============================================================================
// ChannelState
// ============================================================================

void CastleOPLMusicPlayer::ChannelState::reset(const byte *channelOrderList) {
	orderList = channelOrderList;
	orderPosition = 0;
	patternIndex = 0;
	patternDataOffset = 0;
	patternOffset = 0;
	delay = 0;
	instrument = 0;
	transpose = 0;
	currentNote = 0;
	baseSIDFrequency = 0;
	sidFrequencyOffset = 0;
	baseFrequencyFnum = 0;
	baseFrequencyBlock = 0;
	frequencyFnum = 0;
	frequencyBlock = 0;
	vibratoStep = 1;
	vibratoReverse = false;
	keyOn = false;
	gateReleased = true;
}

// ============================================================================
// Constructor / Destructor
// ============================================================================

CastleOPLMusicPlayer::CastleOPLMusicPlayer()
	: _opl(nullptr),
	  _musicActive(false),
	  _tick(0) {
	_opl = OPL::Config::create();
	if (!_opl || !_opl->init()) {
		warning("CastleOPLMusicPlayer: Failed to create OPL emulator");
		delete _opl;
		_opl = nullptr;
	}
}

CastleOPLMusicPlayer::~CastleOPLMusicPlayer() {
	stopMusic();
	delete _opl;
}

// ============================================================================
// Public interface
// ============================================================================

void CastleOPLMusicPlayer::startMusic() {
	if (!_opl)
		return;
	stopMusic();
	_opl->start(new Common::Functor0Mem<void, CastleOPLMusicPlayer>(
		this, &CastleOPLMusicPlayer::onTimer), 50);
	setupSong();
}

void CastleOPLMusicPlayer::stopMusic() {
	_musicActive = false;
	if (_opl) {
		silenceAll();
		_opl->stop();
	}
}

bool CastleOPLMusicPlayer::isPlaying() const {
	return _musicActive;
}

// ============================================================================
// OPL register helpers
// ============================================================================

void CastleOPLMusicPlayer::noteToFnumBlock(int note, uint16 &fnum, byte &block) const {
	if (note < 0)
		note = 0;
	if (note > kMaxNote)
		note = kMaxNote;

	uint16 combined = kOPLFreqs[note];
	fnum = combined & 0x03FF;
	block = (combined >> 10) & 0x07;
}

void CastleOPLMusicPlayer::setFrequency(int channel, uint16 fnum, byte block) {
	_channels[channel].frequencyFnum = fnum;
	_channels[channel].frequencyBlock = block;
	writeFrequency(channel, fnum, block);
}

void CastleOPLMusicPlayer::writeFrequency(int channel, uint16 fnum, byte block) {
	if (!_opl)
		return;

	_opl->writeReg(0xA0 + channel, fnum & 0xFF);
	byte b0 = ((fnum >> 8) & 0x03) | (block << 2);
	if (_channels[channel].keyOn)
		b0 |= 0x20;
	_opl->writeReg(0xB0 + channel, b0);
}

void CastleOPLMusicPlayer::setOPLInstrument(int channel, byte instrument) {
	if (!_opl)
		return;

	const CastleOPLBasePatch &patch = kOPLBasePatches[instrument % ARRAYSIZE(kOPLBasePatches)];
	byte mod = kOPLModOffset[channel];
	byte car = kOPLCarOffset[channel];

	_opl->writeReg(0x20 + mod, patch.modChar);
	_opl->writeReg(0x20 + car, patch.carChar);
	_opl->writeReg(0x40 + mod, attenuateOPLLevel(patch.modLevel));
	_opl->writeReg(0x40 + car, attenuateOPLLevel(patch.carLevel));
	_opl->writeReg(0x60 + mod, patch.modAD);
	_opl->writeReg(0x60 + car, patch.carAD);
	_opl->writeReg(0x80 + mod, patch.modSR);
	_opl->writeReg(0x80 + car, patch.carSR);
	_opl->writeReg(0xE0 + mod, patch.modWave);
	_opl->writeReg(0xE0 + car, patch.carWave);
	_opl->writeReg(0xC0 + channel, patch.feedbackConnection);
}

void CastleOPLMusicPlayer::noteOn(int channel, byte note) {
	if (!_opl)
		return;

	noteOff(channel);

	uint16 fnum = 0;
	byte block = 0;
	int effectiveNote = note + _channels[channel].transpose + 20;
	noteToFnumBlock(effectiveNote, fnum, block);
	_channels[channel].currentNote = CLIP<int>(effectiveNote, 0, kMaxNote);
	_channels[channel].baseSIDFrequency = getCastleSIDFrequency(effectiveNote);
	_channels[channel].sidFrequencyOffset = 0;
	_channels[channel].baseFrequencyFnum = fnum;
	_channels[channel].baseFrequencyBlock = block;
	_channels[channel].vibratoStep = 1;
	_channels[channel].vibratoReverse = false;
	_channels[channel].keyOn = true;
	_channels[channel].gateReleased = false;
	setFrequency(channel, fnum, block);
}

void CastleOPLMusicPlayer::noteOff(int channel) {
	if (!_opl)
		return;

	_channels[channel].keyOn = false;
	_channels[channel].gateReleased = true;
	_channels[channel].currentNote = 0;
	_channels[channel].baseSIDFrequency = 0;
	_channels[channel].sidFrequencyOffset = 0;
	writeFrequency(channel, _channels[channel].frequencyFnum, _channels[channel].frequencyBlock);
}

void CastleOPLMusicPlayer::gateOff(int channel) {
	if (!_opl || _channels[channel].gateReleased)
		return;

	_channels[channel].keyOn = false;
	_channels[channel].gateReleased = true;
	writeFrequency(channel, _channels[channel].frequencyFnum, _channels[channel].frequencyBlock);
}

// ============================================================================
// Timer / sequencer core
// ============================================================================

void CastleOPLMusicPlayer::onTimer() {
	if (!_musicActive)
		return;

	for (int i = 0; i < kChannelCount; i++) {
		if (_channels[i].delay > 0) {
			const InstrumentData &instrument = kInstruments[_channels[i].instrument % ARRAYSIZE(kInstruments)];
			if (_channels[i].currentNote != 0 && _channels[i].delay == instrument.gateOffTime)
				gateOff(i);
			applyFrameEffects(i);
			_channels[i].delay--;
			continue;
		}

		parseCommands(i);
	}

	_tick++;
}

void CastleOPLMusicPlayer::applyFrameEffects(int channel) {
	ChannelState &c = _channels[channel];
	if (c.currentNote == 0 || c.baseSIDFrequency == 0 || c.baseFrequencyFnum == 0)
		return;

	const InstrumentData &instrument = kInstruments[c.instrument % ARRAYSIZE(kInstruments)];
	const int8 *vibratoTable = getCastleVibratoTable(instrument.vibrato);
	if (!vibratoTable)
		return;

	int8 sidDelta = vibratoTable[c.vibratoStep & 0x07];
	c.sidFrequencyOffset += c.vibratoReverse ? -sidDelta : sidDelta;
	c.vibratoStep++;
	if (c.vibratoStep >= 8) {
		c.vibratoStep = 1;
		c.vibratoReverse = !c.vibratoReverse;
	}

	int32 sidFrequency = MAX<int32>(1, (int32)c.baseSIDFrequency + c.sidFrequencyOffset);
	uint32 scaledFnum = ((uint32)c.baseFrequencyFnum * sidFrequency + (c.baseSIDFrequency / 2)) / c.baseSIDFrequency;
	byte block = c.baseFrequencyBlock;

	while (scaledFnum > 0x3FF && block < 7) {
		scaledFnum = (scaledFnum + 1) >> 1;
		block++;
	}

	setFrequency(channel, MIN<uint32>(scaledFnum, 0x3FF), block);
}

// ============================================================================
// Song setup
// ============================================================================

void CastleOPLMusicPlayer::setupSong() {
	silenceAll();
	_tick = 0;

	// Enable wave select (required for non-sine waveforms)
	_opl->writeReg(0x01, 0x20);
	_opl->writeReg(0xBD, 0x00);

	for (int i = 0; i < kChannelCount; i++) {
		_channels[i].reset(kChannelOrderLists[i]);
		setOPLInstrument(i, i == 2 ? 6 : 5);
		loadNextPattern(i);
	}

	_musicActive = true;
}

void CastleOPLMusicPlayer::silenceAll() {
	if (!_opl)
		return;

	for (int i = 0; i < kChannelCount; i++) {
		_channels[i].keyOn = false;
		_opl->writeReg(0xB0 + i, 0x00);
		_opl->writeReg(0x40 + kOPLModOffset[i], 0x3F);
		_opl->writeReg(0x40 + kOPLCarOffset[i], 0x3F);
	}
}

// ============================================================================
// Pattern command parser
// ============================================================================

void CastleOPLMusicPlayer::loadNextPattern(int channel) {
	ChannelState &c = _channels[channel];
	int safety = 128;

	while (safety-- > 0) {
		byte value = c.orderList[c.orderPosition++];

		if (value == kOrderEnd) {
			c.orderPosition = 0;
			continue;
		}

		if (value == kOrderTranspose) {
			byte transpose = c.orderList[c.orderPosition++];
			c.transpose = transpose >= 0x80 ? transpose - 0x100 : transpose;
			continue;
		}

		if (value < ARRAYSIZE(kPatternOffsets)) {
			c.patternIndex = value;
			c.patternDataOffset = kPatternOffsets[value];
			c.patternOffset = 0;
			debugC(1, kFreescapeDebugMedia,
				"Castle OPL t=%u ch=%d order=%u PATTERN %u dataOffset=%u transpose=%d",
				(uint)_tick, channel, c.orderPosition - 1, c.patternIndex, c.patternDataOffset, c.transpose);
			return;
		}
	}

	c.patternDataOffset = 0;
	c.patternOffset = 0;
}

byte CastleOPLMusicPlayer::readPatternByte(int channel) {
	ChannelState &c = _channels[channel];
	uint16 offset = c.patternDataOffset + c.patternOffset;
	if (offset >= ARRAYSIZE(kPatternData)) {
		loadNextPattern(channel);
		offset = c.patternDataOffset + c.patternOffset;
	}

	c.patternOffset++;
	return kPatternData[offset];
}

void CastleOPLMusicPlayer::parseCommands(int channel) {
	ChannelState &c = _channels[channel];
	int safety = 128;

	while (safety-- > 0) {
		byte command = readPatternByte(channel);
		uint16 commandOffset = c.patternOffset - 1;

		if (command == 0xFF) {
			loadNextPattern(channel);
			continue;
		}

		if (command >= 0x80 && command < 0x90) {
			c.instrument = command & 0x0F;
			setOPLInstrument(channel, c.instrument);
			debugC(1, kFreescapeDebugMedia,
				"Castle OPL t=%u ch=%d pat=%u pos=%u INST %u",
				(uint)_tick, channel, c.patternIndex, commandOffset, c.instrument);
			continue;
		}

		if (command == 0xA0) {
			byte duration = readPatternByte(channel);
			debugC(1, kFreescapeDebugMedia,
				"Castle OPL t=%u ch=%d pat=%u pos=%u REST dur=%u inst=%u",
				(uint)_tick, channel, c.patternIndex, commandOffset, duration, c.instrument);
			noteOff(channel);
			c.delay = scaleDuration(duration);
			return;
		}

		if (command >= 0x90 && command < 0xC0) {
			debugC(1, kFreescapeDebugMedia,
				"Castle OPL t=%u ch=%d pat=%u pos=%u EFFECT $%02x skipped inst=%u",
				(uint)_tick, channel, c.patternIndex, commandOffset, command, c.instrument);
			continue;
		}

		if (command >= 0xC0) {
			byte slideDuration = readPatternByte(channel);
			byte slideLo = readPatternByte(channel);
			byte slideHi = readPatternByte(channel);
			debugC(1, kFreescapeDebugMedia,
				"Castle OPL t=%u ch=%d pat=%u pos=%u SLIDE $%02x dur=%u delta=$%02x%02x skipped inst=%u",
				(uint)_tick, channel, c.patternIndex, commandOffset, command, slideDuration, slideHi, slideLo, c.instrument);
			continue;
		}

		byte duration = readPatternByte(channel);
		if (command == 0) {
			debugC(1, kFreescapeDebugMedia,
				"Castle OPL t=%u ch=%d pat=%u pos=%u OFF dur=%u inst=%u",
				(uint)_tick, channel, c.patternIndex, commandOffset, duration, c.instrument);
			noteOff(channel);
		} else {
			int effectiveNote = command + c.transpose + 20;
			noteOn(channel, command);
			debugC(1, kFreescapeDebugMedia,
				"Castle OPL t=%u ch=%d pat=%u pos=%u NOTE raw=$%02x effective=%d inst=%u transpose=%d dur=%u fnum=$%03x block=%u",
				(uint)_tick, channel, c.patternIndex, commandOffset, command, effectiveNote, c.instrument,
				c.transpose, duration, c.frequencyFnum, c.frequencyBlock);
		}
		c.delay = scaleDuration(duration);
		return;
	}

	debugC(1, kFreescapeDebugMedia,
		"Castle OPL t=%u ch=%d pat=%u parser safety stop inst=%u",
		(uint)_tick, channel, c.patternIndex, c.instrument);
	noteOff(channel);
	c.delay = 12;
}

} // namespace Freescape
