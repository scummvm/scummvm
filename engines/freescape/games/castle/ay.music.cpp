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

#include "engines/freescape/games/castle/ay.music.h"

#include "common/textconsole.h"
#include "common/util.h"
#include "freescape/freescape.h"
#include "freescape/games/castle/castle.musicdata.h"

using namespace Freescape::CastleMusicData;

namespace Freescape {

const byte kAYInstrumentVolumes[] = {
	13, 11, 10, 11, 13, 13, 14, 12
};

const uint16 kAttackRate[16] = {
	0x0F00, 0x0F00, 0x0F00, 0x0C80,
	0x07E5, 0x055B, 0x0469, 0x03C0,
	0x0300, 0x0133, 0x009A, 0x0060,
	0x004D, 0x001A, 0x000F, 0x000A
};

const uint16 kDecayReleaseRate[16] = {
	0x0F00, 0x0C80, 0x0640, 0x042B,
	0x02A2, 0x01C9, 0x0178, 0x0140,
	0x0100, 0x0066, 0x0033, 0x0020,
	0x001A, 0x0009, 0x0005, 0x0003
};

const uint32 kSIDToAYPeriodScale = 1064276;

uint16 sidFrequencyToAYPeriod(uint16 sidFrequency) {
	if (sidFrequency == 0)
		return 0;

	return CLIP<uint32>((kSIDToAYPeriodScale + (sidFrequency / 2)) / sidFrequency, 1, 4095);
}

uint16 applySIDFrequencyOffset(uint16 baseSIDFrequency, int16 frequencyOffset) {
	int32 sidFrequency = MAX<int32>(1, (int32)baseSIDFrequency + frequencyOffset);
	return sidFrequencyToAYPeriod(sidFrequency);
}

void CastleAYMusicPlayer::ChannelState::reset(const byte *channelOrderList) {
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
	basePeriod = 0;
	currentPeriod = 0;
	adsrVolume = 0;
	attackRate = 0;
	decayRate = 0;
	sustainLevel = 0;
	releaseRate = 0;
	sidFrequencyOffset = 0;
	vibratoStep = 1;
	vibratoReverse = false;
	toneEnabled = false;
	noiseEnabled = false;
	active = false;
	adsrPhase = kPhaseOff;
}

CastleAYMusicPlayer::CastleAYMusicPlayer(Audio::Mixer *mixer)
	: AY8912Stream(44100, 1000000),
	  _mixer(mixer),
	  _musicActive(false),
	  _mixerRegister(0x38),
	  _tickSampleCount(0),
	  _tick(0) {
}

CastleAYMusicPlayer::~CastleAYMusicPlayer() {
	stopMusic();
}

void CastleAYMusicPlayer::startMusic() {
	if (!_mixer)
		return;

	stopMusic();
	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_handle, toAudioStream(),
	                   -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
	setupSong();
}

void CastleAYMusicPlayer::stopMusic() {
	_musicActive = false;
	silenceAll();
	if (_mixer)
		_mixer->stopHandle(_handle);
}

bool CastleAYMusicPlayer::isPlaying() const {
	return _musicActive;
}

int CastleAYMusicPlayer::readBuffer(int16 *buffer, const int numSamples) {
	if (!_musicActive) {
		memset(buffer, 0, numSamples * sizeof(int16));
		return numSamples;
	}

	int samplesGenerated = 0;
	int samplesPerTick = (getRate() / 50) * 2;

	while (samplesGenerated < numSamples) {
		int remaining = samplesPerTick - _tickSampleCount;
		int toGenerate = MIN(numSamples - samplesGenerated, remaining);

		if (toGenerate > 0) {
			generateSamples(buffer + samplesGenerated, toGenerate);
			samplesGenerated += toGenerate;
			_tickSampleCount += toGenerate;
		}

		if (_tickSampleCount >= samplesPerTick) {
			_tickSampleCount -= samplesPerTick;
			onTimer();
		}
	}

	return samplesGenerated;
}

void CastleAYMusicPlayer::onTimer() {
	if (!_musicActive)
		return;

	for (int i = 0; i < kChannelCount; i++) {
		if (_channels[i].delay > 0) {
			if (_channels[i].active) {
				const InstrumentData &instrument = kInstruments[_channels[i].instrument % ARRAYSIZE(kInstruments)];
				if (_channels[i].delay == instrument.gateOffTime)
					releaseADSR(i);
			}
			applyFrameEffects(i);
			updateADSR(i);
			_channels[i].delay--;
			continue;
		}

		parseCommands(i);
		applyFrameEffects(i);
		updateADSR(i);
	}

	_tick++;
}

void CastleAYMusicPlayer::setupSong() {
	silenceAll();
	_tick = 0;
	_tickSampleCount = 0;

	_mixerRegister = 0x38;
	setReg(7, _mixerRegister);
	setReg(6, 0x07);

	for (int i = 0; i < kChannelCount; i++) {
		_channels[i].reset(kChannelOrderLists[i]);
		loadNextPattern(i);
	}

	_musicActive = true;
}

void CastleAYMusicPlayer::silenceAll() {
	for (int r = 0; r < 14; r++)
		setReg(r, 0);

	_mixerRegister = 0x3F;
	setReg(7, _mixerRegister);
}

void CastleAYMusicPlayer::loadNextPattern(int channel) {
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
				"Castle AY t=%u ch=%d order=%u PATTERN %u dataOffset=%u transpose=%d",
				(uint)_tick, channel, c.orderPosition - 1, c.patternIndex, c.patternDataOffset, c.transpose);
			return;
		}
	}

	c.patternDataOffset = 0;
	c.patternOffset = 0;
}

