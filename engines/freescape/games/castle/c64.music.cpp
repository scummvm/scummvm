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

#include "engines/freescape/games/castle/c64.music.h"

#include "common/textconsole.h"
#include "common/util.h"
#include "freescape/freescape.h"
#include "freescape/games/castle/castle.musicdata.h"
#include "freescape/sid.h"

using namespace Freescape::CastleMusicData;

namespace Freescape {

const int kCastleSIDVoiceOffset[] = { 0, 7, 14 };
const int8 kCastleVibrato10[] = { 0, 20, 10, -10, -20, 15, 5, -20 };
const int8 kCastleVibrato18[] = { 0, 3, -3, 3, -3, 3, -3, 3 };
const int8 kCastleVibrato20[] = { 0, -100, -100, -100, -100, -100, -100, -100 };

// Original Castle Master C64 SID frequency tables at $065A/$06BA.
const byte kCastleSIDFreqLo[] = {
	0x16, 0x27, 0x38, 0x4B, 0x5F, 0x73, 0x8A, 0xA1, 0xBA, 0xD4, 0xF0, 0x0E, 0x2D, 0x4E, 0x71, 0x96,
	0xBD, 0xE7, 0x13, 0x42, 0x74, 0xA9, 0xE0, 0x1B, 0x5A, 0x9B, 0xE2, 0x2C, 0x7B, 0xCE, 0x27, 0x85,
	0xE8, 0x51, 0xC1, 0x37, 0xB4, 0x37, 0xC4, 0x57, 0xF5, 0x9C, 0x4E, 0x09, 0xD0, 0xA3, 0x82, 0x6E,
	0x68, 0x6E, 0x88, 0xAF, 0xEB, 0x39, 0x9C, 0x13, 0xA1, 0x46, 0x04, 0xDC, 0xD0, 0xDC, 0x10, 0x5E,
	0xD6, 0x72, 0x38, 0x26, 0x42, 0x8C, 0x08, 0xB8, 0xA0, 0xB8, 0x20, 0xBC, 0xAC, 0xE4, 0x70, 0x4C,
	0x84, 0x18, 0x10, 0x70, 0x40, 0x70, 0x40, 0x78, 0x58, 0xC8, 0xE0, 0x98, 0x08, 0x30, 0x20, 0x2E
};

const byte kCastleSIDFreqHi[] = {
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x06, 0x06,
	0x06, 0x07, 0x07, 0x08, 0x08, 0x09, 0x09, 0x0A, 0x0A, 0x0B, 0x0C, 0x0D, 0x0D, 0x0E, 0x0F, 0x10,
	0x11, 0x12, 0x13, 0x14, 0x15, 0x17, 0x18, 0x1A, 0x1B, 0x1D, 0x1F, 0x20, 0x22, 0x24, 0x27, 0x29,
	0x2B, 0x2E, 0x31, 0x34, 0x37, 0x3A, 0x3E, 0x41, 0x45, 0x49, 0x4E, 0x52, 0x57, 0x5C, 0x62, 0x68,
	0x6E, 0x75, 0x7C, 0x83, 0x8B, 0x93, 0x9C, 0xA5, 0xAF, 0xB9, 0xC4, 0xD0, 0xDD, 0xEA, 0xF8, 0xFD
};

const int8 *getCastleVibratoTable(byte vibrato) {
	switch (vibrato) {
	case 0x10:
		return kCastleVibrato10;
	case 0x18:
		return kCastleVibrato18;
	case 0x20:
		return kCastleVibrato20;
	default:
		return nullptr;
	}
}

void CastleC64MusicPlayer::ChannelState::reset(const byte *channelOrderList) {
	orderList = channelOrderList;
	orderPosition = 0;
	patternIndex = 0;
	patternDataOffset = 0;
	patternOffset = 0;
	delay = 0;
	instrument = 0;
	transpose = 0;
	currentNote = 0;
	baseFrequency = 0;
	frequencyOffset = 0;
	vibratoStep = 1;
	vibratoReverse = false;
	control = 0;
	active = false;
}

CastleC64MusicPlayer::CastleC64MusicPlayer()
	: _sid(nullptr),
	  _musicActive(false),
	  _tick(0) {
}

CastleC64MusicPlayer::~CastleC64MusicPlayer() {
	stopMusic();
}

void CastleC64MusicPlayer::initSID() {
	destroySID();

	_sid = SID::Config::create(SID::Config::kSidPAL);
	if (!_sid || !_sid->init()) {
		warning("CastleC64MusicPlayer: Failed to create SID emulator");
		destroySID();
		return;
	}

	_sid->start(new Common::Functor0Mem<void, CastleC64MusicPlayer>(this, &CastleC64MusicPlayer::onTimer), 50);
}

void CastleC64MusicPlayer::destroySID() {
	if (_sid) {
		_sid->stop();
		delete _sid;
		_sid = nullptr;
	}
}

void CastleC64MusicPlayer::sidWrite(int reg, byte data) {
	if (_sid)
		_sid->writeReg(reg, data);
}

void CastleC64MusicPlayer::startMusic() {
	stopMusic();
	initSID();
	if (!_sid)
		return;

	setupSong();
}

void CastleC64MusicPlayer::stopMusic() {
	_musicActive = false;
	silenceAll();
	destroySID();
}

bool CastleC64MusicPlayer::isPlaying() const {
	return _musicActive;
}

void CastleC64MusicPlayer::silenceAll() {
	for (int i = 0; i <= kSIDVolume; i++)
		sidWrite(i, 0);
}

void CastleC64MusicPlayer::setupSong() {
	silenceAll();
	_tick = 0;

	sidWrite(kSIDFilterLo, 0x00);
	sidWrite(kSIDFilterHi, 0x00);
	sidWrite(kSIDFilterCtrl, 0x00);
	sidWrite(kSIDVolume, 0x0F);

	for (int i = 0; i < kChannelCount; i++) {
		_channels[i].reset(kChannelOrderLists[i]);
		loadNextPattern(i);
	}

	_musicActive = true;
}

void CastleC64MusicPlayer::onTimer() {
	if (!_musicActive)
		return;

	for (int i = 0; i < kChannelCount; i++) {
		if (_channels[i].delay > 0) {
			if (_channels[i].active) {
				const InstrumentData &instrument = kInstruments[_channels[i].instrument % ARRAYSIZE(kInstruments)];
				if (_channels[i].delay == instrument.gateOffTime)
					gateOff(i);
				else
					applyFrameEffects(i);
			}

			_channels[i].delay--;
			continue;
		}

		parseCommands(i);
	}

	_tick++;
}

void CastleC64MusicPlayer::loadNextPattern(int channel) {
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
				"Castle SID t=%u ch=%d order=%u PATTERN %u dataOffset=%u transpose=%d",
				(uint)_tick, channel, c.orderPosition - 1, c.patternIndex, c.patternDataOffset, c.transpose);
			return;
		}
	}

	c.patternDataOffset = 0;
	c.patternOffset = 0;
}

