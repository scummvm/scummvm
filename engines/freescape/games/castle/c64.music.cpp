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
	return getCastleSIDFrequency(note);
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
	return getCastleSIDControlForInstrument(instrument);
}

} // namespace Freescape