byte CastleAYMusicPlayer::readPatternByte(int channel) {
	ChannelState &c = _channels[channel];
	uint16 offset = c.patternDataOffset + c.patternOffset;
	if (offset >= ARRAYSIZE(kPatternData)) {
		loadNextPattern(channel);
		offset = c.patternDataOffset + c.patternOffset;
	}

	c.patternOffset++;
	return kPatternData[offset];
}

void CastleAYMusicPlayer::parseCommands(int channel) {
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
				"Castle AY t=%u ch=%d pat=%u pos=%u INST %u",
				(uint)_tick, channel, c.patternIndex, commandOffset, c.instrument);
			continue;
		}

		if (command == 0xA0) {
			byte duration = readPatternByte(channel);
			debugC(1, kFreescapeDebugMedia,
				"Castle AY t=%u ch=%d pat=%u pos=%u REST dur=%u inst=%u",
				(uint)_tick, channel, c.patternIndex, commandOffset, duration, c.instrument);
			noteOff(channel);
			c.delay = MAX<uint16>(1, duration);
			return;
		}

		if (command >= 0x90 && command < 0xC0) {
			debugC(1, kFreescapeDebugMedia,
				"Castle AY t=%u ch=%d pat=%u pos=%u EFFECT $%02x skipped inst=%u",
				(uint)_tick, channel, c.patternIndex, commandOffset, command, c.instrument);
			continue;
		}

		if (command >= 0xC0) {
			byte slideDuration = readPatternByte(channel);
			byte slideLo = readPatternByte(channel);
			byte slideHi = readPatternByte(channel);
			debugC(1, kFreescapeDebugMedia,
				"Castle AY t=%u ch=%d pat=%u pos=%u SLIDE $%02x dur=%u delta=$%02x%02x skipped inst=%u",
				(uint)_tick, channel, c.patternIndex, commandOffset, command, slideDuration, slideHi, slideLo, c.instrument);
			continue;
		}

		byte duration = readPatternByte(channel);
		if (command == 0) {
			debugC(1, kFreescapeDebugMedia,
				"Castle AY t=%u ch=%d pat=%u pos=%u OFF dur=%u inst=%u",
				(uint)_tick, channel, c.patternIndex, commandOffset, duration, c.instrument);
			noteOff(channel);
		} else {
			int effectiveNote = command + c.transpose + 20;
			noteOn(channel, command);
			debugC(1, kFreescapeDebugMedia,
				"Castle AY t=%u ch=%d pat=%u pos=%u NOTE raw=$%02x effective=%d inst=%u transpose=%d dur=%u period=%u",
				(uint)_tick, channel, c.patternIndex, commandOffset, command, effectiveNote, c.instrument,
				c.transpose, duration, c.currentPeriod);
		}
		c.delay = MAX<uint16>(1, duration);
		return;
	}

	debugC(1, kFreescapeDebugMedia,
		"Castle AY t=%u ch=%d pat=%u parser safety stop inst=%u",
		(uint)_tick, channel, c.patternIndex, c.instrument);
	noteOff(channel);
	c.delay = 12;
}

void CastleAYMusicPlayer::noteOn(int channel, byte note) {
	ChannelState &c = _channels[channel];
	const InstrumentData &instrument = kInstruments[c.instrument % ARRAYSIZE(kInstruments)];
	byte control = sidControlForInstrument(c.instrument);

	if ((control & 0x01) == 0) {
		noteOff(channel);
		return;
	}

	int effectiveNote = note + c.transpose + 20;
	c.baseSIDFrequency = getCastleSIDFrequency(effectiveNote);
	c.basePeriod = sidFrequencyToAYPeriod(c.baseSIDFrequency);
	c.sidFrequencyOffset = 0;
	c.vibratoStep = 1;
	c.vibratoReverse = false;
	writeChannelPeriod(channel, c.basePeriod);

	bool noiseEnabled = (control & 0x80) != 0;
	bool toneEnabled = (control & 0x70) != 0;
	if (noiseEnabled)
		writeNoisePeriod(c.basePeriod);
	setChannelOutput(channel, toneEnabled, noiseEnabled);

	c.currentNote = CLIP<int>(effectiveNote, 0, kMaxNote);
	c.active = true;
	triggerADSR(channel, instrument.attackDecay, instrument.sustainRelease);
}

