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

#ifndef FREESCAPE_DARK_C64_SFX_H
#define FREESCAPE_DARK_C64_SFX_H

#include "audio/sid.h"
#include "freescape/sid.h"

namespace Freescape {

class DarkSideC64SFXPlayer {
public:
	DarkSideC64SFXPlayer();
	~DarkSideC64SFXPlayer();

	void playSfx(int sfxIndex);
	void sfxTick();
	void stopAllSfx();

	bool isSfxActive() const;
	void initSID();
	void destroySID();

private:
	SID::SID *_sid;

	void sidWrite(int reg, uint8 data);
	void onTimer();

	// State machine ($C801 equivalent)
	uint8 _state;          // 0=off, 1=start, 2=slide

	// Work buffer (copied from SFX data table)
	uint8 _numNotes;
	uint8 _repeatCount;
	uint8 _waveform;
	uint8 _speed;

	// Runtime state
	uint8 _repeatLeft;     // $CE52: remaining sequence repeats
	uint8 _notesLeft;      // $CE55: remaining notes in this pass
	uint8 _freqIndex;      // $CE54: index into freq/delta arrays (by 2)
	uint8 _durIndex;       // $CE53: index into duration array (by 1)
	uint8 _durCounter;     // $CE4F: remaining ticks for current note
	uint8 _speedCounter;   // $CE56: frames until next freq update

	// Current frequency (16-bit)
	uint8 _curFreqLo;      // $CE50
	uint8 _curFreqHi;      // $CE51

	// Precomputed start frequencies and deltas per note
	int16 _startFreqs[16]; // $CE97: starting frequency for each note (lo,hi)
	int16 _deltas[16];     // $CEA6: per-speed-unit frequency delta per note
	uint8 _durCopies[9];   // $CEB5: copy of durations per note

	void silenceV1();
	void silenceAll();
	void setupSfx(int index);
	void tickStart();
	void tickSlide();
};

} // namespace Freescape

#endif // FREESCAPE_DARK_C64_SFX_H
