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

namespace Freescape {

// SID register offsets (shared with Driller SFX player)
enum DarkSIDRegs {
	kDarkSIDV1FreqLo  = 0x00,
	kDarkSIDV1FreqHi  = 0x01,
	kDarkSIDV1PwLo    = 0x02,
	kDarkSIDV1PwHi    = 0x03,
	kDarkSIDV1Ctrl    = 0x04,
	kDarkSIDV1AD      = 0x05,
	kDarkSIDV1SR      = 0x06,

	kDarkSIDV2FreqLo  = 0x07,
	kDarkSIDV2FreqHi  = 0x08,
	kDarkSIDV2PwLo    = 0x09,
	kDarkSIDV2PwHi    = 0x0A,
	kDarkSIDV2Ctrl    = 0x0B,
	kDarkSIDV2AD      = 0x0C,
	kDarkSIDV2SR      = 0x0D,

	kDarkSIDV3FreqLo  = 0x0E,
	kDarkSIDV3FreqHi  = 0x0F,
	kDarkSIDV3PwLo    = 0x10,
	kDarkSIDV3PwHi    = 0x11,
	kDarkSIDV3Ctrl    = 0x12,
	kDarkSIDV3AD      = 0x13,
	kDarkSIDV3SR      = 0x14,

	kDarkSIDFilterLo  = 0x15,
	kDarkSIDFilterHi  = 0x16,
	kDarkSIDFilterCtrl = 0x17,
	kDarkSIDVolume    = 0x18
};

// 40-byte SFX descriptor from the data table at $C802 in dark2.prg
struct DarkSideSFXData {
	uint8 numNotes;        // Number of frequency transitions
	uint8 repeatCount;     // Times to replay the full sequence
	uint8 reserved;
	uint8 freqWaypoints[20]; // Up to 10 frequency waypoints (lo,hi pairs)
	uint8 padding;         // Offset 23
	uint8 durations[9];    // Duration for each transition (in speed units)
	uint8 speed;           // Frames per speed unit
	uint8 padding2;        // Offset 34
	uint8 pwLo;            // Pulse Width low byte
	uint8 pwHi;            // Pulse Width high byte
	uint8 waveform;        // SID control register (gate bit managed separately)
	uint8 attackDecay;     // SID Attack/Decay register
	uint8 sustainRelease;  // SID Sustain/Release register
};

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