void CastleAYMusicPlayer::noteOff(int channel) {
	ChannelState &c = _channels[channel];
	c.active = false;
	c.adsrPhase = kPhaseOff;
	c.adsrVolume = 0;
	c.currentNote = 0;
	c.baseSIDFrequency = 0;
	c.basePeriod = 0;
	c.sidFrequencyOffset = 0;
	setReg(8 + channel, 0);
	setChannelOutput(channel, false, false);
}

void CastleAYMusicPlayer::writeChannelPeriod(int channel, uint16 period) {
	_channels[channel].currentPeriod = period;
	setReg(channel * 2, period & 0xFF);
	setReg(channel * 2 + 1, (period >> 8) & 0x0F);
}

void CastleAYMusicPlayer::noteToPeriod(int note, uint16 &period) const {
	if (note < 0)
		note = 0;
	if (note > kMaxNote)
		note = kMaxNote;

	period = sidFrequencyToAYPeriod(getCastleSIDFrequency(note));
}

void CastleAYMusicPlayer::applyFrameEffects(int channel) {
	ChannelState &c = _channels[channel];
	if (c.adsrPhase == kPhaseOff || c.currentNote == 0 || c.baseSIDFrequency == 0)
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

	uint16 period = applySIDFrequencyOffset(c.baseSIDFrequency, c.sidFrequencyOffset);
	writeChannelPeriod(channel, period);
	if (c.noiseEnabled)
		writeNoisePeriod(period);
}

void CastleAYMusicPlayer::triggerADSR(int channel, byte attackDecay, byte sustainRelease) {
	ChannelState &c = _channels[channel];
	c.adsrPhase = kPhaseAttack;
	c.attackRate = kAttackRate[attackDecay >> 4];
	c.decayRate = kDecayReleaseRate[attackDecay & 0x0F];
	c.sustainLevel = sustainRelease >> 4;
	c.releaseRate = kDecayReleaseRate[sustainRelease & 0x0F];
}

void CastleAYMusicPlayer::releaseADSR(int channel) {
	ChannelState &c = _channels[channel];
	c.active = false;
	if (c.adsrPhase != kPhaseOff && c.adsrPhase != kPhaseRelease)
		c.adsrPhase = kPhaseRelease;
}

void CastleAYMusicPlayer::updateADSR(int channel) {
	ChannelState &c = _channels[channel];

	switch (c.adsrPhase) {
	case kPhaseAttack:
		c.adsrVolume += c.attackRate;
		if (c.adsrVolume >= 0x0F00) {
			c.adsrVolume = 0x0F00;
			c.adsrPhase = kPhaseDecay;
		}
		break;

	case kPhaseDecay: {
		uint16 sustainTarget = (uint16)c.sustainLevel << 8;
		if (c.adsrVolume > c.decayRate + sustainTarget) {
			c.adsrVolume -= c.decayRate;
		} else {
			c.adsrVolume = sustainTarget;
			c.adsrPhase = kPhaseSustain;
		}
		break;
	}

	case kPhaseSustain:
		break;

	case kPhaseRelease:
		if (c.adsrVolume > c.releaseRate) {
			c.adsrVolume -= c.releaseRate;
		} else {
			c.adsrVolume = 0;
			c.adsrPhase = kPhaseOff;
			c.currentNote = 0;
			setChannelOutput(channel, false, false);
		}
		break;

	case kPhaseOff:
		c.adsrVolume = 0;
		break;
	}

	byte volume = (c.adsrVolume >> 8) * instrumentVolumeScale(c.instrument) / 15;
	setReg(8 + channel, volume);
}

void CastleAYMusicPlayer::setChannelOutput(int channel, bool toneEnabled, bool noiseEnabled) {
	_channels[channel].toneEnabled = toneEnabled;
	_channels[channel].noiseEnabled = noiseEnabled;

	if (toneEnabled)
		_mixerRegister &= ~(1 << channel);
	else
		_mixerRegister |= (1 << channel);

	if (noiseEnabled)
		_mixerRegister &= ~(1 << (channel + 3));
	else
		_mixerRegister |= (1 << (channel + 3));

	setReg(7, _mixerRegister);
}

void CastleAYMusicPlayer::writeNoisePeriod(uint16 period) {
	setReg(6, CLIP<uint16>(period >> 5, 1, 31));
}

byte CastleAYMusicPlayer::sidControlForInstrument(byte instrument) const {
	return getCastleSIDControlForInstrument(instrument);
}

byte CastleAYMusicPlayer::instrumentVolumeScale(byte instrument) const {
	return kAYInstrumentVolumes[instrument % ARRAYSIZE(kAYInstrumentVolumes)];
}

} // namespace Freescape
