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

#include "engines/freescape/games/driller/opl.music.h"

#include "common/textconsole.h"
#include "common/util.h"
#include "freescape/freescape.h"
#include "freescape/games/driller/driller.musicdata.h"

namespace Freescape {

struct DrillerOPLBasePatch {
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

const uint16 kDrillerOPLFreqs[] = {
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

const byte kOPLModOffset[] = { 0x00, 0x01, 0x02 };
const byte kOPLCarOffset[] = { 0x03, 0x04, 0x05 };

const DrillerOPLBasePatch kDrillerOPLBasePatches[] = {
	{ 0x21, 0x21, 0x22, 0x04, 0xF2, 0xF3, 0x74, 0x45, 0x00, 0x00, 0x04 }, // triangle
	{ 0x02, 0x01, 0x1C, 0x00, 0xE3, 0xF2, 0x63, 0x35, 0x00, 0x01, 0x06 }, // pulse
	{ 0x31, 0x21, 0x25, 0x03, 0xD3, 0xE3, 0x64, 0x46, 0x00, 0x00, 0x02 }, // saw
	{ 0x01, 0x01, 0x2A, 0x08, 0xF4, 0xF2, 0x42, 0x31, 0x00, 0x00, 0x0E }, // noise/percussion
	{ 0x22, 0x21, 0x18, 0x00, 0xF4, 0xF2, 0x55, 0x36, 0x00, 0x00, 0x08 }  // default lead
};

const byte kDrillerMusicAttenuation = 4;
const byte kDrillerNoiseAttenuation = 0;
const byte kDrillerNarrowPulseAttenuation = 2;
const uint16 kDrillerNarrowPulseEdgeDistance = 0x0300;
const byte kDrillerPulseBrightnessBoostMax = 24;
const int kDrillerArpeggioSize = 3;

byte attenuateDrillerOPLLevel(byte level, byte attenuation) {
	return MIN<byte>((level & 0x3F) + attenuation, 0x3F) | (level & 0xC0);
}

byte getDrillerOPLAttenuation(byte control) {
	return (control & 0x80) ? kDrillerNoiseAttenuation : kDrillerMusicAttenuation;
}

uint16 getDrillerSIDFrequency(uint8_t note) {
	if (note >= 96)
		note = 95;
	return frq_lo[note] | (frq_hi[note] << 8);
}

byte getDrillerWaveformFamily(byte control) {
	if (control & 0x80)
		return 3;
	if (control & 0x40)
		return 1;
	if (control & 0x20)
		return 2;
	if (control & 0x10)
		return 0;
	return 4;
}

byte getDrillerInstrumentControl(const uint8_t *instA0, const uint8_t *instA1, bool useAlternateControl) {
	if (useAlternateControl && (instA1[2] & 0xF0))
		return instA1[2];
	if (instA0[1] & 0xF0)
		return instA0[1];
	if (instA0[6] & 0xF0)
		return instA0[6];
	if (instA1[2] & 0xF0)
		return instA1[2];
	return instA0[1];
}

// SID sustain level (0-15, linear) -> OPL2 sustain-level nibble (attenuation).
const byte kDrillerSidSustainToOPL[16] = {
	15, 8, 6, 5, 4, 3, 3, 2, 2, 2, 1, 1, 1, 0, 0, 0
};

// SID rate (0=fast..15=slow) -> OPL rate (15=fast..0=never), inverted and
// compressed into [5,15] so even the slowest SID rate stays audible.
byte sidRateToOPL(byte sidRate) {
	return 15 - (sidRate * 10) / 15;
}

// Convert a SID instrument's AD/SR bytes to OPL2 carrier envelope registers.
void deriveDrillerOPLEnvelope(byte sidAD, byte sidSR, byte &oplAD, byte &oplSR, bool &sustaining) {
	byte sidAttack = sidAD >> 4;
	byte sidSustain = sidSR >> 4;
	byte attack = sidRateToOPL(sidAttack);
	byte decay = sidRateToOPL(sidAD & 0x0F);
	byte release = sidRateToOPL(sidSR & 0x0F);

	byte sustainLevel;
	if (sidSustain != 0) {
		sustaining = true;
		sustainLevel = kDrillerSidSustainToOPL[sidSustain];
	} else if (sidAttack >= 10) {
		// Slow-attack swell, no SID sustain: hold it so the note does not
		// collapse into a short puff (instruments 1, 2, 14).
		sustaining = true;
		sustainLevel = 0;
	} else {
		// Fast attack, no sustain: plucked voice that decays away.
		sustaining = false;
		sustainLevel = 15;
	}

	oplAD = (attack << 4) | decay;
	oplSR = (sustainLevel << 4) | release;
}

void DrillerOPLMusicPlayer::VoiceState::reset() {
	trackDataPtr = nullptr;
	trackIndex = 0;
	patternDataPtr = nullptr;
	patternIndex = 0;
	instrumentIndex = 0;
	delayCounter = 0;
	noteDuration = 0;
	gateMask = 0xFF;
	currentNote = 0;
	currentNoteSlideTarget = 0;
	currentControl = 0;
	whatever0 = 0;
	whatever1 = 0;
	whatever2 = 0;
	whatever3 = 0;
	whatever4 = 0;
	whatever2_vibDirToggle = 0;
	portaStepRaw = 0;
	memset(something_else, 0, sizeof(something_else));
	ctrl0 = 0;
	arpTableIndex = 0;
	arpSpeedHiNibble = 0;
	stuff_freq_porta_vib = 0;
	stuff_freq_base = 0;
	stuff_arp_counter = 0;
	things_vib_state = 0;
	things_vib_depth = 0;
	things_vib_delay_reload = 0;
	things_vib_delay_ctr = 0;
	glideDownTimer = 0;
	baseSIDFrequency = 0;
	baseFrequencyFnum = 0;
	baseFrequencyBlock = 0;
	frequencyFnum = 0;
	frequencyBlock = 0;
	keyOn = false;
	gateReleased = true;
}

DrillerOPLMusicPlayer::DrillerOPLMusicPlayer(int tuneIndex)
	: _opl(nullptr),
	  _musicActive(false),
	  _targetTuneIndex(tuneIndex),
	  _globalTempo(3),
	  _globalTempoCounter(1),
	  _tick(0) {
	_opl = OPL::Config::create();
	if (!_opl || !_opl->init()) {
		warning("DrillerOPLMusicPlayer: Failed to create OPL emulator");
		delete _opl;
		_opl = nullptr;
	}
}

DrillerOPLMusicPlayer::~DrillerOPLMusicPlayer() {
	stopMusic();
	delete _opl;
}

void DrillerOPLMusicPlayer::startMusic() {
	if (!_opl)
		return;

	stopMusic();
	_opl->start(new Common::Functor0Mem<void, DrillerOPLMusicPlayer>(
		this, &DrillerOPLMusicPlayer::onTimer), 50);
	setupTune(_targetTuneIndex);
}

void DrillerOPLMusicPlayer::stopMusic() {
	_musicActive = false;
	if (_opl) {
		silenceAll();
		_opl->stop();
	}
}

bool DrillerOPLMusicPlayer::isPlaying() const {
	return _musicActive;
}

void DrillerOPLMusicPlayer::setupTune(int tuneIndex) {
	if (tuneIndex < 1 || tuneIndex >= NUM_TUNES)
		tuneIndex = 1;

	silenceAll();
	_tick = 0;
	_globalTempo = tune_tempo_data[tuneIndex];
	if (_globalTempo == 0)
		_globalTempo = 1;
	_globalTempoCounter = 1;

	if (_opl) {
		_opl->writeReg(0x01, 0x20);
		_opl->writeReg(0xBD, 0x00);
	}

	const uint8_t *const *currentTuneTracks = tune_track_data[tuneIndex];
	for (int i = 0; i < kChannelCount; ++i) {
		_voiceState[i].reset();
		_voiceState[i].trackDataPtr = currentTuneTracks ? currentTuneTracks[i] : nullptr;
	}

	resetVoices();
	_musicActive = true;
}

void DrillerOPLMusicPlayer::resetVoices() {
	for (int i = 0; i < kChannelCount; ++i) {
		VoiceState &v = _voiceState[i];
		if (!v.trackDataPtr) {
			noteOff(i);
			continue;
		}

		v.trackIndex = 0;
		v.patternDataPtr = nullptr;
		v.patternIndex = 0;
		v.instrumentIndex = initialInstrumentIndex[i];
		v.delayCounter = 0;
		v.noteDuration = initialSomethingData[i][2];
		v.gateMask = 0xFF;
		v.currentNote = initialStuffData[i][3];
		v.currentNoteSlideTarget = initialThingsData[i][6];
		v.currentControl = 0;
		v.whatever0 = 0;
		v.whatever1 = 0;
		v.whatever2 = 0;
		v.whatever3 = 0;
		v.whatever4 = 0;
		v.whatever2_vibDirToggle = initialPwDirection[i];
		v.portaStepRaw = initialSomethingData[i][0];
		memcpy(v.something_else, initialSomethingElseData[i], sizeof(v.something_else));
		v.ctrl0 = initialCtrl0[i];
		v.arpTableIndex = 0;
		v.arpSpeedHiNibble = initialStuffData[i][5];
		v.stuff_freq_porta_vib = initialStuffData[i][0] | (initialStuffData[i][4] << 8);
		v.stuff_freq_base = initialStuffData[i][1] | (initialStuffData[i][2] << 8);
		v.stuff_arp_counter = initialStuffData[i][6];
		v.things_vib_state = initialThingsData[i][0];
		v.things_vib_depth = initialThingsData[i][1];
		v.things_vib_delay_reload = initialThingsData[i][2];
		v.things_vib_delay_ctr = initialThingsData[i][3];
		v.glideDownTimer = initialTwoCtr[i];
		v.baseSIDFrequency = 0;
		v.keyOn = false;
		v.gateReleased = true;
	}
}

void DrillerOPLMusicPlayer::silenceAll() {
	if (!_opl)
		return;

	_opl->writeReg(0xBD, 0x00);

	for (int i = 0; i < kChannelCount; ++i) {
		_voiceState[i].keyOn = false;
		_voiceState[i].gateReleased = true;
		_opl->writeReg(0xB0 + i, 0x00);
		_opl->writeReg(0x40 + kOPLModOffset[i], 0x3F);
		_opl->writeReg(0x40 + kOPLCarOffset[i], 0x3F);
	}
}

void DrillerOPLMusicPlayer::onTimer() {
	if (!_musicActive)
		return;

	for (int i = 0; i < kChannelCount; ++i)
		playVoice(i);

	_globalTempoCounter--;
	if (_globalTempoCounter < 0)
		_globalTempoCounter = _globalTempo;

	_tick++;
}

void DrillerOPLMusicPlayer::playVoice(int channel) {
	VoiceState &v = _voiceState[channel];
	if (!v.trackDataPtr)
		return;

	int instBase = v.instrumentIndex;
	if (instBase < 0 || instBase >= NUM_INSTRUMENTS * 8) {
		instBase = 0;
		v.instrumentIndex = 0;
	}
	const uint8_t *instA0 = &instrumentDataA0[instBase];
	const uint8_t *instA1 = &instrumentDataA1[instBase];

	if (instA0[7] & 0x04) {
		bool useAlternateControl = false;
		if (v.glideDownTimer != 0) {
			v.glideDownTimer--;
			useAlternateControl = instA1[2] != 0;
		}

		byte control = getDrillerInstrumentControl(instA0, instA1, useAlternateControl);
		if (control != v.currentControl) {
			v.currentControl = control;
			debugC(1, kFreescapeDebugMedia,
				"Driller OPL control tick=%u ch=%d inst=%u note=%u ctrl=%02x alt=%d",
				_tick, channel, v.instrumentIndex / 8, v.currentNote, v.currentControl, useAlternateControl ? 1 : 0);
			setOPLInstrument(channel, v);
		}
	}

	if (_globalTempoCounter != 0) {
		applyContinuousEffects(channel, v, instA0, instA1, false);
		return;
	}

	v.delayCounter--;
	if (v.delayCounter >= 0) {
		applyContinuousEffects(channel, v, instA0, instA1, false);
		return;
	}

	uint8_t patternNum = v.trackDataPtr[v.trackIndex];
	if (patternNum == 0xFE) {
		stopMusic();
		return;
	}

	if (patternNum == 0xFF) {
		v.trackIndex = 0;
		patternNum = v.trackDataPtr[v.trackIndex];
		if (patternNum == 0xFF || patternNum == 0xFE) {
			stopMusic();
			return;
		}
	}

	if (patternNum >= NUM_PATTERNS) {
		warning("Driller OPL: invalid pattern number %d on channel %d", patternNum, channel);
		stopMusic();
		return;
	}

	v.patternDataPtr = pattern_addresses[patternNum];
	v.gateMask = 0xFF;
	v.whatever2 = 0;
	v.whatever1 = 0;
	v.whatever0 = 0;

	int safety = 128;
	while (safety-- > 0) {
		uint8_t cmd = v.patternDataPtr[v.patternIndex];

		if (cmd >= 0xFD) {
			v.patternIndex++;
			v.noteDuration = v.patternDataPtr[v.patternIndex++];
			continue;
		}

		if (cmd >= 0xFB) {
			v.patternIndex++;
			v.whatever2 = (cmd == 0xFB) ? 1 : 2;
			v.portaStepRaw = v.patternDataPtr[v.patternIndex++];
			v.whatever1 = 0;
			v.whatever0 = 0;
			continue;
		}

		if (cmd >= 0xFA) {
			v.patternIndex++;
			uint8_t instNum = v.patternDataPtr[v.patternIndex++];
			if (instNum >= NUM_INSTRUMENTS)
				instNum = 0;
			v.instrumentIndex = instNum * 8;
			instBase = v.instrumentIndex;
			instA0 = &instrumentDataA0[instBase];
			instA1 = &instrumentDataA1[instBase];

			uint8_t instrumentByte = instA0[0];
			v.something_else[0] = instrumentByte & 0xF0;
			v.something_else[1] = instrumentByte & 0xF0;
			v.something_else[2] = instrumentByte & 0x0F;
			v.ctrl0 = instrumentByte & 0x0F;
			v.currentControl = getDrillerInstrumentControl(instA0, instA1, false);
			continue;
		}

		v.currentNote = cmd;
		v.delayCounter = v.noteDuration;
		v.whatever3 = 0;
		v.whatever4 = 0;
		v.glideDownTimer = 2;

		applyNote(channel, v, instA0, instA1);

		v.patternIndex++;
		if (v.patternDataPtr[v.patternIndex] == 0xFF) {
			v.patternIndex = 0;
			v.trackIndex++;
			uint8_t trackCmd = v.trackDataPtr[v.trackIndex];
			if (trackCmd == 0xFF)
				v.trackIndex = 0;
			else if (trackCmd == 0xFE) {
				stopMusic();
				return;
			}
		}

		ContinuousEffectEntry entry = postNoteEffectSetup(v, instA0, instA1);
		if (entry == kFullEffectPath)
			applyContinuousEffects(channel, v, instA0, instA1, false);
		else if (entry == kPortamentoOnlyPath)
			applyContinuousEffects(channel, v, instA0, instA1, true);
		return;
	}

	warning("Driller OPL: parser safety stop on channel %d", channel);
	noteOff(channel);
	v.delayCounter = 12;
}

void DrillerOPLMusicPlayer::applyNote(int channel, VoiceState &v, const uint8_t *instA0, const uint8_t *instA1) {
	uint8_t note = v.currentNote;

	if (instA0[7] & 0x02) {
		v.something_else[0] = v.something_else[1];
		v.something_else[2] = v.ctrl0;
	}

	if (note == 0) {
		note = v.currentNoteSlideTarget;
		v.currentNote = note;
		v.currentNoteSlideTarget = 0;
		v.gateMask--;
		noteOff(channel);
		return;
	}

	if (note >= 96)
		note = 95;

	v.currentNote = note;
	v.currentNoteSlideTarget = note;
	v.stuff_freq_porta_vib = getDrillerSIDFrequency(note);
	v.stuff_freq_base = v.stuff_freq_porta_vib;
	v.baseSIDFrequency = v.stuff_freq_base;
	v.currentControl = getDrillerInstrumentControl(instA0, instA1, false);
	debugC(1, kFreescapeDebugMedia,
		"Driller OPL note tick=%u ch=%d inst=%u note=%u ctrl=%02x track=%u pat=%u",
		_tick, channel, v.instrumentIndex / 8, note, v.currentControl, v.trackIndex, v.patternIndex);
	setOPLInstrument(channel, v);
	noteOn(channel, v, note);
}

DrillerOPLMusicPlayer::ContinuousEffectEntry DrillerOPLMusicPlayer::postNoteEffectSetup(VoiceState &v, const uint8_t *instA0, const uint8_t *instA1) {
	if (v.currentNoteSlideTarget == 0)
		return kFullEffectPath;

	if (v.whatever2 != 0)
		return kPortamentoOnlyPath;

	if (instA1[4] != 0) {
		v.whatever2 = instA1[4];
		v.portaStepRaw = instA1[3];
		return kPortamentoOnlyPath;
	}

	if (instA1[0] != 0) {
		if (instA0[5] != 0) {
			v.arpTableIndex = instA0[5] & 0x0F;
			v.arpSpeedHiNibble = (instA0[5] & 0xF0) >> 4;
			v.stuff_arp_counter = 0;
			v.whatever1 = 1;
			v.whatever0 = 0;
			return kVoiceDone;
		}

		v.whatever1 = 0;
		v.things_vib_depth = instA1[0];
		v.things_vib_delay_reload = instA1[1];
		v.things_vib_delay_ctr = instA1[1];
		v.things_vib_state = 0;
		v.whatever0 = 1;
		return kVoiceDone;
	}

	if (instA0[5] != 0) {
		v.arpTableIndex = instA0[5] & 0x0F;
		v.arpSpeedHiNibble = (instA0[5] & 0xF0) >> 4;
		v.stuff_arp_counter = 0;
		v.whatever1 = 1;
		v.whatever0 = 0;
		return kVoiceDone;
	}

	v.whatever1 = 0;
	v.whatever0 = 0;
	return kVoiceDone;
}

void DrillerOPLMusicPlayer::applyContinuousEffects(int channel, VoiceState &v, const uint8_t *instA0, const uint8_t *instA1, bool startAtPortamento) {
	if (!startAtPortamento) {
		uint8_t lfoSpeed = instA0[4];
		if (lfoSpeed != 0) {
			if (v.whatever2_vibDirToggle == 0) {
				uint16_t sum = (uint16_t)v.something_else[0] + lfoSpeed;
				v.something_else[0] = sum & 0xFF;
				v.something_else[2] = (v.something_else[2] + (sum >> 8)) & 0xFF;
				if (v.something_else[2] >= 0x0E)
					v.whatever2_vibDirToggle++;
			} else {
				uint16_t diff = (uint16_t)v.something_else[0] - lfoSpeed;
				uint8_t borrow = (diff > 0xFF) ? 1 : 0;
				v.something_else[0] = diff & 0xFF;
				v.something_else[2] = (v.something_else[2] - borrow) & 0xFF;
				if (v.something_else[2] < 0x08)
					v.whatever2_vibDirToggle--;
			}
			applyPulseWidth(channel, v);
		}

		if (v.whatever1) {
			if (v.stuff_arp_counter == v.arpSpeedHiNibble)
				v.stuff_arp_counter = 0;

			uint8_t arpNote = v.currentNote;
			if (v.stuff_arp_counter < kDrillerArpeggioSize) {
				uint16_t noteWithOffset = v.currentNote + arpeggio_data[v.stuff_arp_counter];
				arpNote = noteWithOffset >= 96 ? 95 : noteWithOffset;
			}

			writeNoteFrequency(channel, v, arpNote);
			v.stuff_arp_counter++;
			return;
		}

		if (v.whatever0) {
			uint8_t freqLo = v.stuff_freq_porta_vib & 0xFF;
			uint8_t freqHi = (v.stuff_freq_porta_vib >> 8) & 0xFF;

			if (v.things_vib_state == 0 || v.things_vib_state >= 3) {
				uint16_t diff = (uint16_t)freqLo - v.things_vib_depth;
				uint8_t borrow = (diff > 0xFF) ? 1 : 0;
				freqLo = diff & 0xFF;
				freqHi = (freqHi - borrow) & 0xFF;
			} else {
				uint16_t sum = (uint16_t)freqLo + v.things_vib_depth;
				freqLo = sum & 0xFF;
				freqHi = (freqHi + (sum >> 8)) & 0xFF;
			}

			v.stuff_freq_porta_vib = freqLo | (freqHi << 8);
			writeSIDFrequency(channel, v, v.stuff_freq_porta_vib);
			v.things_vib_delay_ctr--;
			if (v.things_vib_delay_ctr == 0) {
				v.things_vib_delay_ctr = v.things_vib_delay_reload;
				v.things_vib_state++;
				if (v.things_vib_state >= 5)
					v.things_vib_state = 1;
			}
			return;
		}
	}

	if (v.whatever2 == 1) {
		uint8_t freqLo = v.stuff_freq_porta_vib & 0xFF;
		uint8_t freqHi = (v.stuff_freq_porta_vib >> 8) & 0xFF;
		uint16_t diff = (uint16_t)freqLo - v.portaStepRaw - 1;
		uint8_t borrow = (diff > 0xFF) ? 1 : 0;
		freqLo = diff & 0xFF;
		freqHi = (freqHi - borrow) & 0xFF;
		v.stuff_freq_porta_vib = freqLo | (freqHi << 8);
		writeSIDFrequency(channel, v, v.stuff_freq_porta_vib);
	} else if (v.whatever2 == 2) {
		uint8_t freqLo = v.stuff_freq_porta_vib & 0xFF;
		uint8_t freqHi = (v.stuff_freq_porta_vib >> 8) & 0xFF;
		uint16_t sum = (uint16_t)freqLo + v.portaStepRaw;
		freqLo = sum & 0xFF;
		freqHi = (freqHi + (sum >> 8)) & 0xFF;
		v.stuff_freq_porta_vib = freqLo | (freqHi << 8);
		writeSIDFrequency(channel, v, v.stuff_freq_porta_vib);
	} else if (v.whatever2 == 3) {
		uint8_t freqLo = v.stuff_freq_porta_vib & 0xFF;
		uint8_t freqHi = ((v.stuff_freq_porta_vib >> 8) - v.portaStepRaw) & 0xFF;
		v.stuff_freq_porta_vib = freqLo | (freqHi << 8);
		writeSIDFrequency(channel, v, v.stuff_freq_porta_vib);
	} else if (v.whatever2 != 0) {
		uint8_t freqLo = v.stuff_freq_porta_vib & 0xFF;
		uint8_t freqHi = ((v.stuff_freq_porta_vib >> 8) + v.portaStepRaw) & 0xFF;
		v.stuff_freq_porta_vib = freqLo | (freqHi << 8);
		writeSIDFrequency(channel, v, v.stuff_freq_porta_vib);
	}

	if (instA0[7] & 0x01)
		applyHardRestart(channel, v, instA1);
}

void DrillerOPLMusicPlayer::applyHardRestart(int channel, VoiceState &v, const uint8_t *instA1) {
	uint8_t storedHi = (v.stuff_freq_base >> 8) & 0xFF;
	if (storedHi != 0) {
		storedHi--;
		v.stuff_freq_base = (v.stuff_freq_base & 0x00FF) | (storedHi << 8);
	}

	if (v.whatever3 != 0) {
		v.whatever3--;
		noteOff(channel);
		return;
	}

	if (v.whatever4 != instA1[5]) {
		v.whatever3++;
		v.whatever4++;
	} else {
		v.whatever4 = 0;
		v.whatever3 = 0;
	}

	v.stuff_freq_base = (v.stuff_freq_base & 0x00FF) | ((v.stuff_freq_porta_vib >> 8) << 8);
	if (!v.keyOn && v.currentNote != 0) {
		v.keyOn = true;
		v.gateReleased = false;
		writeSIDFrequency(channel, v, v.stuff_freq_porta_vib);
	}
}

void DrillerOPLMusicPlayer::setOPLInstrument(int channel, VoiceState &v) {
	if (!_opl)
		return;

	const DrillerOPLBasePatch &patch = kDrillerOPLBasePatches[getDrillerWaveformFamily(v.currentControl)];
	byte mod = kOPLModOffset[channel];
	byte car = kOPLCarOffset[channel];

	// Give the carrier the instrument's own envelope (from its SID AD/SR) so the
	// 22 instruments stay distinct and sustained voices hold instead of fading.
	int instBase = v.instrumentIndex;
	if (instBase < 0 || instBase >= NUM_INSTRUMENTS * 8)
		instBase = 0;
	byte carAD, carSR;
	bool sustaining;
	deriveDrillerOPLEnvelope(instrumentDataA0[instBase + 2], instrumentDataA0[instBase + 3], carAD, carSR, sustaining);
	byte carChar = (patch.carChar & ~0x20) | (sustaining ? 0x20 : 0x00);

	_opl->writeReg(0x20 + mod, patch.modChar);
	_opl->writeReg(0x20 + car, carChar);
	_opl->writeReg(0x60 + mod, patch.modAD);
	_opl->writeReg(0x60 + car, carAD);
	_opl->writeReg(0x80 + mod, patch.modSR);
	_opl->writeReg(0x80 + car, carSR);
	_opl->writeReg(0xE0 + mod, patch.modWave);
	_opl->writeReg(0xE0 + car, patch.carWave);
	_opl->writeReg(0xC0 + channel, patch.feedbackConnection);
	applyPulseWidth(channel, v);
}

void DrillerOPLMusicPlayer::applyPulseWidth(int channel, const VoiceState &v) {
	if (!_opl)
		return;

	const DrillerOPLBasePatch &patch = kDrillerOPLBasePatches[getDrillerWaveformFamily(v.currentControl)];
	byte modLevel = patch.modLevel;
	byte carLevel = patch.carLevel;
	byte feedbackConnection = patch.feedbackConnection;
	byte attenuation = getDrillerOPLAttenuation(v.currentControl);

	if (v.currentControl & 0x40) {
		uint16 pulseWidth = (v.something_else[0] | (v.something_else[2] << 8)) & 0x0FFF;
		uint16 edgeDistance = MIN<uint16>(pulseWidth, 0x1000 - pulseWidth);
		uint16 centerDistance = pulseWidth < 0x0800 ? 0x0800 - pulseWidth : pulseWidth - 0x0800;
		byte brightnessBoost = MIN<byte>(centerDistance >> 5, kDrillerPulseBrightnessBoostMax);

		modLevel = patch.modLevel > brightnessBoost ? patch.modLevel - brightnessBoost : 0;

		byte feedback = (patch.feedbackConnection >> 1) & 0x07;
		// Only a little feedback; near the max (7) the FM tone turns to noise.
		feedback = MIN<byte>(4, feedback + (centerDistance >> 9));
		feedbackConnection = (patch.feedbackConnection & 0x01) | (feedback << 1);
		if (edgeDistance <= kDrillerNarrowPulseEdgeDistance)
			attenuation = kDrillerNarrowPulseAttenuation;
	}

	byte mod = kOPLModOffset[channel];
	byte car = kOPLCarOffset[channel];
	_opl->writeReg(0xC0 + channel, feedbackConnection);
	_opl->writeReg(0x40 + mod, attenuateDrillerOPLLevel(modLevel, attenuation));
	_opl->writeReg(0x40 + car, attenuateDrillerOPLLevel(carLevel, attenuation));
}

void DrillerOPLMusicPlayer::noteOn(int channel, VoiceState &v, uint8_t note) {
	if (!_opl)
		return;

	v.keyOn = false;
	writeFrequency(channel, v, v.frequencyFnum, v.frequencyBlock);

	noteToFnumBlock(note, v.baseFrequencyFnum, v.baseFrequencyBlock);
	v.baseSIDFrequency = getDrillerSIDFrequency(note);
	v.keyOn = true;
	v.gateReleased = false;
	setFrequency(channel, v, v.baseFrequencyFnum, v.baseFrequencyBlock);
}

void DrillerOPLMusicPlayer::noteOff(int channel) {
	if (!_opl)
		return;

	VoiceState &v = _voiceState[channel];
	v.keyOn = false;
	v.gateReleased = true;
	writeFrequency(channel, v, v.frequencyFnum, v.frequencyBlock);
}

void DrillerOPLMusicPlayer::setFrequency(int channel, VoiceState &v, uint16 fnum, byte block) {
	v.frequencyFnum = fnum;
	v.frequencyBlock = block;
	writeFrequency(channel, v, fnum, block);
}

void DrillerOPLMusicPlayer::writeFrequency(int channel, const VoiceState &v, uint16 fnum, byte block) {
	if (!_opl)
		return;

	_opl->writeReg(0xA0 + channel, fnum & 0xFF);
	byte b0 = ((fnum >> 8) & 0x03) | (block << 2);
	if (v.keyOn)
		b0 |= 0x20;
	_opl->writeReg(0xB0 + channel, b0);
}

void DrillerOPLMusicPlayer::writeNoteFrequency(int channel, VoiceState &v, uint8_t note) {
	uint16 fnum = 0;
	byte block = 0;
	noteToFnumBlock(note, fnum, block);
	setFrequency(channel, v, fnum, block);
}

void DrillerOPLMusicPlayer::writeSIDFrequency(int channel, VoiceState &v, uint16 sidFrequency) {
	if (v.baseSIDFrequency == 0 || v.baseFrequencyFnum == 0)
		return;

	uint32 scaledFnum = ((uint32)v.baseFrequencyFnum * sidFrequency + (v.baseSIDFrequency / 2)) / v.baseSIDFrequency;
	byte block = v.baseFrequencyBlock;
	while (scaledFnum > 0x3FF && block < 7) {
		scaledFnum = (scaledFnum + 1) >> 1;
		block++;
	}

	setFrequency(channel, v, MIN<uint32>(scaledFnum, 0x3FF), block);
}

void DrillerOPLMusicPlayer::noteToFnumBlock(int note, uint16 &fnum, byte &block) const {
	note = CLIP<int>(note, 0, MIN<int>(kMaxNote, ARRAYSIZE(kDrillerOPLFreqs) - 1));
	uint16 combined = kDrillerOPLFreqs[note];
	fnum = combined & 0x03FF;
	block = (combined >> 10) & 0x07;
}

} // namespace Freescape