byte CastleC64MusicPlayer::readPatternByte(int channel) {
	ChannelState &c = _channels[channel];
	uint16 offset = c.patternDataOffset + c.patternOffset;
	if (offset >= ARRAYSIZE(kPatternData)) {
		loadNextPattern(channel);
		offset = c.patternDataOffset + c.patternOffset;
	}

	c.patternOffset++;
	return kPatternData[offset];
}

void CastleC64MusicPlayer::parseCommands(int channel) {
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
			debugC(1, kFreescapeDebugMedia,
				"Castle SID t=%u ch=%d pat=%u pos=%u INST %u",
				(uint)_tick, channel, c.patternIndex, commandOffset, c.instrument);
			continue;
		}

		if (command == 0xA0) {
			byte duration = readPatternByte(channel);
			debugC(1, kFreescapeDebugMedia,
				"Castle SID t=%u ch=%d pat=%u pos=%u REST dur=%u inst=%u",
				(uint)_tick, channel, c.patternIndex, commandOffset, duration, c.instrument);
			rest(channel);
			c.delay = MAX<uint16>(1, duration);
			return;
		}

		if (command >= 0x90 && command < 0xC0) {
			debugC(1, kFreescapeDebugMedia,
				"Castle SID t=%u ch=%d pat=%u pos=%u EFFECT $%02x skipped inst=%u",
				(uint)_tick, channel, c.patternIndex, commandOffset, command, c.instrument);
			continue;
		}

		if (command >= 0xC0) {
			byte slideDuration = readPatternByte(channel);
			byte slideLo = readPatternByte(channel);
			byte slideHi = readPatternByte(channel);
			debugC(1, kFreescapeDebugMedia,
				"Castle SID t=%u ch=%d pat=%u pos=%u SLIDE $%02x dur=%u delta=$%02x%02x skipped inst=%u",
				(uint)_tick, channel, c.patternIndex, commandOffset, command, slideDuration, slideHi, slideLo, c.instrument);
			continue;
		}

		byte duration = readPatternByte(channel);
		if (command == 0) {
			debugC(1, kFreescapeDebugMedia,
				"Castle SID t=%u ch=%d pat=%u pos=%u OFF dur=%u inst=%u",
				(uint)_tick, channel, c.patternIndex, commandOffset, duration, c.instrument);
			rest(channel);
		} else {
			int effectiveNote = command + c.transpose + 20;
			noteOn(channel, command);
			debugC(1, kFreescapeDebugMedia,
				"Castle SID t=%u ch=%d pat=%u pos=%u NOTE raw=$%02x effective=%d inst=%u transpose=%d dur=%u freq=$%04x ctrl=$%02x",
				(uint)_tick, channel, c.patternIndex, commandOffset, command, effectiveNote, c.instrument,
				c.transpose, duration, c.baseFrequency, c.control);
		}
		c.delay = MAX<uint16>(1, duration);
		return;
	}

	debugC(1, kFreescapeDebugMedia,
		"Castle SID t=%u ch=%d pat=%u parser safety stop inst=%u",
		(uint)_tick, channel, c.patternIndex, c.instrument);
	rest(channel);
	c.delay = 12;
}

