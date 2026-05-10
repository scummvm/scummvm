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

#ifndef FREESCAPE_ECLIPSE_C64_SFX_H
#define FREESCAPE_ECLIPSE_C64_SFX_H

#include "audio/sid.h"
#include "freescape/sid.h"

namespace Freescape {

class EclipseC64SFXPlayer {
public:
	EclipseC64SFXPlayer();
	~EclipseC64SFXPlayer();

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

	uint8 _state;

	uint8 _numNotes;
	uint8 _repeatCount;
	uint8 _waveform;
	uint8 _speed;

	uint8 _repeatLeft;
	uint8 _notesLeft;
	uint8 _freqIndex;
	uint8 _durIndex;
	uint8 _durCounter;
	uint8 _speedCounter;

	uint8 _curFreqLo;
	uint8 _curFreqHi;

	int16 _startFreqs[16];
	int16 _deltas[16];
	uint8 _durCopies[9];

	void silenceV1();
	void silenceAll();
	void setupSfx(int index);
	void tickStart();
	void tickSlide();
};

} // namespace Freescape

#endif // FREESCAPE_ECLIPSE_C64_SFX_H
