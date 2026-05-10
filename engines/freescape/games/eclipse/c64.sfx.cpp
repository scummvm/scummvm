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

#include "freescape/games/eclipse/c64.sfx.h"
#include "freescape/freescape.h"

#include "common/debug.h"
#include "common/textconsole.h"

namespace Freescape {

// 21 SFX entries extracted from totec2.prg at $C802 (address $C802-$CB49).
// Each entry is 40 bytes. Same descriptor format as Dark Side C64.
static const C64SFXData kEclipseSFXData[21] = {
	// SFX #1: Noise, 2 notes, repeat 1
	{2, 1, 0,
	 {0x00, 0x44, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {4, 0, 0, 0, 0, 0, 0, 0, 0}, 1, 0,
	 0xEA, 0x01, 0x80, 0x15, 0x30},

	// SFX #2: Pulse, 4 notes, repeat 1
	{4, 1, 0,
	 {0x00, 0x24, 0x00, 0x84, 0x00, 0x28, 0x00, 0x80, 0x00, 0x24,
	  0x00, 0x94, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {2, 2, 2, 6, 1, 0, 0, 0, 0}, 1, 0,
	 0xEA, 0x06, 0x40, 0x19, 0x13},

	// SFX #3: Sawtooth, 2 notes, repeat 1
	{2, 1, 0,
	 {0x00, 0x08, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {8, 0, 0, 0, 0, 0, 0, 0, 0}, 1, 0,
	 0x9E, 0x02, 0x20, 0x0C, 0x04},

	// SFX #4: Triangle, 2 notes, repeat 1
	{2, 1, 0,
	 {0x00, 0xCC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {55, 0, 0, 0, 0, 0, 0, 0, 0}, 1, 0,
	 0xA0, 0x00, 0x10, 0x1E, 0x11},

	// SFX #5: Noise, 2 notes, repeat 1
	{2, 1, 0,
	 {0x00, 0x88, 0x00, 0x00, 0x00, 0x34, 0x00, 0x14, 0x00, 0x30,
	  0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {28, 9, 4, 13, 14, 0, 0, 0, 0}, 1, 0,
	 0x3A, 0x0D, 0x80, 0x19, 0x10},

	// SFX #6: Triangle, 2 notes, repeat 1
	{2, 1, 0,
	 {0x00, 0xCC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {55, 0, 0, 0, 0, 0, 0, 0, 0}, 1, 0,
	 0xA0, 0x00, 0x10, 0x1E, 0x11},

	// SFX #7: Pulse, 3 notes, repeat 4
	{3, 4, 0,
	 {0x00, 0x00, 0x00, 0x2C, 0x00, 0x24, 0x00, 0x28, 0x00, 0x58,
	  0x00, 0x68, 0x00, 0x6C, 0x00, 0x64, 0x00, 0x54, 0x00, 0x3C}, 0,
	 {3, 1, 7, 7, 4, 5, 14, 8, 3}, 1, 0,
	 0x62, 0x07, 0x40, 0x6B, 0xD4},

	// SFX #8: Noise, 7 notes, repeat 1
	{7, 1, 0,
	 {0x00, 0x00, 0x00, 0x04, 0x00, 0x10, 0x00, 0x30, 0x00, 0x50,
	  0x00, 0xA0, 0x00, 0xCC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {9, 12, 11, 5, 4, 2, 0, 0, 0}, 1, 0,
	 0x80, 0x02, 0x80, 0x35, 0xF9},

	// SFX #9: Triangle, 2 notes, repeat 5
	{2, 5, 0,
	 {0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {6, 0, 0, 0, 0, 0, 0, 0, 0}, 2, 0,
	 0xD0, 0x02, 0x10, 0x19, 0xF7},

	// SFX #10: Noise, 3 notes, repeat 1
	{3, 1, 0,
	 {0x00, 0x1C, 0x00, 0x00, 0x00, 0x18, 0x00, 0x7C, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {4, 7, 2, 21, 0, 0, 0, 0, 0}, 1, 0,
	 0x38, 0x0F, 0x80, 0x38, 0xFC},

	// SFX #11: Triangle, 4 notes, repeat 1
	{4, 1, 0,
	 {0x00, 0x90, 0x00, 0xC0, 0x00, 0x4C, 0x00, 0x4C, 0x00, 0x54,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {7, 10, 6, 8, 0, 0, 0, 0, 0}, 1, 0,
	 0xC8, 0x00, 0x10, 0x32, 0xF4},

	// SFX #12: Noise, 3 notes, repeat 1
	{3, 1, 0,
	 {0x00, 0xD4, 0x00, 0x00, 0x00, 0xD4, 0x00, 0x00, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {4, 5, 0, 0, 0, 0, 0, 0, 0}, 1, 0,
	 0xA2, 0x03, 0x80, 0x2A, 0x32},

	// SFX #13: Triangle, 4 notes, repeat 3
	{4, 3, 0,
	 {0x00, 0xAC, 0x00, 0x84, 0x00, 0xAC, 0x00, 0x84, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {4, 2, 5, 0, 0, 0, 0, 0, 0}, 1, 0,
	 0xAA, 0x05, 0x10, 0x16, 0xF8},

	// SFX #14: Sawtooth, 2 notes, repeat 5
	{2, 5, 0,
	 {0x00, 0x34, 0x00, 0x4C, 0x00, 0x50, 0x00, 0x44, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {7, 4, 4, 0, 0, 0, 0, 0, 0}, 1, 0,
	 0x90, 0x01, 0x20, 0x33, 0xFA},

	// SFX #15: Noise, 3 notes, repeat 1
	{3, 1, 0,
	 {0x00, 0xB8, 0x00, 0xD4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {8, 7, 0, 0, 0, 0, 0, 0, 0}, 1, 0,
	 0xEA, 0x06, 0x80, 0x15, 0xF7},

	// SFX #16: Sawtooth, 5 notes, repeat 1
	{5, 1, 0,
	 {0x00, 0x14, 0x00, 0x50, 0x00, 0x98, 0x00, 0xC4, 0x00, 0x94,
	  0x00, 0xC0, 0x00, 0x34, 0x00, 0x74, 0x00, 0x48, 0x00, 0x88}, 0,
	 {24, 15, 6, 6, 18, 4, 4, 5, 2}, 1, 3,
	 0xEA, 0x0B, 0x20, 0x32, 0xF8},

	// SFX #17: Sawtooth, 2 notes, repeat 1
	{2, 1, 0,
	 {0x00, 0x00, 0x00, 0x08, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {25, 7, 0, 0, 0, 0, 0, 0, 0}, 4, 0,
	 0x06, 0x04, 0x20, 0x65, 0xF0},

	// SFX #18: Noise, 3 notes, repeat 1
	{3, 1, 0,
	 {0x00, 0x0C, 0x00, 0x0C, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {2, 6, 0, 0, 0, 0, 0, 0, 0}, 1, 0,
	 0xB2, 0x02, 0x80, 0x29, 0x11},

	// SFX #19: Pulse, 4 notes, repeat 1
	{4, 1, 0,
	 {0x00, 0x08, 0x00, 0x08, 0x00, 0x28, 0x00, 0x60, 0x00, 0x68,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {8, 18, 9, 0, 0, 0, 0, 0, 0}, 2, 0,
	 0xEE, 0x02, 0x40, 0x4A, 0x11},

	// SFX #20: Pulse, 4 notes, repeat 1
	{4, 1, 0,
	 {0x00, 0x00, 0x00, 0x18, 0x00, 0x50, 0x00, 0x94, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {21, 11, 8, 5, 0, 0, 0, 0, 0}, 2, 0,
	 0x72, 0x06, 0x40, 0x35, 0xF4},

	// SFX #21: Noise, 2 notes, repeat 1
	{2, 1, 0,
	 {0x00, 0x4C, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0,
	 {15, 0, 0, 0, 0, 0, 0, 0, 0}, 1, 0,
	 0x20, 0x0E, 0x80, 0x26, 0x00},
};

static int16 signedDivide(int16 dividend, uint8 divisor) {
	if (divisor == 0)
		return 0;
	return dividend / (int16)divisor;
}

EclipseC64SFXPlayer::EclipseC64SFXPlayer()
	: _sid(nullptr),
	  _state(0),
	  _numNotes(0), _repeatCount(0), _waveform(0), _speed(1),
	  _repeatLeft(0), _notesLeft(0), _freqIndex(0), _durIndex(0),
	  _durCounter(0), _speedCounter(0),
	  _curFreqLo(0), _curFreqHi(0) {
	memset(_startFreqs, 0, sizeof(_startFreqs));
	memset(_deltas, 0, sizeof(_deltas));
	memset(_durCopies, 0, sizeof(_durCopies));
}

EclipseC64SFXPlayer::~EclipseC64SFXPlayer() {
	destroySID();
}

void EclipseC64SFXPlayer::destroySID() {
	if (_sid) {
		_sid->stop();
		delete _sid;
		_sid = nullptr;
	}
}

void EclipseC64SFXPlayer::initSID() {
	if (_sid) {
		_sid->stop();
		delete _sid;
	}

	_sid = SID::Config::create(SID::Config::kSidPAL);
	if (!_sid || !_sid->init())
		error("Failed to initialise SID emulator for Eclipse SFX");

	for (int i = 0; i < 0x19; i++)
		sidWrite(i, 0);
	sidWrite(kSIDVolume, 0x0F);

	_sid->start(new Common::Functor0Mem<void, EclipseC64SFXPlayer>(this, &EclipseC64SFXPlayer::onTimer), 50);
}

void EclipseC64SFXPlayer::sidWrite(int reg, uint8 data) {
	if (_sid) {
		debugC(4, kFreescapeDebugMedia, "Eclipse SFX SID Write: Reg $%02X = $%02X", reg, data);
		_sid->writeReg(reg, data);
	}
}

void EclipseC64SFXPlayer::onTimer() {
	sfxTick();
}

bool EclipseC64SFXPlayer::isSfxActive() const {
	return _state != 0;
}

void EclipseC64SFXPlayer::stopAllSfx() {
	_state = 0;
	silenceAll();
}

void EclipseC64SFXPlayer::silenceV1() {
	_state = 0;
	for (int i = kSIDV1FreqLo; i <= kSIDV1SR; i++)
		sidWrite(i, 0);
}

void EclipseC64SFXPlayer::silenceAll() {
	_state = 0;
	for (int i = 0; i <= 0x13; i++)
		sidWrite(i, 0);
	sidWrite(kSIDVolume, 0x0F);
}

void EclipseC64SFXPlayer::setupSfx(int index) {
	const C64SFXData &sfx = kEclipseSFXData[index];

	debugC(1, kFreescapeDebugMedia, "Eclipse C64 SFX: setup #%d (notes=%d repeat=%d wf=$%02X)",
		   index + 1, sfx.numNotes, sfx.repeatCount, sfx.waveform);

	silenceV1();

	sidWrite(kSIDV1Ctrl, 0);
	sidWrite(kSIDV1PwLo, sfx.pwLo);
	sidWrite(kSIDV1PwHi, sfx.pwHi);
	sidWrite(kSIDV1AD, sfx.attackDecay);
	sidWrite(kSIDV1SR, sfx.sustainRelease);

	_numNotes = sfx.numNotes;
	_repeatCount = sfx.repeatCount;
	_waveform = sfx.waveform;
	_speed = sfx.speed;

	int numTransitions = sfx.numNotes;
	for (int i = 0; i < numTransitions && i < 9; i++) {
		uint16 freqStart = sfx.freqWaypoints[i * 2] | (sfx.freqWaypoints[i * 2 + 1] << 8);
		uint16 freqEnd = sfx.freqWaypoints[(i + 1) * 2] | (sfx.freqWaypoints[(i + 1) * 2 + 1] << 8);

		_startFreqs[i] = (int16)freqStart;

		int16 diff = (int16)(freqEnd - freqStart);
		_deltas[i] = signedDivide(diff, sfx.durations[i]);

		_durCopies[i] = sfx.durations[i];

		debugC(2, kFreescapeDebugMedia, "  Note %d: freq $%04X->$%04X dur=%d delta=%d",
			   i, freqStart, freqEnd, sfx.durations[i], _deltas[i]);
	}

	_state = 1;
}

void EclipseC64SFXPlayer::playSfx(int sfxIndex) {
	if (sfxIndex < 1 || sfxIndex > 21) {
		debugC(1, kFreescapeDebugMedia, "Eclipse C64 SFX: invalid index %d", sfxIndex);
		return;
	}

	if (_state != 0) {
		debugC(2, kFreescapeDebugMedia, "Eclipse C64 SFX: busy, dropping #%d", sfxIndex);
		return;
	}

	setupSfx(sfxIndex - 1);
}

void EclipseC64SFXPlayer::sfxTick() {
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

	_state = 0;
}

void EclipseC64SFXPlayer::tickStart() {
	_repeatLeft = _repeatCount;

	_notesLeft = _numNotes - 1;
	_freqIndex = 0;
	_durIndex = 0;

	_speedCounter = _speed;

	uint16 freq = (uint16)_startFreqs[0];
	_curFreqLo = freq & 0xFF;
	_curFreqHi = (freq >> 8) & 0xFF;
	sidWrite(kSIDV1FreqLo, _curFreqLo);
	sidWrite(kSIDV1FreqHi, _curFreqHi);

	_durCounter = _durCopies[0];

	sidWrite(kSIDV1Ctrl, _waveform | 0x01);

	_state = 2;
}

void EclipseC64SFXPlayer::tickSlide() {
	_speedCounter--;
	if (_speedCounter != 0)
		return;

	_speedCounter = _speed;

	int noteIdx = _freqIndex / 2;
	uint16 freq = (_curFreqHi << 8) | _curFreqLo;
	freq = (uint16)((int16)freq + _deltas[noteIdx]);
	_curFreqLo = freq & 0xFF;
	_curFreqHi = (freq >> 8) & 0xFF;

	sidWrite(kSIDV1FreqLo, _curFreqLo);
	sidWrite(kSIDV1FreqHi, _curFreqHi);

	_durCounter--;
	if (_durCounter != 0)
		return;

	_freqIndex += 2;
	_durIndex++;
	_notesLeft--;

	if (_notesLeft != 0) {
		noteIdx = _freqIndex / 2;
		_durCounter = _durCopies[_durIndex];
		_speedCounter = _speed;

		freq = (uint16)_startFreqs[noteIdx];
		_curFreqLo = freq & 0xFF;
		_curFreqHi = (freq >> 8) & 0xFF;
		sidWrite(kSIDV1FreqLo, _curFreqLo);
		sidWrite(kSIDV1FreqHi, _curFreqHi);
		return;
	}

	_repeatLeft--;
	if (_repeatLeft != 0) {
		_notesLeft = _numNotes - 1;
		_freqIndex = 0;
		_durIndex = 0;
		_speedCounter = _speed;

		freq = (uint16)_startFreqs[0];
		_curFreqLo = freq & 0xFF;
		_curFreqHi = (freq >> 8) & 0xFF;
		sidWrite(kSIDV1FreqLo, _curFreqLo);
		sidWrite(kSIDV1FreqHi, _curFreqHi);

		_durCounter = _durCopies[0];
		return;
	}

	_state = 0;
	sidWrite(kSIDV1Ctrl, _waveform & 0xFE);
}

} // End of namespace Freescape
