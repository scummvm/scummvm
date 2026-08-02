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

#include "engines/freescape/games/dark/opl.music.h"

#include "common/debug.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "freescape/freescape.h"
#include "freescape/games/dark/dark.musicdata.h"

using namespace Freescape::DarkMusicData;

namespace Freescape {

// Converted from the SID frequency table in dark.musicdata.h, within 2.3 cents.
// The tune is written at A4 = 433.5 Hz, so this is not concert pitch.
const uint16 kDarkOPLFreqs[96] = {
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
	0x1BC1, 0x1BFA, 0x1E1B, 0x1E3B, 0x1E5D, 0x0000
};

const byte kDarkOPLModOffset[] = { 0x00, 0x01, 0x02 };
const byte kDarkOPLCarOffset[] = { 0x03, 0x04, 0x05 };

struct DarkOPLPatch {
	byte modChar;            // reg 0x20: AM | VIB | EGT | KSR | MULT
	byte carChar;
	byte modLevel;           // reg 0x40: modulator total level == FM index
	byte carLevel;
	byte modWave;            // reg 0xE0
	byte carWave;
	byte feedbackConnection; // reg 0xC0
};

// Every operator sets EGT (0x20) so the envelope holds at the sustain level
// until key-off, like the SID gate bit.
//
// The melodic voices are least-squares fits of the two-operator FM spectrum to
// the analytic SID spectra over the first 16 harmonics: triangle is odd
// harmonics at 1/n^2, sawtooth every harmonic at 1/n, pulse of duty d is
// |sin(pi*n*d)|/n. The fit wants a saw-like modulator, which is what moderate
// feedback produces. Feedback 7 is chaotic and harsh, none at all is dull and
// quiet; 4 to 5 is where it sits.
const DarkOPLPatch kDarkOPLPatches[] = {
	// 0: silent
	{ 0x20, 0x20, 0x3F, 0x3F, 0x00, 0x00, 0x00 },
	// 1: triangle - 2:1, gentle
	{ 0x22, 0x21, 0x28, 0x00, 0x00, 0x00, 0x08 },
	// 2: sawtooth - 1:1, the classic fed-back OPL ramp
	{ 0x21, 0x21, 0x19, 0x00, 0x00, 0x00, 0x0A },
	// 3: pulse - 1:1, index set per frame from the pulse width
	{ 0x21, 0x21, 0x18, 0x00, 0x00, 0x00, 0x08 },
	// 4: noise - single operator, played on the rhythm-mode hi-hat
	{ 0x2E, 0x20, 0x08, 0x3F, 0x00, 0x00, 0x00 }
};

// FM index and carrier attenuation per pulse-width high nibble. The index is
// from the same fit; the carrier term is the RMS a pulse of that duty loses
// against a square, at half strength because the harmonics a thin pulse leans
// on sit where the ear is most sensitive.
const byte kPulseWidthModLevel[16] = {
	18, 22, 23, 24, 26, 28, 29, 29, 29, 29, 28, 26, 24, 23, 22, 18
};

const byte kPulseWidthCarLevel[16] = {
	 6,  3,  2,  1,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  6
};

const byte kDarkNoiseFamily = 4;

// Flag bit 3 opens a note with one frame of noise at a fixed pitch. The OPL has
// no melodic noise, so that frame is a dense inharmonic burst at the SID's
// pitch ($4800, 1084 Hz).
const uint16 kSpecialAttackFnum = 714;
const byte kSpecialAttackBlock = 5;
const byte kSpecialAttackChar = 0x2F;  // EGT | MULT 15

// OPL2 rhythm mode: bit 5 of register 0xBD turns channels 6-8 into percussion,
// the only real noise on an OPL2. Melodic playback only uses channels 0-2.
const byte kDarkRhythmEnable   = 0x20;
const byte kDarkRhythmHiHatBit = 0x01;
const byte kDarkRhythmHiHatOp  = 0x11; // channel 7 modulator operator offset
const byte kDarkRhythmChannel  = 7;

// SID envelope nibbles to OPL2 rates, matched in log space against
// 0.22 * 2^(14-AR) ms for attack and 1.27 * 2^(15-rate) ms for decay/release.
// SID's slowest rates are beyond the OPL and clamp to 1.
const byte kDarkAttackToOPL[16] = {
	11, 9, 8, 7, 7, 6, 6, 5, 5, 4, 3, 2, 2, 1, 1, 1
};

const byte kDarkDecayToOPL[16] = {
	13, 11, 10, 9, 9, 8, 8, 7, 7, 6, 5, 4, 4, 2, 1, 1
};

// SID sustain is a linear fraction S/15, OPL sustain level is attenuation in
// 3 dB steps. Nibble 0 means silence.
const byte kDarkSustainToOPL[16] = {
	15, 8, 6, 5, 4, 3, 3, 2, 2, 1, 1, 1, 1, 0, 0, 0
};

byte darkWaveformFamily(byte ctrl) {
	if (ctrl & 0x80)
		return 4;
	if (ctrl & 0x40)
		return 3;
	if (ctrl & 0x20)
		return 2;
	if (ctrl & 0x10)
		return 1;
	return 0;
}

// ============================================================================
// ChannelState
// ============================================================================

void DarkSideOPLMusicPlayer::ChannelState::reset() {
	orderList = nullptr;
	orderPos = 0;
	pattern = nullptr;
	patternPos = 0;
	instrumentOffset = 0;
	currentNote = 0;
	transpose = 0;
	frequencyFnum = 0;
	frequencyBlock = 0;
	durationReload = 0;
	durationCounter = 0;
	effectMode = 0;
	effectParam = 0;
	slideTarget = 0;
	slideParam = 0;
	arpeggioPos = 0;
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
	specialAttack = false;
	attackDone = false;
	envCounter = 0;
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
// Construction / public interface
// ============================================================================

DarkSideOPLMusicPlayer::DarkSideOPLMusicPlayer()
	: _opl(nullptr), _musicActive(false), _speedDivider(1), _speedCounter(0),
	  _rhythmReg(kDarkRhythmEnable) {
	_opl = OPL::Config::create();
	if (!_opl || !_opl->init()) {
		warning("DarkSideOPLMusicPlayer: Failed to create OPL emulator");
		delete _opl;
		_opl = nullptr;
	}
}

DarkSideOPLMusicPlayer::~DarkSideOPLMusicPlayer() {
	stopMusic();
	delete _opl;
}

void DarkSideOPLMusicPlayer::startMusic() {
	if (!_opl)
		return;
	stopMusic();
	// The C64 leaves CIA#1 Timer A at the KERNAL default and ticks the music
	// once per IRQ, so the sequencer runs at 60 Hz, not the 50 Hz video rate.
	_opl->start(new Common::Functor0Mem<void, DarkSideOPLMusicPlayer>(
		this, &DarkSideOPLMusicPlayer::onTimer), 60);
	setupSong();
}

void DarkSideOPLMusicPlayer::stopMusic() {
	_musicActive = false;
	if (_opl) {
		silenceAll();
		_opl->stop();
	}
}

bool DarkSideOPLMusicPlayer::isPlaying() const {
	return _musicActive;
}

// ============================================================================
// OPL register helpers
// ============================================================================

void DarkSideOPLMusicPlayer::noteToFnumBlock(byte note, uint16 &fnum, byte &block) const {
	if (note > kMaxNote)
		note = kMaxNote;
	uint16 combined = kDarkOPLFreqs[note];
	fnum = combined & 0x03FF;
	block = (combined >> 10) & 0x07;
}

// Push a pitch to the chip without disturbing the note's base frequency: the
// per-frame effects offset from that base, so storing their result would make
// each frame modulate the last one and walk the note away. Only note-on and the
// timed slide, which is meant to accumulate, go through setFrequency().
void DarkSideOPLMusicPlayer::writeFrequency(int channel, uint16 fnum, byte block) {
	if (!_opl)
		return;

	// A rhythm voice is pitched from the percussion channel and must never
	// have the key-on bit set.
	if (_channels[channel].rhythmVoice) {
		_opl->writeReg(0xA0 + kDarkRhythmChannel, fnum & 0xFF);
		_opl->writeReg(0xB0 + kDarkRhythmChannel, ((fnum >> 8) & 0x03) | (block << 2));
		return;
	}

	_opl->writeReg(0xA0 + channel, fnum & 0xFF);
	byte b0 = ((fnum >> 8) & 0x03) | (block << 2);
	if (_channels[channel].keyOn)
		b0 |= 0x20;
	_opl->writeReg(0xB0 + channel, b0);
}

void DarkSideOPLMusicPlayer::setFrequency(int channel, uint16 fnum, byte block) {
	_channels[channel].frequencyFnum = fnum;
	_channels[channel].frequencyBlock = block;
	writeFrequency(channel, fnum, block);
}

void DarkSideOPLMusicPlayer::programEnvelope(byte op, byte attack, byte decay, byte sustain, byte release) {
	_opl->writeReg(0x60 + op, (attack << 4) | decay);
	_opl->writeReg(0x80 + op, (sustain << 4) | release);
}

void DarkSideOPLMusicPlayer::setOPLInstrument(int channel, byte instrumentOffset) {
	if (!_opl)
		return;

	byte ctrl = kInstruments[instrumentOffset + 0];
	byte attackDecay = kInstruments[instrumentOffset + 1];
	byte sustainRelease = kInstruments[instrumentOffset + 2];
	byte initialPulseWidth = kInstruments[instrumentOffset + 3];
	byte pulseWidthMod = kInstruments[instrumentOffset + 5];
	byte family = darkWaveformFamily(ctrl);
	const DarkOPLPatch &patch = kDarkOPLPatches[family];

	byte attack = kDarkAttackToOPL[attackDecay >> 4];
	byte decay = kDarkDecayToOPL[attackDecay & 0x0F];
	byte sustain = kDarkSustainToOPL[sustainRelease >> 4];
	byte release = kDarkDecayToOPL[sustainRelease & 0x0F];

	bool wasRhythm = _channels[channel].rhythmVoice;
	_channels[channel].rhythmVoice = (family == kDarkNoiseFamily);

	// Flag bit 7 carries the pulse width on instead of restarting it. The main
	// bass begins at 0% duty and relies on this: resetting every note pins it
	// at the thinnest, quietest end of the sweep.
	if ((kInstruments[instrumentOffset + 7] & 0x80) == 0) {
		_channels[channel].pulseWidth = ((initialPulseWidth & 0x0F) << 8) | (initialPulseWidth & 0xF0);
		_channels[channel].pulseWidthDirection = 0;
	}
	_channels[channel].pulseWidthMod = pulseWidthMod;
	_channels[channel].modBaseLevel = patch.modLevel;
	_channels[channel].carBaseLevel = patch.carLevel;
	_channels[channel].modLevel = patch.modLevel;
	_channels[channel].carLevel = patch.carLevel;

	if (_channels[channel].rhythmVoice) {
		// Silence the melodic channel this voice would otherwise have used.
		_channels[channel].keyOn = false;
		_opl->writeReg(0xB0 + channel, 0x00);
		_opl->writeReg(0x40 + kDarkOPLModOffset[channel], 0x3F);
		_opl->writeReg(0x40 + kDarkOPLCarOffset[channel], 0x3F);

		_opl->writeReg(0x20 + kDarkRhythmHiHatOp, patch.modChar);
		_opl->writeReg(0xE0 + kDarkRhythmHiHatOp, patch.modWave);
		programEnvelope(kDarkRhythmHiHatOp, attack, decay, sustain, release);
		updatePulseWidth(channel, false);
		applyOperatorLevels(channel);
		return;
	}

	if (wasRhythm) {
		_rhythmReg &= ~kDarkRhythmHiHatBit;
		_opl->writeReg(0xBD, _rhythmReg);
	}

	byte mod = kDarkOPLModOffset[channel];
	byte car = kDarkOPLCarOffset[channel];

	_opl->writeReg(0x20 + mod, patch.modChar);
	_opl->writeReg(0x20 + car, patch.carChar);

	// The FM index follows the modulator's absolute output, and a SID
	// oscillator keeps its waveform as the note decays, so hold the modulator
	// flat and let only the carrier follow the ADSR. Enveloping it collapses
	// the index in milliseconds and leaves a bass note inaudible.
	programEnvelope(mod, 15, 0, 0, 0);
	programEnvelope(car, attack, decay, sustain, release);
	_opl->writeReg(0xE0 + mod, patch.modWave);
	_opl->writeReg(0xE0 + car, patch.carWave);
	_opl->writeReg(0xC0 + channel, patch.feedbackConnection);

	updatePulseWidth(channel, false);
	applyOperatorLevels(channel);
}

void DarkSideOPLMusicPlayer::noteOn(int channel) {
	if (!_opl)
		return;
	_channels[channel].keyOn = true;

	if (_channels[channel].rhythmVoice) {
		// Percussion voices are keyed from register 0xBD, not from 0xB0.
		_opl->writeReg(0xBD, _rhythmReg & ~kDarkRhythmHiHatBit);
		_rhythmReg |= kDarkRhythmHiHatBit;
		_opl->writeReg(0xBD, _rhythmReg);
		return;
	}

	_opl->writeReg(0xA0 + channel, _channels[channel].frequencyFnum & 0xFF);
	_opl->writeReg(0xB0 + channel, 0x20 | (_channels[channel].frequencyBlock << 2) |
	                                 ((_channels[channel].frequencyFnum >> 8) & 0x03));
}

void DarkSideOPLMusicPlayer::noteOff(int channel) {
	if (!_opl)
		return;
	_channels[channel].keyOn = false;

	if (_channels[channel].rhythmVoice) {
		_rhythmReg &= ~kDarkRhythmHiHatBit;
		_opl->writeReg(0xBD, _rhythmReg);
		return;
	}

	_opl->writeReg(0xB0 + channel, ((_channels[channel].frequencyFnum >> 8) & 0x03) |
	                                 (_channels[channel].frequencyBlock << 2));
}

// ============================================================================
// Timer / sequencer core
// ============================================================================

void DarkSideOPLMusicPlayer::onTimer() {
	if (!_musicActive)
		return;

	for (int channel = 0; channel < kChannelCount; channel++)
		_channels[channel].envCounter++;

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

void DarkSideOPLMusicPlayer::processChannel(int channel, bool newBeat) {
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

void DarkSideOPLMusicPlayer::finalizeChannel(int channel) {
	// Mirrors the SID engine clearing the gate bit halfway through the note.
	if (_channels[channel].durationReload != 0 &&
	    !_channels[channel].gateOffDisabled &&
	    ((_channels[channel].durationReload >> 1) == _channels[channel].durationCounter) &&
	    _channels[channel].keyOn) {
		noteOff(channel);
	}

	updatePulseWidth(channel, true);
	applyOperatorLevels(channel);
}

// ============================================================================
// Song setup
// ============================================================================

void DarkSideOPLMusicPlayer::setupSong() {
	silenceAll();

	if (_opl)
		_opl->writeReg(0x01, 0x20); // wave select, needed for non-sine waveforms

	_speedDivider = 1;
	_speedCounter = 0;

	for (int i = 0; i < kChannelCount; i++) {
		_channels[i].reset();
		_channels[i].orderList = kOrderLists[i];
		loadNextPattern(i);
	}

	_musicActive = true;
}

void DarkSideOPLMusicPlayer::silenceAll() {
	if (!_opl)
		return;
	for (int ch = 0; ch < kChannelCount; ch++) {
		_channels[ch].keyOn = false;
		_opl->writeReg(0xB0 + ch, 0x00);
		_opl->writeReg(0x40 + kDarkOPLModOffset[ch], 0x3F);
		_opl->writeReg(0x40 + kDarkOPLCarOffset[ch], 0x3F);
	}

	// Leave rhythm mode armed with every trigger released.
	_rhythmReg = kDarkRhythmEnable;
	_opl->writeReg(0xBD, _rhythmReg);
	_opl->writeReg(0x40 + kDarkRhythmHiHatOp, 0x3F);
}

// ============================================================================
// Order list / pattern navigation
// ============================================================================

void DarkSideOPLMusicPlayer::loadNextPattern(int channel) {
	int safety = 200;
	while (safety-- > 0) {
		byte value = _channels[channel].orderList[_channels[channel].orderPos];
		_channels[channel].orderPos++;

		if (value == 0xFF) {
			_channels[channel].orderPos = 0;
			continue;
		}

		// The C64 order lists encode a transpose differently from the Amiga and
		// Atari ones: anything from $80 up, biased by $40 rather than $20.
		if (value >= 0x80) {
			_channels[channel].transpose = (byte)((value + 0x40) & 0xFF);
			continue;
		}

		if (value < ARRAYSIZE(kPatterns)) {
			_channels[channel].pattern = kPatterns[value];
			_channels[channel].patternPos = 0;
			debugC(3, kFreescapeDebugMedia, "Dark-AdLib: ch%d order -> pattern %d (transpose %d)",
				channel, value, (int8)_channels[channel].transpose);
		}
		break;
	}
}

byte DarkSideOPLMusicPlayer::readPatternByte(int channel) {
	return _channels[channel].pattern[_channels[channel].patternPos++];
}

byte DarkSideOPLMusicPlayer::clampNote(int note) const {
	if (note < 0)
		return 0;
	return note > kMaxNote ? (byte)kMaxNote : (byte)note;
}

// ============================================================================
// Pattern command parser
// ============================================================================

void DarkSideOPLMusicPlayer::parseCommands(int channel) {
	if (_channels[channel].effectMode != 2) {
		_channels[channel].effectParam = 0;
		_channels[channel].effectMode = 0;
		_channels[channel].arpeggioSequenceLen = 0;
		_channels[channel].arpeggioPos = 0;
	}
	_channels[channel].slideTarget = 0;
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
			// SID filter, no OPL equivalent
			readPatternByte(channel);
			cmd = readPatternByte(channel);
			if (cmd == 0xFF) {
				loadNextPattern(channel);
				continue;
			}
		}

		if (cmd >= 0xF0) {
			_speedDivider = cmd & 0x0F;
			debugC(2, kFreescapeDebugMedia, "Dark-AdLib: ch%d speed $%02X -> %d ticks per beat",
				channel, cmd, _speedDivider + 1);
			continue;
		}

		if (cmd >= 0xC0) {
			byte instrument = cmd & 0x1F;
			if (instrument < 18)
				_channels[channel].instrumentOffset = instrument * 8;
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
			_channels[channel].slideTarget = readPatternByte(channel) + _channels[channel].transpose;
			_channels[channel].slideParam = readPatternByte(channel);
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

void DarkSideOPLMusicPlayer::applyNote(int channel, byte note) {
	byte instrumentOffset = _channels[channel].instrumentOffset;
	byte ctrl = kInstruments[instrumentOffset + 0];
	byte sustainRelease = kInstruments[instrumentOffset + 2];
	byte autoEffect = kInstruments[instrumentOffset + 6];
	byte flags = kInstruments[instrumentOffset + 7];
	byte actualNote = note;
	bool gateEnabled = (ctrl & 0x01) != 0;

	if (actualNote != 0)
		actualNote = clampNote((actualNote + _channels[channel].transpose) & 0xFF);

	_channels[channel].currentNote = actualNote;
	_channels[channel].waveform = ctrl;
	_channels[channel].instrumentFlags = flags;
	_channels[channel].stepDownCounter = 0;
	_channels[channel].specialAttack = (flags & 0x08) != 0;
	_channels[channel].attackDone = false;
	_channels[channel].envCounter = 0xFF;

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

	byte instrument = instrumentOffset / 8;
	byte family = darkWaveformFamily(ctrl);

	if (actualNote == 0 || !gateEnabled) {
		noteOff(channel);
		debugC(1, kFreescapeDebugMedia, "Dark-AdLib: ch%d rest  inst=%-2d dur=%d",
			channel, instrument, _channels[channel].durationReload);
	} else {
		// Key-off then key-on restarts the envelope from its current level,
		// like re-gating a held SID voice.
		noteOff(channel);
		noteOn(channel);
		debugC(1, kFreescapeDebugMedia,
			"Dark-AdLib: ch%d NOTE %3d (%d%+d) %5dHz inst=%-2d dur=%d%s%s",
			channel, _channels[channel].currentNote, note, (int8)_channels[channel].transpose,
			(int)(((uint32)_channels[channel].frequencyFnum * 49716) >> (20 - _channels[channel].frequencyBlock)),
			instrument, _channels[channel].durationReload,
			family == kDarkNoiseFamily ? " [rhythm hi-hat]" : "",
			_channels[channel].effectParam ? " [arpeggio]" : "");
	}

	_channels[channel].durationCounter = _channels[channel].durationReload;
	_channels[channel].delayCounter = _channels[channel].delayValue;
	_channels[channel].arpeggioPos = 0;
}

void DarkSideOPLMusicPlayer::loadCurrentFrequency(int channel) {
	uint16 fnum;
	byte block;
	noteToFnumBlock(clampNote(_channels[channel].currentNote), fnum, block);
	setFrequency(channel, fnum, block);
}

// ============================================================================
// Effects
// ============================================================================

void DarkSideOPLMusicPlayer::buildEffectArpeggio(int channel) {
	byte len = 0;
	for (int i = 0; i < 8 && len < 8; i++) {
		if (_channels[channel].effectParam & (1 << i))
			_channels[channel].arpeggioSequence[len++] = kArpIntervals[i];
	}
	if (len > 0)
		_channels[channel].arpeggioSequence[len++] = 0;
	_channels[channel].arpeggioSequenceLen = len;
	_channels[channel].arpeggioPos = 0;
}

void DarkSideOPLMusicPlayer::applyFrameEffects(int channel) {
	if (_channels[channel].currentNote == 0)
		return;

	if (applySpecialAttack(channel))
		return;

	if (applyInstrumentVibrato(channel))
		return;

	applyEffectArpeggio(channel);
	applyTimedSlide(channel);
}

bool DarkSideOPLMusicPlayer::applySpecialAttack(int channel) {
	ChannelState &c = _channels[channel];
	if (!c.specialAttack || c.attackDone || c.rhythmVoice || !_opl)
		return false;

	byte mod = kDarkOPLModOffset[channel];
	if (c.envCounter < 1) {
		_opl->writeReg(0x20 + mod, kSpecialAttackChar);
		_opl->writeReg(0x40 + mod, 0x00);
		writeFrequency(channel, kSpecialAttackFnum, kSpecialAttackBlock);
	} else {
		_opl->writeReg(0x20 + mod, kDarkOPLPatches[darkWaveformFamily(c.waveform)].modChar);
		applyOperatorLevels(channel);
		loadCurrentFrequency(channel);
		c.attackDone = true;
	}
	return true;
}

bool DarkSideOPLMusicPlayer::applyInstrumentVibrato(int channel) {
	byte vibrato = kInstruments[_channels[channel].instrumentOffset + 4];
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

	int32 delta = ((int32)nextFnum << nextBlock) - ((int32)noteFnum << noteBlock);
	if (delta <= 0)
		return false;

	while (shift-- != 0)
		delta >>= 1;

	if (_channels[channel].vibratoPhase & 0x80) {
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

	if (freq < 1)
		freq = 1;

	byte block = 0;
	while (freq > 1023 && block < 7) {
		freq >>= 1;
		block++;
	}
	writeFrequency(channel, freq & 0x3FF, block);
	return true;
}

void DarkSideOPLMusicPlayer::applyEffectArpeggio(int channel) {
	if (_channels[channel].effectParam == 0 || _channels[channel].arpeggioSequenceLen == 0)
		return;

	if (_channels[channel].arpeggioPos >= _channels[channel].arpeggioSequenceLen)
		_channels[channel].arpeggioPos = 0;

	byte note = clampNote(_channels[channel].currentNote +
	                      _channels[channel].arpeggioSequence[_channels[channel].arpeggioPos]);
	_channels[channel].arpeggioPos++;

	uint16 fnum;
	byte block;
	noteToFnumBlock(note, fnum, block);
	writeFrequency(channel, fnum, block);
}

void DarkSideOPLMusicPlayer::applyTimedSlide(int channel) {
	if (_channels[channel].slideTarget == 0)
		return;

	byte total = _channels[channel].durationReload;
	byte remaining = _channels[channel].durationCounter;
	byte start = _channels[channel].slideParam >> 4;
	byte span = _channels[channel].slideParam & 0x0F;
	byte elapsed = total - remaining;

	if (elapsed <= start || elapsed > start + span || span == 0)
		return;

	byte currentNote = clampNote(_channels[channel].currentNote);
	byte targetNote = clampNote(_channels[channel].slideTarget);
	if (currentNote == targetNote)
		return;

	uint16 srcFnum, tgtFnum;
	byte srcBlock, tgtBlock;
	noteToFnumBlock(currentNote, srcFnum, srcBlock);
	noteToFnumBlock(targetNote, tgtFnum, tgtBlock);

	int32 srcFreq = (int32)srcFnum << srcBlock;
	int32 tgtFreq = (int32)tgtFnum << tgtBlock;
	int32 difference = ABS(srcFreq - tgtFreq);
	uint16 divisor = span * (_speedDivider + 1);
	if (divisor == 0)
		return;

	int32 delta = difference / divisor;
	if (delta == 0)
		return;

	int32 curFreq = (int32)_channels[channel].frequencyFnum << _channels[channel].frequencyBlock;
	curFreq += (tgtFreq > srcFreq) ? delta : -delta;
	if (curFreq < 1)
		curFreq = 1;

	byte block = 0;
	while (curFreq > 1023 && block < 7) {
		curFreq >>= 1;
		block++;
	}
	setFrequency(channel, curFreq & 0x3FF, block);
}

// ============================================================================
// Operator levels
// ============================================================================

void DarkSideOPLMusicPlayer::updatePulseWidth(int channel, bool advance) {
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

	// The SID pulse width is 12 bits and sweeps continuously, so interpolate
	// between entries. The high nibble alone quantises it to sixteen steps,
	// which reads as a static tone -- and on a sustained note that sweep is the
	// whole character of the voice.
	uint16 pw = _channels[channel].pulseWidth & 0x0FFF;
	byte low = pw & 0xFF;
	byte duty = pw >> 8;
	byte next = (duty + 1) & 0x0F;

	int mod = kPulseWidthModLevel[duty] +
	          ((kPulseWidthModLevel[next] - kPulseWidthModLevel[duty]) * low) / 256;
	int car = kPulseWidthCarLevel[duty] +
	          ((kPulseWidthCarLevel[next] - kPulseWidthCarLevel[duty]) * low) / 256;

	_channels[channel].modLevel = (byte)CLIP(mod, 0, 63);
	_channels[channel].carLevel = MIN<byte>(_channels[channel].carBaseLevel + car, 0x3F);
}

// The envelope lives in the chip, so the total-level registers only carry the
// patch levels plus the pulse-width brightness motion.
void DarkSideOPLMusicPlayer::applyOperatorLevels(int channel) {
	if (!_opl)
		return;

	if (_channels[channel].rhythmVoice) {
		_opl->writeReg(0x40 + kDarkRhythmHiHatOp, _channels[channel].modLevel & 0x3F);
		return;
	}

	_opl->writeReg(0x40 + kDarkOPLModOffset[channel], _channels[channel].modLevel & 0x3F);
	_opl->writeReg(0x40 + kDarkOPLCarOffset[channel], _channels[channel].carLevel & 0x3F);
}

} // End of namespace Freescape
