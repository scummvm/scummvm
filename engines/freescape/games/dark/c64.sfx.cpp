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

#include "freescape/games/dark/c64.sfx.h"
#include "freescape/freescape.h"

#include "common/debug.h"
#include "common/textconsole.h"

namespace Freescape {

// 25 SFX entries extracted from dark2.prg at $C802 (address $C802-$CBEA).
// Each entry is 40 bytes in the original 6502 format.
// See SOUND_ANALYSIS.md for full documentation.
static const C64SFXData kC64SFXData[25] = {
	// SFX #1: Shoot (Noise, high→silence)
	{2, 1, 0,
	 {0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {21, 0, 0, 0, 0, 0, 0, 0, 0}, 1, 0,
	 0xEE, 0x02, 0x80, 0x13, 0xF6},

	// SFX #2: Hit (Noise, quick high→mid x3)
	{2, 3, 0,
	 {0x00, 0x44, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {4, 0, 0, 0, 0, 0, 0, 0, 0}, 1, 0,
	 0xEA, 0x01, 0x80, 0x23, 0xDA},

	// SFX #3: Step down (Pulse, high→silence)
	{2, 1, 0,
	 {0x00, 0x34, 0x00, 0x00, 0x00, 0x08, 0x00, 0x2C, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {50, 6, 2, 5, 0, 0, 0, 0, 0}, 1, 0,
	 0x72, 0x06, 0x40, 0x35, 0xF4},

	// SFX #4: Step up (Saw, silence→mid ascending)
	{2, 1, 0,
	 {0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {46, 0, 0, 0, 0, 0, 0, 0, 0}, 1, 0,
	 0xA2, 0x03, 0x20, 0x34, 0xF4},

	// SFX #5: Area change (Noise, flat→high sweep)
	{3, 1, 0,
	 {0x00, 0x34, 0x00, 0x34, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {14, 9, 0, 0, 0, 0, 0, 0, 0}, 1, 0,
	 0x64, 0x05, 0x80, 0x52, 0x6A},

	// SFX #6: Menu / area select (Triangle, hi→lo→lo→hi bounce)
	{3, 1, 0,
	 {0x00, 0x74, 0x00, 0x24, 0x00, 0x24, 0x00, 0x78, 0x00, 0x24,
	  0x00, 0x94, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {2, 2, 8, 6, 1, 0, 0, 0, 0}, 2, 0,
	 0xEA, 0x06, 0x10, 0x19, 0xF9},

	// SFX #7: Noise burst sequence
	{3, 1, 0,
	 {0x00, 0x1C, 0x00, 0x00, 0x00, 0x18, 0x00, 0x7C, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {4, 7, 2, 21, 0, 0, 0, 0, 0}, 1, 0,
	 0x38, 0x0F, 0x80, 0x38, 0xFC},

	// SFX #8: Triangle slide (repeat x5)
	{2, 5, 0,
	 {0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {6, 0, 0, 0, 0, 0, 0, 0, 0}, 2, 0,
	 0xD0, 0x02, 0x10, 0x19, 0xF7},

	// SFX #9: Game start (Triangle, rising 4-note arpeggio)
	{4, 1, 0,
	 {0x00, 0x90, 0x00, 0xC0, 0x00, 0x4C, 0x00, 0x4C, 0x00, 0x54,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {7, 10, 6, 8, 0, 0, 0, 0, 0}, 1, 0,
	 0xC8, 0x00, 0x10, 0x32, 0xF4},

	// SFX #10: Noise burst (repeat x2)
	{3, 2, 0,
	 {0x00, 0x68, 0x00, 0x70, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {2, 1, 0, 0, 0, 0, 0, 0, 0}, 1, 0,
	 0x06, 0x04, 0x80, 0x15, 0xF9},

	// SFX #11: Tri+Ring modulation
	{2, 1, 0,
	 {0x00, 0x00, 0x00, 0x2C, 0x00, 0x18, 0x00, 0x24, 0x00, 0x90,
	  0x00, 0x30, 0x00, 0x90, 0x00, 0xB0, 0x00, 0xC0, 0x00, 0x14}, 0,
	 {4, 8, 14, 19, 18, 4, 8, 3, 0x80}, 1, 0xB0,
	 0x00, 0xCC, 0x14, 0x25, 0xF7},

	// SFX #12: Long noise sequence (6 notes)
	{6, 1, 0,
	 {0x00, 0x4C, 0x00, 0x28, 0x00, 0x34, 0x00, 0x14, 0x00, 0x30,
	  0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {5, 9, 4, 13, 14, 0, 0, 0, 0}, 1, 0,
	 0x26, 0x0D, 0x80, 0x16, 0xFA},

	// SFX #13: Noise, 4-note descending
	{4, 1, 0,
	 {0x00, 0x7C, 0x00, 0x7C, 0x00, 0x50, 0x00, 0x44, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {2, 4, 4, 0, 0, 0, 0, 0, 0}, 1, 0,
	 0x90, 0x01, 0x80, 0x33, 0xFA},

	// SFX #14: Fall (Pulse, long high→silence slide)
	{2, 1, 0,
	 {0x00, 0xFC, 0x00, 0x00, 0x00, 0x90, 0x00, 0x6C, 0x00, 0x6C,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {43, 5, 4, 7, 0, 0, 0, 0, 0}, 2, 0,
	 0x30, 0x07, 0x40, 0x33, 0x79},

	// SFX #15: ECD destroy pre-noise (7-step rising sweep)
	{7, 1, 0,
	 {0x00, 0x00, 0x00, 0x04, 0x00, 0x10, 0x00, 0x30, 0x00, 0x50,
	  0x00, 0xA0, 0x00, 0xCC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {9, 12, 11, 5, 4, 2, 0, 0, 0}, 1, 0,
	 0x80, 0x02, 0x80, 0x35, 0xF9},

	// SFX #16: Tri+Ring (identical to #11)
	{2, 1, 0,
	 {0x00, 0x00, 0x00, 0x2C, 0x00, 0x18, 0x00, 0x24, 0x00, 0x90,
	  0x00, 0x30, 0x00, 0x90, 0x00, 0xB0, 0x00, 0xC0, 0x00, 0x14}, 0,
	 {4, 8, 14, 19, 18, 4, 8, 3, 0x80}, 1, 0xB0,
	 0x00, 0xCC, 0x14, 0x25, 0xF7},

	// SFX #17: Triangle slide down
	{2, 1, 0,
	 {0x00, 0xB0, 0x00, 0x40, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {5, 4, 0, 0, 0, 0, 0, 0, 0}, 1, 0,
	 0x90, 0x02, 0x10, 0x36, 0xF6},

	// SFX #18: Sawtooth oscillation (repeat x3)
	{4, 3, 0,
	 {0x00, 0xAC, 0x00, 0x84, 0x00, 0xAC, 0x00, 0x84, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {4, 2, 5, 0, 0, 0, 0, 0, 0}, 1, 0,
	 0xAA, 0x05, 0x20, 0x16, 0xF8},

	// SFX #19: Restore ECD (Noise, quick burst)
	{3, 1, 0,
	 {0x00, 0x18, 0x00, 0x24, 0x00, 0x14, 0x00, 0x00, 0x00, 0x34,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {2, 3, 2, 9, 0, 0, 0, 0, 0}, 1, 0,
	 0x62, 0x0D, 0x80, 0x16, 0xFA},

	// SFX #20: No shield / depleted (Triangle, 4-note warning x2)
	{4, 2, 0,
	 {0x00, 0x5C, 0x00, 0xA0, 0x00, 0xA0, 0x00, 0x60, 0x00, 0x8C,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {4, 4, 4, 5, 0, 0, 0, 0, 0}, 1, 0,
	 0x88, 0x09, 0x10, 0x14, 0xF6},

	// SFX #21: Sawtooth multi-note (speed=3)
	{4, 1, 0,
	 {0x00, 0x00, 0x00, 0x00, 0x00, 0x8C, 0x00, 0xBC, 0x00, 0x8C,
	  0x00, 0xC0, 0x00, 0x34, 0x00, 0x74, 0x00, 0x48, 0x00, 0x88}, 0,
	 {2, 6, 15, 4, 18, 4, 4, 5, 2}, 3, 3,
	 0xEA, 0x0B, 0x20, 0x32, 0xF8},

	// SFX #22: Noise ascending→silence
	{3, 1, 0,
	 {0x00, 0xB8, 0x00, 0xD4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {8, 7, 0, 0, 0, 0, 0, 0, 0}, 1, 0,
	 0xEA, 0x06, 0x80, 0x32, 0xF7},

	// SFX #23: Pulse pattern (repeat x5)
	{5, 5, 0,
	 {0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x24, 0x00, 0x00,
	  0x00, 0x40, 0x00, 0x40, 0x00, 0x30, 0x00, 0x28, 0x00, 0x00}, 0,
	 {7, 9, 8, 9, 13, 0, 17, 11, 0}, 1, 0,
	 0x84, 0x08, 0x40, 0x15, 0xF1},

	// SFX #24: Sawtooth sweep (repeat x5)
	{2, 5, 0,
	 {0x00, 0xE0, 0x00, 0x44, 0x00, 0x68, 0x00, 0x40, 0x00, 0x40,
	  0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {6, 0, 5, 0, 0, 0, 0, 0, 0}, 1, 0,
	 0x12, 0x01, 0x20, 0x39, 0xF8},

	// SFX #25: Noise multi-bounce (speed=2)
	{4, 1, 0,
	 {0x00, 0x7C, 0x00, 0x00, 0x00, 0x28, 0x00, 0x28, 0x00, 0x58,
	  0x00, 0x68, 0x00, 0x6C, 0x00, 0x64, 0x00, 0x54, 0x00, 0x3C}, 0,
	 {6, 5, 7, 7, 4, 5, 14, 8, 3}, 2, 0,
	 0x6E, 0x05, 0x80, 0x74, 0xF4},
};

DarkSideC64SFXPlayer::DarkSideC64SFXPlayer()
	: _sid(nullptr),
	  _state(0),
	  _numNotes(0), _repeatCount(0), _waveform(0), _speed(1),
	  _repeatLeft(0), _notesLeft(0), _freqIndex(0), _durIndex(0),
	  _durCounter(0), _speedCounter(0),
	  _curFreqLo(0), _curFreqHi(0) {
	memset(_startFreqs, 0, sizeof(_startFreqs));
	memset(_deltas, 0, sizeof(_deltas));
	memset(_durCopies, 0, sizeof(_durCopies));
	initSID();
}

DarkSideC64SFXPlayer::~DarkSideC64SFXPlayer() {
	destroySID();
}

void DarkSideC64SFXPlayer::destroySID() {
	if (_sid) {
		_sid->stop();
		delete _sid;
		_sid = nullptr;
	}
}

void DarkSideC64SFXPlayer::initSID() {
	if (_sid) {
		_sid->stop();
		delete _sid;
	}

	_sid = SID::Config::create(SID::Config::kSidPAL);
	if (!_sid || !_sid->init())
		error("Failed to initialise SID emulator for Dark Side SFX");

	for (int i = 0; i < 0x19; i++)
		sidWrite(i, 0);
	sidWrite(kSIDVolume, 0x0F);

	_sid->start(new Common::Functor0Mem<void, DarkSideC64SFXPlayer>(this, &DarkSideC64SFXPlayer::onTimer), 50);
}

void DarkSideC64SFXPlayer::sidWrite(int reg, uint8 data) {
	if (_sid) {
		debugC(4, kFreescapeDebugMedia, "Dark SFX SID Write: Reg $%02X = $%02X", reg, data);
		_sid->writeReg(reg, data);
	}
}

void DarkSideC64SFXPlayer::onTimer() {
	sfxTick();
}

bool DarkSideC64SFXPlayer::isSfxActive() const {
	return _state != 0;
}

void DarkSideC64SFXPlayer::stopAllSfx() {
	_state = 0;
	silenceAll();
}

void DarkSideC64SFXPlayer::silenceV1() {
	// $CE5C: zero V1 SID registers ($D400-$D406), clear state
	_state = 0;
	for (int i = kSIDV1FreqLo; i <= kSIDV1SR; i++)
		sidWrite(i, 0);
}

void DarkSideC64SFXPlayer::silenceAll() {
	// $CE57: zero all SID registers ($D400-$D413), clear state
	_state = 0;
	for (int i = 0; i <= 0x13; i++)
		sidWrite(i, 0);
	sidWrite(kSIDVolume, 0x0F);
}

// Signed 32÷16 division matching the original $2122 routine.
// Computes (dividend / divisor) as a signed 16-bit result.
static int16 signedDivide(int16 dividend, uint8 divisor) {
	if (divisor == 0)
		return 0;
	return dividend / (int16)divisor;
}

void DarkSideC64SFXPlayer::setupSfx(int index) {
	const C64SFXData &sfx = kC64SFXData[index];

	debugC(1, kFreescapeDebugMedia, "Dark Side C64 SFX: setup #%d (notes=%d repeat=%d wf=$%02X)",
		   index + 1, sfx.numNotes, sfx.repeatCount, sfx.waveform);

	// Call silence V1 first ($CE5C)
	silenceV1();

	// Gate off
	sidWrite(kSIDV1Ctrl, 0);

	// Load SID registers from descriptor
	sidWrite(kSIDV1PwLo, sfx.pwLo);
	sidWrite(kSIDV1PwHi, sfx.pwHi);
	sidWrite(kSIDV1AD, sfx.attackDecay);
	sidWrite(kSIDV1SR, sfx.sustainRelease);

	// Store working copies
	_numNotes = sfx.numNotes;
	_repeatCount = sfx.repeatCount;
	_waveform = sfx.waveform;
	_speed = sfx.speed;

	// Compute per-note start frequencies, deltas, and duration copies.
	// Matches the $CCFC loop: for each note transition, compute
	// delta = (freq[i+1] - freq[i]) / duration[i]
	int numTransitions = sfx.numNotes;
	for (int i = 0; i < numTransitions && i < 9; i++) {
		// Frequency waypoints are stored as (lo, hi) pairs at offsets 0,2,4...
		uint16 freqStart = sfx.freqWaypoints[i * 2] | (sfx.freqWaypoints[i * 2 + 1] << 8);
		uint16 freqEnd = sfx.freqWaypoints[(i + 1) * 2] | (sfx.freqWaypoints[(i + 1) * 2 + 1] << 8);

		_startFreqs[i] = (int16)freqStart;

		int16 diff = (int16)(freqEnd - freqStart);
		_deltas[i] = signedDivide(diff, sfx.durations[i]);

		_durCopies[i] = sfx.durations[i];

		debugC(2, kFreescapeDebugMedia, "  Note %d: freq $%04X→$%04X dur=%d delta=%d",
			   i, freqStart, freqEnd, sfx.durations[i], _deltas[i]);
	}

	// Set state to start phase
	_state = 1;
}

void DarkSideC64SFXPlayer::playSfx(int sfxIndex) {
	// Guard: SFX indices are 1-based, table is 0-based
	if (sfxIndex < 1 || sfxIndex > 25) {
		debugC(1, kFreescapeDebugMedia, "Dark Side C64 SFX: invalid index %d", sfxIndex);
		return;
	}

	// Guard: if SFX is already active, drop the request ($CC90-$CC93)
	if (_state != 0) {
		debugC(2, kFreescapeDebugMedia, "Dark Side C64 SFX: busy, dropping #%d", sfxIndex);
		return;
	}

	setupSfx(sfxIndex - 1);
}

void DarkSideC64SFXPlayer::sfxTick() {
	if (_state == 0)
		return;

	if (_state == 1) {
		tickStart();
		return;
	}

	if (_state == 2) {
		tickSlide();
		return;
	}

	// State $FF equivalent: cleanup → inactive
	_state = 0;
}

// State 1: Start phase ($CD8B-$CDD1)
// Load repeat count, reset indices, load first note, gate on, transition to slide.
void DarkSideC64SFXPlayer::tickStart() {
	_repeatLeft = _repeatCount;

	// Reset note counters and indices
	_notesLeft = _numNotes - 1;
	_freqIndex = 0;
	_durIndex = 0;

	// Load speed counter
	_speedCounter = _speed;

	// Load first note frequency
	uint16 freq = (uint16)_startFreqs[0];
	_curFreqLo = freq & 0xFF;
	_curFreqHi = (freq >> 8) & 0xFF;
	sidWrite(kSIDV1FreqLo, _curFreqLo);
	sidWrite(kSIDV1FreqHi, _curFreqHi);

	// Load first duration
	_durCounter = _durCopies[0];

	// Gate on: waveform | 0x01
	sidWrite(kSIDV1Ctrl, _waveform | 0x01);

	// Transition to slide state
	_state = 2;
}

// State 2: Slide phase ($CDD2-$CE4E)
// Decrement speed counter, update frequency with delta, advance notes.
void DarkSideC64SFXPlayer::tickSlide() {
	// Speed prescaler
	_speedCounter--;
	if (_speedCounter != 0)
		return;

	// Reload speed counter
	_speedCounter = _speed;

	// Update frequency: add delta for current note
	int noteIdx = _freqIndex / 2;
	uint16 freq = (_curFreqHi << 8) | _curFreqLo;
	freq = (uint16)((int16)freq + _deltas[noteIdx]);
	_curFreqLo = freq & 0xFF;
	_curFreqHi = (freq >> 8) & 0xFF;

	sidWrite(kSIDV1FreqLo, _curFreqLo);
	sidWrite(kSIDV1FreqHi, _curFreqHi);

	// Decrement duration
	_durCounter--;
	if (_durCounter != 0)
		return;

	// Duration expired: advance to next note
	_freqIndex += 2;
	_durIndex++;
	_notesLeft--;

	if (_notesLeft != 0) {
		// Load next note parameters
		noteIdx = _freqIndex / 2;
		_durCounter = _durCopies[_durIndex];
		_speedCounter = _speed;

		// Load start frequency for next note
		freq = (uint16)_startFreqs[noteIdx];
		_curFreqLo = freq & 0xFF;
		_curFreqHi = (freq >> 8) & 0xFF;
		sidWrite(kSIDV1FreqLo, _curFreqLo);
		sidWrite(kSIDV1FreqHi, _curFreqHi);
		return;
	}

	// All notes done: check repeat
	_repeatLeft--;
	if (_repeatLeft != 0) {
		// Restart sequence ($CD91): reload counters, keep same SFX data
		_notesLeft = _numNotes - 1;
		_freqIndex = 0;
		_durIndex = 0;
		_speedCounter = _speed;

		// Load first note frequency
		freq = (uint16)_startFreqs[0];
		_curFreqLo = freq & 0xFF;
		_curFreqHi = (freq >> 8) & 0xFF;
		sidWrite(kSIDV1FreqLo, _curFreqLo);
		sidWrite(kSIDV1FreqHi, _curFreqHi);

		_durCounter = _durCopies[0];
		return;
	}

	// Sequence finished: gate off
	_state = 0;
	sidWrite(kSIDV1Ctrl, _waveform & 0xFE);  // Clear gate bit
}

} // End of namespace Freescape
