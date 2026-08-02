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

#include "engines/freescape/games/eclipse/opl.music.h"

#include "common/debug.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "freescape/freescape.h"
#include "freescape/wb.h"
#include "freescape/games/eclipse/eclipse.musicdata.h"

using namespace Freescape::EclipseMusicData;

namespace Freescape {

struct EclipseOPLBasePatch {
	byte modChar;            // reg 0x20: AM | VIB | EGT | KSR | MULT
	byte carChar;
	byte modLevel;           // reg 0x40: modulator total level == FM index
	byte carLevel;           // reg 0x40: carrier total level == output level
	byte modWave;            // reg 0xE0: 0 sine, 1 half-sine, 2 abs-sine, 3 pulse-sine
	byte carWave;
	byte feedbackConnection; // reg 0xC0: feedback | connection
};

// ============================================================================
// Embedded music data (shared with AY player, extracted from C64)
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

// Instruments, order lists, pattern data, and arpeggio intervals
// are in eclipse.musicdata.h (shared with the AY player).

// ============================================================================
// (pattern data removed - now in eclipse.musicdata.h)
//
// ============================================================================
// OPL2 FM base patches (our own creation, but driven by the original SID data)
// ============================================================================

// OPL operator register offsets for channels 0-2
// Each OPL channel has 2 operators (modulator + carrier)
// Modulator offsets: 0x00, 0x01, 0x02 for channels 0-2
// Carrier offsets:   0x03, 0x04, 0x05 for channels 0-2
const byte kOPLModOffset[] = { 0x00, 0x01, 0x02 };
const byte kOPLCarOffset[] = { 0x03, 0x04, 0x05 };

// Every operator sets EGT (0x20) so the envelope holds at the sustain level
// until key-off, like the SID gate bit.
const EclipseOPLBasePatch kOPLBasePatches[] = {
	// 0: silent
	{ 0x20, 0x20, 0x3F, 0x3F, 0x00, 0x00, 0x00 },
	// 1: triangle - 2:1 FM at a low index, the weak odd harmonics of a SID triangle
	{ 0x22, 0x21, 0x22, 0x00, 0x00, 0x00, 0x00 },
	// 2: sawtooth - abs-sine modulator plus feedback folds towards a ramp
	{ 0x21, 0x21, 0x18, 0x00, 0x02, 0x00, 0x0C },
	// 3: pulse - half-sine carrier at 2:1, index swept by the SID pulse width
	{ 0x22, 0x21, 0x18, 0x00, 0x00, 0x01, 0x06 },
	// 4: noise - single operator, played on the rhythm-mode hi-hat
	{ 0x2E, 0x20, 0x08, 0x3F, 0x00, 0x00, 0x00 },
};

const byte kNoiseWaveformFamily = 4;

// OPL2 rhythm mode: bit 5 of register 0xBD turns channels 6-8 into percussion
// voices, the only source of real noise on an OPL2. Melodic playback only ever
// uses channels 0-2, so they are free.
const byte kRhythmEnable   = 0x20;
const byte kRhythmHiHatBit = 0x01;
const byte kRhythmHiHatOp  = 0x11; // channel 7 modulator operator offset
const byte kRhythmChannel  = 7;

// SID envelope nibbles to OPL2 envelope rates, matched in log space against
// 0.22 * 2^(14-AR) ms for attack and 1.27 * 2^(15-rate) ms for decay/release.
// SID's slowest rates are beyond what the OPL can reach and clamp to 1.
const byte kSIDAttackToOPL[16] = {
	11, 9, 8, 7, 7, 6, 6, 5, 5, 4, 3, 2, 2, 1, 1, 1
};

const byte kSIDDecayToOPL[16] = {
	13, 11, 10, 9, 9, 8, 8, 7, 7, 6, 5, 4, 4, 2, 1, 1
};

// SID sustain is a linear fraction S/15, OPL sustain level is attenuation in
// 3 dB steps. Nibble 0 means silence.
const byte kSIDSustainToOPL[16] = {
	15, 8, 6, 5, 4, 3, 3, 2, 2, 1, 1, 1, 1, 0, 0, 0
};

const char *const kWaveformFamilyName[] = {
	"silent", "triangle", "sawtooth", "pulse", "noise"
};

int oplFrequencyHz(uint16 fnum, byte block) {
	return (int)(((uint32)fnum * 49716) >> (20 - block));
}

byte getWaveformFamily(byte ctrl) {
	if ((ctrl & 0x80) != 0)
		return 4;
	if ((ctrl & 0x40) != 0)
		return 3;
	if ((ctrl & 0x20) != 0)
		return 2;
	if ((ctrl & 0x10) != 0)
		return 1;
	return 0;
}

uint16 decodePulseWidth(byte packed) {
	return ((packed & 0x0F) << 8) | (packed & 0xF0);
}

// ============================================================================
// ChannelState
// ============================================================================

void EclipseOPLMusicPlayer::ChannelState::reset() {
	orderList = nullptr;
	orderPos = 0;
	patternDataOffset = 0;
	patternOffset = 0;
	instrumentOffset = 0;
	currentNote = 0;
	transpose = 0;
	frequencyFnum = 0;
	frequencyBlock = 0;
	durationReload = 0;
	durationCounter = 0;
	effectMode = 0;
	effectParam = 0;
	arpeggioTarget = 0;
	arpeggioParam = 0;
	arpeggioSequencePos = 0;
	memset(arpeggioSequence, 0, sizeof(arpeggioSequence));
	arpeggioSequenceLen = 0;
	noteStepCommand = 0;
	stepDownCounter = 0;
	vibratoPhase = 0;
	vibratoCounter = 0;
	delayValue = 0;
	delayCounter = 0;
	waveform = 0;
	instrumentFlags = 0;
	gateOffDisabled = false;
	keyOn = false;
	pulseWidth = 0;
	pulseWidthMod = 0;
	pulseWidthDirection = 0;
	modBaseLevel = 0x3F;
	carBaseLevel = 0x3F;
	modLevel = 0x3F;
	carLevel = 0x3F;
	rhythmVoice = false;
}

// ============================================================================
// Constructor / Destructor
// ============================================================================

EclipseOPLMusicPlayer::EclipseOPLMusicPlayer()
	: _opl(nullptr),
	  _musicActive(false),
	  _speedDivider(1),
	  _speedCounter(0),
	  _rhythmReg(kRhythmEnable) {
	memcpy(_arpeggioIntervals, kArpeggioIntervals, 8);

	_opl = OPL::Config::create();
	if (!_opl || !_opl->init()) {
		warning("EclipseOPLMusicPlayer: Failed to create OPL emulator");
		delete _opl;
		_opl = nullptr;
	}
}

EclipseOPLMusicPlayer::~EclipseOPLMusicPlayer() {
	stopMusic();
	delete _opl;
}

// ============================================================================
// Public interface
// ============================================================================

void EclipseOPLMusicPlayer::startMusic() {
	if (!_opl)
		return;
	stopMusic();
	_opl->start(new Common::Functor0Mem<void, EclipseOPLMusicPlayer>(
		this, &EclipseOPLMusicPlayer::onTimer), 50);
	setupSong();
}

void EclipseOPLMusicPlayer::stopMusic() {
	_musicActive = false;
	if (_opl) {
		silenceAll();
		_opl->stop();
	}
}

bool EclipseOPLMusicPlayer::isPlaying() const {
	return _musicActive;
}

// ============================================================================
// OPL register helpers
// ============================================================================

void EclipseOPLMusicPlayer::noteToFnumBlock(byte note, uint16 &fnum, byte &block) const {
	if (note > kMaxNote)
		note = kMaxNote;
	uint16 combined = kOPLFreqs[note];
	fnum = combined & 0x03FF;
	block = (combined >> 10) & 0x07;
}

void EclipseOPLMusicPlayer::setFrequency(int channel, uint16 fnum, byte block) {
	_channels[channel].frequencyFnum = fnum;
	_channels[channel].frequencyBlock = block;
	writeFrequency(channel, fnum, block);
}

void EclipseOPLMusicPlayer::writeFrequency(int channel, uint16 fnum, byte block) {
	if (!_opl)
		return;

	// A rhythm voice is pitched from the percussion channel and must never
	// have the key-on bit set.
	if (_channels[channel].rhythmVoice) {
		_opl->writeReg(0xA0 + kRhythmChannel, fnum & 0xFF);
		_opl->writeReg(0xB0 + kRhythmChannel, ((fnum >> 8) & 0x03) | (block << 2));
		return;
	}

	_opl->writeReg(0xA0 + channel, fnum & 0xFF);
	// Preserve key-on bit in 0xB0
	byte b0 = ((fnum >> 8) & 0x03) | (block << 2);
	if (_channels[channel].keyOn)
		b0 |= 0x20;
	_opl->writeReg(0xB0 + channel, b0);
}

void EclipseOPLMusicPlayer::programEnvelope(byte op, byte attack, byte decay, byte sustain, byte release) {
	_opl->writeReg(0x60 + op, (attack << 4) | decay);
	_opl->writeReg(0x80 + op, (sustain << 4) | release);
}

void EclipseOPLMusicPlayer::setOPLInstrument(int channel, byte instrumentOffset) {
	if (!_opl)
		return;
	byte patchIdx = instrumentOffset / kInstrumentSize;
	if (patchIdx >= kInstrumentCount)
		patchIdx = 0;

	byte ctrl = kInstruments[instrumentOffset + 0];
	byte attackDecay = kInstruments[instrumentOffset + 1];
	byte sustainRelease = kInstruments[instrumentOffset + 2];
	byte family = getWaveformFamily(ctrl);
	const EclipseOPLBasePatch &patch = kOPLBasePatches[family];

	byte attack = kSIDAttackToOPL[attackDecay >> 4];
	byte decay = kSIDDecayToOPL[attackDecay & 0x0F];
	byte sustain = kSIDSustainToOPL[sustainRelease >> 4];
	byte release = kSIDDecayToOPL[sustainRelease & 0x0F];

	bool wasRhythm = _channels[channel].rhythmVoice;
	_channels[channel].rhythmVoice = (family == kNoiseWaveformFamily);

	debugC(2, kFreescapeDebugMedia,
		"TE-AdLib: ch%d patch inst=%-2d %-8s SID ad=$%02X sr=$%02X -> OPL ar=%2d dr=%2d sl=%2d rr=%2d"
		" | %s fb=%d modTL=%d carTL=%d",
		channel, patchIdx, kWaveformFamilyName[family], attackDecay, sustainRelease,
		attack, decay, sustain, release,
		(patch.feedbackConnection & 0x01) ? "additive" : "FM",
		(patch.feedbackConnection >> 1) & 0x07, patch.modLevel, patch.carLevel);

	_channels[channel].pulseWidth = decodePulseWidth(kPulseWidthInit[patchIdx]);
	_channels[channel].pulseWidthMod = kPulseWidthMod[patchIdx];
	_channels[channel].pulseWidthDirection = 0;
	_channels[channel].modBaseLevel = patch.modLevel;
	_channels[channel].carBaseLevel = patch.carLevel;
	_channels[channel].modLevel = patch.modLevel;
	_channels[channel].carLevel = patch.carLevel;

	if (_channels[channel].rhythmVoice) {
		// Silence the melodic channel this voice would otherwise have used.
		_channels[channel].keyOn = false;
		_opl->writeReg(0xB0 + channel, 0x00);
		_opl->writeReg(0x40 + kOPLModOffset[channel], 0x3F);
		_opl->writeReg(0x40 + kOPLCarOffset[channel], 0x3F);

		_opl->writeReg(0x20 + kRhythmHiHatOp, patch.modChar);
		_opl->writeReg(0xE0 + kRhythmHiHatOp, patch.modWave);
		programEnvelope(kRhythmHiHatOp, attack, decay, sustain, release);
		updatePulseWidth(channel, false);
		applyOperatorLevels(channel);
		return;
	}

	if (wasRhythm) {
		_rhythmReg &= ~kRhythmHiHatBit;
		_opl->writeReg(0xBD, _rhythmReg);
	}

	byte mod = kOPLModOffset[channel];
	byte car = kOPLCarOffset[channel];

	_opl->writeReg(0x20 + mod, patch.modChar);
	_opl->writeReg(0x20 + car, patch.carChar);

	// The FM index follows the modulator's absolute output, and a SID
	// oscillator keeps its waveform as the note decays, so hold the modulator
	// flat and let only the carrier follow the ADSR. Enveloping it too
	// collapses the index within milliseconds, leaving a bass note as a
	// near-pure sine that is inaudible at 70 Hz. Additive patches are
	// different: both operators are heard, so both take the real envelope.
	if ((patch.feedbackConnection & 0x01) != 0)
		programEnvelope(mod, attack, decay, sustain, release);
	else
		programEnvelope(mod, 15, 0, 0, 0);
	programEnvelope(car, attack, decay, sustain, release);
	_opl->writeReg(0xE0 + mod, patch.modWave);
	_opl->writeReg(0xE0 + car, patch.carWave);
	_opl->writeReg(0xC0 + channel, patch.feedbackConnection);

	updatePulseWidth(channel, false);
	applyOperatorLevels(channel);
}

void EclipseOPLMusicPlayer::noteOn(int channel) {
	if (!_opl)
		return;
	_channels[channel].keyOn = true;

	if (_channels[channel].rhythmVoice) {
		// Percussion voices are keyed from register 0xBD, not from 0xB0.
		_opl->writeReg(0xBD, _rhythmReg & ~kRhythmHiHatBit);
		_rhythmReg |= kRhythmHiHatBit;
		_opl->writeReg(0xBD, _rhythmReg);
		return;
	}

	_opl->writeReg(0xA0 + channel, _channels[channel].frequencyFnum & 0xFF);
	_opl->writeReg(0xB0 + channel, 0x20 | (_channels[channel].frequencyBlock << 2) |
	                                 ((_channels[channel].frequencyFnum >> 8) & 0x03));
}

void EclipseOPLMusicPlayer::noteOff(int channel) {
	if (!_opl)
		return;
	_channels[channel].keyOn = false;

	if (_channels[channel].rhythmVoice) {
		_rhythmReg &= ~kRhythmHiHatBit;
		_opl->writeReg(0xBD, _rhythmReg);
		return;
	}

	byte b0 = ((_channels[channel].frequencyFnum >> 8) & 0x03) |
	          (_channels[channel].frequencyBlock << 2);
	_opl->writeReg(0xB0 + channel, b0);
}

// ============================================================================
// Timer / sequencer core
// ============================================================================

void EclipseOPLMusicPlayer::onTimer() {
	if (!_musicActive)
		return;

	bool newBeat = (_speedCounter == 0);

	for (int channel = kChannelCount - 1; channel >= 0; channel--)
		processChannel(channel, newBeat);

	if (!_musicActive)
		return;

	if (newBeat)
		_speedCounter = _speedDivider;
	else
		_speedCounter--;
}

void EclipseOPLMusicPlayer::processChannel(int channel, bool newBeat) {
	if (newBeat) {
		_channels[channel].durationCounter--;
		if (_channels[channel].durationCounter == 0xFF) {
			parseCommands(channel);
			if (!_musicActive)
				return;
			finalizeChannel(channel);
			return;
		}

		if (_channels[channel].noteStepCommand != 0) {
			if (_channels[channel].noteStepCommand == 0xDE) {
				if (_channels[channel].currentNote > 0)
					_channels[channel].currentNote--;
			} else if (_channels[channel].currentNote < kMaxNote) {
				_channels[channel].currentNote++;
			}
			loadCurrentFrequency(channel);
			finalizeChannel(channel);
			return;
		}
	} else if (_channels[channel].stepDownCounter != 0) {
		_channels[channel].stepDownCounter--;
		if (_channels[channel].currentNote > 0)
			_channels[channel].currentNote--;
		loadCurrentFrequency(channel);
		finalizeChannel(channel);
		return;
	}

	applyFrameEffects(channel);
	finalizeChannel(channel);
}

void EclipseOPLMusicPlayer::finalizeChannel(int channel) {
	// Mirrors the SID engine clearing the gate bit halfway through the note.
	if (_channels[channel].durationReload != 0 &&
	    !_channels[channel].gateOffDisabled &&
	    ((_channels[channel].durationReload >> 1) == _channels[channel].durationCounter) &&
	    _channels[channel].keyOn) {
		noteOff(channel);
		debugC(3, kFreescapeDebugMedia, "TE-AdLib: ch%d gate off (note %d releasing)",
			channel, _channels[channel].currentNote);
	}

	updatePulseWidth(channel, true);
	applyOperatorLevels(channel);
}

// ============================================================================
// Song setup
// ============================================================================

void EclipseOPLMusicPlayer::setupSong() {
	silenceAll();

	// Enable wave select (required for non-sine waveforms)
	if (_opl)
		_opl->writeReg(0x01, 0x20);

	_speedDivider = 1;
	_speedCounter = 0;

	const byte *orderLists[3] = { kOrderList0, kOrderList1, kOrderList2 };

	for (int i = 0; i < kChannelCount; i++) {
		_channels[i].reset();
		_channels[i].orderList = orderLists[i];
		loadNextPattern(i);
	}

	_musicActive = true;
}

void EclipseOPLMusicPlayer::silenceAll() {
	if (!_opl)
		return;
	for (int ch = 0; ch < kChannelCount; ch++) {
		_channels[ch].keyOn = false;
		_opl->writeReg(0xB0 + ch, 0x00); // key off
		_opl->writeReg(0x40 + kOPLModOffset[ch], 0x3F); // silence mod
		_opl->writeReg(0x40 + kOPLCarOffset[ch], 0x3F); // silence car
	}

	// Leave rhythm mode armed with every trigger released.
	_rhythmReg = kRhythmEnable;
	_opl->writeReg(0xBD, _rhythmReg);
	_opl->writeReg(0x40 + kRhythmHiHatOp, 0x3F);
}

// ============================================================================
// Order list / pattern navigation
// ============================================================================

void EclipseOPLMusicPlayer::loadNextPattern(int channel) {
	int safety = 200;
	while (safety-- > 0) {
		byte value = _channels[channel].orderList[_channels[channel].orderPos];
		_channels[channel].orderPos++;

		if (value == 0xFF) {
			_channels[channel].orderPos = 0;
			continue;
		}

		if (value >= 0xC0) {
			_channels[channel].transpose = (byte)WBCommon::decodeOrderTranspose(value);
			continue;
		}

		if (value < ARRAYSIZE(kPatternOffsets)) {
			_channels[channel].patternDataOffset = kPatternOffsets[value];
			_channels[channel].patternOffset = 0;
			debugC(3, kFreescapeDebugMedia, "TE-AdLib: ch%d order %d -> pattern %d (transpose %d)",
				channel, _channels[channel].orderPos - 1, value, (int8)_channels[channel].transpose);
		}
		break;
	}
}

byte EclipseOPLMusicPlayer::readPatternByte(int channel) {
	byte value = kPatternData[_channels[channel].patternDataOffset + _channels[channel].patternOffset];
	_channels[channel].patternOffset++;
	return value;
}

byte EclipseOPLMusicPlayer::clampNote(byte note) const {
	return note > kMaxNote ? static_cast<byte>(kMaxNote) : note;
}

// ============================================================================
// Pattern command parser
// ============================================================================

void EclipseOPLMusicPlayer::parseCommands(int channel) {
	if (_channels[channel].effectMode != 2) {
		_channels[channel].effectParam = 0;
		_channels[channel].effectMode = 0;
		_channels[channel].arpeggioSequenceLen = 0;
		_channels[channel].arpeggioSequencePos = 0;
	}

	_channels[channel].arpeggioTarget = 0;
	_channels[channel].noteStepCommand = 0;

	int safety = 200;
	while (safety-- > 0) {
		byte cmd = readPatternByte(channel);

		if (cmd == 0xFF) {
			loadNextPattern(channel);
			continue;
		}

		if (cmd == 0xFE) {
			stopMusic();
			return;
		}

		if (cmd == 0xFD) {
			readPatternByte(channel);
			cmd = readPatternByte(channel);
			if (cmd == 0xFF) {
				loadNextPattern(channel);
				continue;
			}
		}

		if (cmd >= 0xF0) {
			_speedDivider = cmd & 0x0F;
			debugC(2, kFreescapeDebugMedia, "TE-AdLib: ch%d speed $%02X -> %d ticks per beat",
				channel, cmd, _speedDivider + 1);
			continue;
		}

		if (cmd >= 0xC0) {
			byte instrument = cmd & 0x1F;
			if (instrument < kInstrumentCount)
				_channels[channel].instrumentOffset = instrument * kInstrumentSize;
			continue;
		}

		if (cmd >= 0x80) {
			_channels[channel].durationReload = cmd & 0x3F;
			continue;
		}

		if (cmd == 0x7F) {
			_channels[channel].noteStepCommand = 0xDE;
			_channels[channel].effectMode = 0xDE;
			continue;
		}

		if (cmd == 0x7E) {
			// Portamento down. noteStepCommand stands in for the INC/DEC
			// opcode the C64 engine self-modifies; without it the direction is
			// set but the note never steps.
			_channels[channel].noteStepCommand = 0xFE;
			_channels[channel].effectMode = 0xFE;
			continue;
		}

		if (cmd == 0x7D) {
			_channels[channel].effectMode = 1;
			_channels[channel].effectParam = readPatternByte(channel);
			buildEffectArpeggio(channel);
			continue;
		}

		if (cmd == 0x7C) {
			_channels[channel].effectMode = 2;
			_channels[channel].effectParam = readPatternByte(channel);
			buildEffectArpeggio(channel);
			continue;
		}

		if (cmd == 0x7B) {
			_channels[channel].effectParam = 0;
			_channels[channel].effectMode = 1;
			_channels[channel].arpeggioTarget = readPatternByte(channel) + _channels[channel].transpose;
			_channels[channel].arpeggioParam = readPatternByte(channel);
			continue;
		}

		if (cmd == 0x7A) {
			_channels[channel].delayValue = readPatternByte(channel);
			cmd = readPatternByte(channel);
		}

		applyNote(channel, cmd);
		return;
	}
}

// ============================================================================
// Note application
// ============================================================================

void EclipseOPLMusicPlayer::applyNote(int channel, byte note) {
	byte instrumentOffset = _channels[channel].instrumentOffset;
	byte ctrl = kInstruments[instrumentOffset + 0];
	byte sustainRelease = kInstruments[instrumentOffset + 2];
	byte autoEffect = kInstruments[instrumentOffset + 4];
	byte flags = kInstruments[instrumentOffset + 5];
	byte actualNote = note;
	bool gateEnabled = (ctrl & 0x01) != 0;

	if (actualNote != 0)
		actualNote = clampNote(actualNote + _channels[channel].transpose);

	_channels[channel].currentNote = actualNote;
	_channels[channel].waveform = ctrl;
	_channels[channel].instrumentFlags = flags;
	_channels[channel].stepDownCounter = 0;

	if (actualNote != 0 && _channels[channel].effectParam == 0 && autoEffect != 0) {
		_channels[channel].effectParam = autoEffect;
		buildEffectArpeggio(channel);
	}

	if (actualNote != 0 && (flags & 0x02) != 0) {
		_channels[channel].stepDownCounter = 2;
		_channels[channel].currentNote = clampNote(_channels[channel].currentNote + 2);
	}

	setOPLInstrument(channel, instrumentOffset);

	_channels[channel].gateOffDisabled = (sustainRelease & 0x0F) == 0x0F;

	if (actualNote != 0)
		loadCurrentFrequency(channel);

	byte instrument = instrumentOffset / kInstrumentSize;
	byte family = getWaveformFamily(ctrl);

	if (actualNote == 0 || !gateEnabled) {
		noteOff(channel);
		debugC(1, kFreescapeDebugMedia, "TE-AdLib: ch%d rest  inst=%-2d %-8s dur=%d",
			channel, instrument, kWaveformFamilyName[family],
			_channels[channel].durationReload);
	} else {
		// Key-off then key-on restarts the envelope from its current level,
		// like re-gating a held SID voice.
		noteOff(channel);
		noteOn(channel);
		debugC(1, kFreescapeDebugMedia,
			"TE-AdLib: ch%d NOTE %3d (%d%+d) %5dHz inst=%-2d %-8s dur=%d%s%s",
			channel, _channels[channel].currentNote, note, (int8)_channels[channel].transpose,
			oplFrequencyHz(_channels[channel].frequencyFnum, _channels[channel].frequencyBlock),
			instrument, kWaveformFamilyName[family], _channels[channel].durationReload,
			_channels[channel].rhythmVoice ? " [rhythm hi-hat]" : "",
			_channels[channel].effectParam ? " [arpeggio]" : "");
	}

	_channels[channel].durationCounter = _channels[channel].durationReload;
	_channels[channel].delayCounter = _channels[channel].delayValue;
	_channels[channel].arpeggioSequencePos = 0;
}

// ============================================================================
// Frequency helpers
// ============================================================================

void EclipseOPLMusicPlayer::loadCurrentFrequency(int channel) {
	byte note = clampNote(_channels[channel].currentNote);
	uint16 fnum;
	byte block;
	noteToFnumBlock(note, fnum, block);
	setFrequency(channel, fnum, block);
}

// ============================================================================
// Effects
// ============================================================================

void EclipseOPLMusicPlayer::buildEffectArpeggio(int channel) {
	_channels[channel].arpeggioSequenceLen = WBCommon::buildArpeggioTable(
		_arpeggioIntervals,
		_channels[channel].effectParam,
		_channels[channel].arpeggioSequence,
		sizeof(_channels[channel].arpeggioSequence),
		true);
	_channels[channel].arpeggioSequencePos = 0;
}

void EclipseOPLMusicPlayer::applyFrameEffects(int channel) {
	if (_channels[channel].currentNote == 0)
		return;

	if (applyInstrumentVibrato(channel))
		return;

	applyEffectArpeggio(channel);
	applyTimedSlide(channel);
}

bool EclipseOPLMusicPlayer::applyInstrumentVibrato(int channel) {
	byte vibrato = kInstruments[_channels[channel].instrumentOffset + 3];
	if (vibrato == 0 || _channels[channel].currentNote >= kMaxNote)
		return false;

	byte shift = vibrato & 0x0F;
	byte span = vibrato >> 4;
	if (span == 0)
		return false;

	uint16 noteFnum, nextFnum;
	byte noteBlock, nextBlock;
	noteToFnumBlock(_channels[channel].currentNote, noteFnum, noteBlock);
	noteToFnumBlock(_channels[channel].currentNote + 1, nextFnum, nextBlock);

	// Normalize to same block for delta computation
	int32 noteFreq = noteFnum << noteBlock;
	int32 nextFreq = nextFnum << nextBlock;
	int32 delta = nextFreq - noteFreq;
	if (delta <= 0)
		return false;

	while (shift-- != 0)
		delta >>= 1;

	if ((_channels[channel].vibratoPhase & 0x80) != 0) {
		if (_channels[channel].vibratoCounter != 0)
			_channels[channel].vibratoCounter--;
		if (_channels[channel].vibratoCounter == 0)
			_channels[channel].vibratoPhase = 0;
	} else {
		_channels[channel].vibratoCounter++;
		if (_channels[channel].vibratoCounter >= span)
			_channels[channel].vibratoPhase = 0xFF;
	}

	if (_channels[channel].delayCounter != 0) {
		_channels[channel].delayCounter--;
		return false;
	}

	int32 freq = (int32)_channels[channel].frequencyFnum << _channels[channel].frequencyBlock;
	for (byte i = 0; i < (span >> 1); i++)
		freq -= delta;
	for (byte i = 0; i < _channels[channel].vibratoCounter; i++)
		freq += delta;

	if (freq < 1) freq = 1;

	// Convert back to fnum/block
	byte block = 0;
	while (freq > 1023 && block < 7) {
		freq >>= 1;
		block++;
	}
	writeFrequency(channel, freq & 0x3FF, block);
	return true;
}

void EclipseOPLMusicPlayer::applyEffectArpeggio(int channel) {
	if (_channels[channel].effectParam == 0 || _channels[channel].arpeggioSequenceLen == 0)
		return;

	if (_channels[channel].arpeggioSequencePos >= _channels[channel].arpeggioSequenceLen)
		_channels[channel].arpeggioSequencePos = 0;

	byte note = clampNote(_channels[channel].currentNote +
	                      _channels[channel].arpeggioSequence[_channels[channel].arpeggioSequencePos]);
	_channels[channel].arpeggioSequencePos++;

	uint16 fnum;
	byte block;
	noteToFnumBlock(note, fnum, block);
	writeFrequency(channel, fnum, block);
}

void EclipseOPLMusicPlayer::applyTimedSlide(int channel) {
	if (_channels[channel].arpeggioTarget == 0)
		return;

	byte total = _channels[channel].durationReload;
	byte remaining = _channels[channel].durationCounter;
	byte start = _channels[channel].arpeggioParam >> 4;
	byte span = _channels[channel].arpeggioParam & 0x0F;
	byte elapsed = total - remaining;

	if (elapsed <= start || elapsed > start + span || span == 0)
		return;

	byte currentNote = clampNote(_channels[channel].currentNote);
	byte targetNote = clampNote(_channels[channel].arpeggioTarget);
	if (currentNote == targetNote)
		return;

	uint16 srcFnum, tgtFnum;
	byte srcBlock, tgtBlock;
	noteToFnumBlock(currentNote, srcFnum, srcBlock);
	noteToFnumBlock(targetNote, tgtFnum, tgtBlock);

	int32 srcFreq = srcFnum << srcBlock;
	int32 tgtFreq = tgtFnum << tgtBlock;
	int32 difference = srcFreq > tgtFreq ? srcFreq - tgtFreq : tgtFreq - srcFreq;
	uint16 divisor = span * (_speedDivider + 1);
	if (divisor == 0)
		return;

	int32 delta = difference / divisor;
	if (delta == 0)
		return;

	int32 curFreq = (int32)_channels[channel].frequencyFnum << _channels[channel].frequencyBlock;

	if (tgtFreq > srcFreq)
		curFreq += delta;
	else
		curFreq -= delta;

	if (curFreq < 1) curFreq = 1;

	byte block = 0;
	while (curFreq > 1023 && block < 7) {
		curFreq >>= 1;
		block++;
	}
	setFrequency(channel, curFreq & 0x3FF, block);
}

void EclipseOPLMusicPlayer::updatePulseWidth(int channel, bool advance) {
	if ((_channels[channel].waveform & 0x40) == 0) {
		_channels[channel].modLevel = _channels[channel].modBaseLevel;
		_channels[channel].carLevel = _channels[channel].carBaseLevel;
		return;
	}

	if (advance && _channels[channel].pulseWidthMod != 0) {
		if ((_channels[channel].instrumentFlags & 0x04) != 0) {
			uint16 pulseWidth = _channels[channel].pulseWidth;
			pulseWidth = (pulseWidth & 0x0F00) |
			             (((pulseWidth & 0x00FF) + _channels[channel].pulseWidthMod) & 0x00FF);
			_channels[channel].pulseWidth = pulseWidth;
		} else if (_channels[channel].pulseWidthDirection == 0) {
			_channels[channel].pulseWidth += _channels[channel].pulseWidthMod;
			if ((_channels[channel].pulseWidth >> 8) >= 0x0F)
				_channels[channel].pulseWidthDirection = 1;
		} else {
			_channels[channel].pulseWidth -= _channels[channel].pulseWidthMod;
			if ((_channels[channel].pulseWidth >> 8) < 0x08)
				_channels[channel].pulseWidthDirection = 0;
		}
	}

	uint16 pulseWidth = MIN<uint16>(_channels[channel].pulseWidth & 0x0FFF, 0x0800);
	byte brightnessBoost = pulseWidth < 0x0800 ? (0x0800 - pulseWidth) >> 7 : 0;
	if (brightnessBoost > 12)
		brightnessBoost = 12;

	_channels[channel].modLevel = (_channels[channel].modBaseLevel > brightnessBoost) ?
	                              (_channels[channel].modBaseLevel - brightnessBoost) : 0;
	_channels[channel].carLevel = _channels[channel].carBaseLevel;
}

// The envelope lives in the chip, so the total-level registers only carry the
// patch levels plus the pulse-width brightness motion.
void EclipseOPLMusicPlayer::applyOperatorLevels(int channel) {
	if (!_opl)
		return;

	if (_channels[channel].rhythmVoice) {
		_opl->writeReg(0x40 + kRhythmHiHatOp, _channels[channel].modLevel & 0x3F);
		return;
	}

	_opl->writeReg(0x40 + kOPLModOffset[channel], _channels[channel].modLevel & 0x3F);
	_opl->writeReg(0x40 + kOPLCarOffset[channel], _channels[channel].carLevel & 0x3F);
}

} // namespace Freescape