void CastleC64MusicPlayer::noteOn(int channel, byte note) {
	ChannelState &c = _channels[channel];
	const InstrumentData &instrument = kInstruments[c.instrument % ARRAYSIZE(kInstruments)];
	int voiceOffset = kCastleSIDVoiceOffset[channel];

	int effectiveNote = note + c.transpose + 20;
	c.currentNote = CLIP<int>(effectiveNote, 0, kMaxNote);
	c.baseFrequency = noteToSIDFrequency(effectiveNote);
	c.frequencyOffset = 0;
	c.vibratoStep = 1;
	c.vibratoReverse = false;
	c.control = sidControlForInstrument(c.instrument);
	c.active = true;

	sidWrite(voiceOffset + kSIDV1Ctrl, 0x00);
	writeFrequency(channel, c.baseFrequency);
	sidWrite(voiceOffset + kSIDV1PwLo, 0x00);
	sidWrite(voiceOffset + kSIDV1PwHi, instrument.pulseWidth & 0x0F);
	sidWrite(voiceOffset + kSIDV1AD, instrument.attackDecay);
	sidWrite(voiceOffset + kSIDV1SR, instrument.sustainRelease);
	sidWrite(voiceOffset + kSIDV1Ctrl, c.control);
}

void CastleC64MusicPlayer::rest(int channel) {
	ChannelState &c = _channels[channel];
	int voiceOffset = kCastleSIDVoiceOffset[channel];

	c.active = false;
	c.currentNote = 0;
	c.baseFrequency = 0;
	c.frequencyOffset = 0;
	c.control = 0;
	sidWrite(voiceOffset + kSIDV1Ctrl, 0x00);
}

void CastleC64MusicPlayer::gateOff(int channel) {
	ChannelState &c = _channels[channel];
	int voiceOffset = kCastleSIDVoiceOffset[channel];

	c.control &= 0xFE;
	sidWrite(voiceOffset + kSIDV1Ctrl, c.control);
}

void CastleC64MusicPlayer::writeFrequency(int channel, uint16 frequency) {
	int voiceOffset = kCastleSIDVoiceOffset[channel];
	sidWrite(voiceOffset + kSIDV1FreqLo, frequency & 0xFF);
	sidWrite(voiceOffset + kSIDV1FreqHi, frequency >> 8);
}

uint16 CastleC64MusicPlayer::noteToSIDFrequency(int note) const {
	note = CLIP<int>(note, 0, ARRAYSIZE(kCastleSIDFreqLo) - 1);
	return kCastleSIDFreqLo[note] | (kCastleSIDFreqHi[note] << 8);
}

void CastleC64MusicPlayer::applyFrameEffects(int channel) {
	ChannelState &c = _channels[channel];
	if (!c.active || c.currentNote == 0 || c.baseFrequency == 0)
		return;

	const InstrumentData &instrument = kInstruments[c.instrument % ARRAYSIZE(kInstruments)];
	const int8 *vibratoTable = getCastleVibratoTable(instrument.vibrato);
	if (!vibratoTable)
		return;

	int8 sidDelta = vibratoTable[c.vibratoStep & 0x07];
	c.frequencyOffset += c.vibratoReverse ? -sidDelta : sidDelta;
	c.vibratoStep++;
	if (c.vibratoStep >= 8) {
		c.vibratoStep = 1;
		c.vibratoReverse = !c.vibratoReverse;
	}

	int32 frequency = (int32)c.baseFrequency + c.frequencyOffset;
	writeFrequency(channel, CLIP<int32>(frequency, 0, 0xFFFF));
}

byte CastleC64MusicPlayer::sidControlForInstrument(byte instrument) const {
	const InstrumentData &instrumentData = kInstruments[instrument % ARRAYSIZE(kInstruments)];

	// The original C64 driver's waveform sequence 5 immediately switches the
	// audible waveform to triangle; using the raw noise/pulse byte here makes
	// several default-title notes disappear.
	if (instrumentData.effect == 0x05)
		return 0x11;

	return instrumentData.control;
}

} // namespace Freescape
