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

#ifndef FREESCAPE_DRILLER_C64_SFX_H
#define FREESCAPE_DRILLER_C64_SFX_H

#include "audio/sid.h"
#include "freescape/sid.h"

namespace Freescape {

class DrillerC64SFXPlayer {
public:
	DrillerC64SFXPlayer();
	~DrillerC64SFXPlayer();

	void playSfx(int sfxIndex);
	void sfxTick();           // Called every frame (50Hz) from onTimer
	void stopAllSfx();

	bool isSfxActive() const;
	void initSID();
	void destroySID();

private:
	SID::SID *_sid;

	void sidWrite(int reg, uint8 data);

	// Voice 1 pitch slide state ($CC5B-$CC61)
	uint8 _v1Counter;      // 0xFF=inactive, 0=expired (marked 0xFF next tick)
	uint8 _v1FreqLo;
	uint8 _v1FreqHi;
	uint8 _v1DeltaLo;
	uint8 _v1DeltaHi;
	uint8 _v1TickCtr;
	uint8 _v1TickReload;

	// Voice 3 pitch slide state ($CC62-$CC66)
	uint8 _v3Counter;
	uint8 _v3FreqLo;
	uint8 _v3FreqHi;
	uint8 _v3DeltaLo;
	uint8 _v3DeltaHi;

	// Noise burst timer ($CC67-$CC6A)
	uint8 _noiseTimer;
	uint8 _noiseCounter;
	uint8 _noiseReload;
	uint8 _noiseDec;

	// Phase state machine for multi-step SFX (#6, #14, #15)
	uint8 _sfxPhase;        // 0=inactive
	uint8 _sfxPhaseTimer;   // frames until next phase transition
	uint8 _sfxActiveIndex;  // which SFX owns the phase state

	// Tick handlers
	void tickVoice1Slide();
	void tickVoice3Slide();
	void tickNoiseBurst();
	void tickPhase();

	// Helper to silence all voices
	void silenceAllVoices();

	// Noise burst subroutine ($C818)
	void noiseBurst(uint8 param);

	// Individual SFX routines
	void sfx2();   // Dual-voice noise sweep down
	void sfx3();   // Noise pitch slide
	void sfx4();   // Pulse slide down
	void sfx5();   // Pulse slide up
	void sfx6();   // Triangle blip
	void sfx7();   // Dual noise burst
	void sfx8();   // Triangle slide up
	void sfx9();   // Dual slide (V1+V3) noise
	void sfx10();  // Programmed noise bursts
	void sfx11();  // Triangle slide down (fast)
	void sfx14();  // 3-step chord (V1+V2)
	void sfx15();  // Two-phase pulse effect
	void sfx16();  // Filtered effect (V1+V3)
	void sfx17();  // Freq echo (V1->V3)
	void sfx18();  // Dual noise with modulation

	void onTimer();
};

} // namespace Freescape

#endif // FREESCAPE_DRILLER_C64_SFX_H
