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

#include "freescape/games/driller/c64.sfx.h"
#include "freescape/freescape.h"

#include "common/debug.h"
#include "common/textconsole.h"

namespace Freescape {

DrillerC64SFXPlayer::DrillerC64SFXPlayer()
	: _sid(nullptr),
	  _v1Counter(0xFF), _v1FreqLo(0), _v1FreqHi(0),
	  _v1DeltaLo(0), _v1DeltaHi(0), _v1TickCtr(0), _v1TickReload(0),
	  _v3Counter(0xFF), _v3FreqLo(0), _v3FreqHi(0),
	  _v3DeltaLo(0), _v3DeltaHi(0),
	  _noiseTimer(0), _noiseCounter(0), _noiseReload(0), _noiseDec(0),
	  _sfxPhase(0), _sfxPhaseTimer(0), _sfxActiveIndex(0) {
	initSID();
}

DrillerC64SFXPlayer::~DrillerC64SFXPlayer() {
	destroySID();
}

void DrillerC64SFXPlayer::destroySID() {
	if (_sid) {
		_sid->stop();
		delete _sid;
		_sid = nullptr;
	}
}

void DrillerC64SFXPlayer::initSID() {
	if (_sid) {
		_sid->stop();
		delete _sid;
	}

	_sid = SID::Config::create(SID::Config::kSidPAL);
	if (!_sid || !_sid->init())
		error("Failed to initialise SID emulator for SFX");

	for (int i = 0; i < 0x18; i++)
		sidWrite(i, 0);
	sidWrite(kSIDVolume, 0x0F);

	_sid->start(new Common::Functor0Mem<void, DrillerC64SFXPlayer>(this, &DrillerC64SFXPlayer::onTimer), 50);
}

void DrillerC64SFXPlayer::sidWrite(int reg, uint8 data) {
	if (_sid) {
		debugC(4, kFreescapeDebugMedia, "SFX SID Write: Reg $%02X = $%02X", reg, data);
		_sid->writeReg(reg, data);
	}
}

void DrillerC64SFXPlayer::onTimer() {
	sfxTick();
}

bool DrillerC64SFXPlayer::isSfxActive() const {
	return (_v1Counter != 0xFF) || (_v3Counter != 0xFF) || (_noiseTimer != 0) || (_sfxPhase != 0);
}

void DrillerC64SFXPlayer::stopAllSfx() {
	_v1Counter = 0xFF;
	_v3Counter = 0xFF;
	_noiseTimer = 0;
	_sfxPhase = 0;
	silenceAllVoices();
}

void DrillerC64SFXPlayer::silenceAllVoices() {
	for (int i = 0; i <= 0x17; i++)
		sidWrite(i, 0);
	sidWrite(kSIDVolume, 0x0F);
}

// --- Noise burst subroutine ($C818) ---
void DrillerC64SFXPlayer::noiseBurst(uint8 param) {
	sidWrite(kSIDV1FreqLo, 0);
	sidWrite(kSIDV1FreqHi, param);
	sidWrite(kSIDV1AD, 0x0A);
	sidWrite(kSIDV1SR, 0x09);
	sidWrite(kSIDV1Ctrl, 0x81); // Noise + gate
}

// --- Tick handler ($C96F) - called every frame (50Hz) ---

void DrillerC64SFXPlayer::sfxTick() {
	tickNoiseBurst();
	tickVoice1Slide();
	tickVoice3Slide();
	tickPhase();
}

void DrillerC64SFXPlayer::tickNoiseBurst() {
	if (_noiseTimer == 0)
		return;

	_noiseCounter--;
	if (_noiseCounter == 0) {
		noiseBurst(_noiseTimer);
		_noiseCounter = _noiseReload;
		_noiseReload--;
		_noiseDec--;
		if (_noiseDec == 0) {
			_noiseTimer = 0;
		}
	}
}

// Tick handler for Voice 1 pitch slide.
// Matches the original 6502 at $C97F:
//   - Check counter==0 BEFORE decrementing (expired last frame → mark 0xFF)
//   - On counter expiry: do NOT silence - ADSR release handles decay
//   - Write freq to SID THEN decrement counter
void DrillerC64SFXPlayer::tickVoice1Slide() {
	if (_v1Counter == 0xFF)
		return;

	// Counter expired last frame - silence V1.
	// Matches the $C950 pattern: write 0 to ctrl (no waveform, no gate).
	// The game is silent between SFX, so the voice must be stopped.
	if (_v1Counter == 0) {
		_v1Counter = 0xFF;
		sidWrite(kSIDV1Ctrl, 0);
		return;
	}

	// Tick prescaler: skip frames until prescaler expires
	if (_v1TickCtr != 0) {
		_v1TickCtr--;
		return;
	}
	_v1TickCtr = _v1TickReload;

	// 16-bit frequency addition (CLC; ADC)
	uint16 freq = (_v1FreqHi << 8) | _v1FreqLo;
	uint16 delta = (_v1DeltaHi << 8) | _v1DeltaLo;
	freq += delta;
	_v1FreqLo = freq & 0xFF;
	_v1FreqHi = (freq >> 8) & 0xFF;

	// Write to SID FIRST
	sidWrite(kSIDV1FreqLo, _v1FreqLo);
	sidWrite(kSIDV1FreqHi, _v1FreqHi);

	// THEN decrement counter
	_v1Counter--;
}

// Tick handler for Voice 3 pitch slide (no tick prescaler).
void DrillerC64SFXPlayer::tickVoice3Slide() {
	if (_v3Counter == 0xFF)
		return;

	if (_v3Counter == 0) {
		_v3Counter = 0xFF;
		return;
	}

	uint16 freq = (_v3FreqHi << 8) | _v3FreqLo;
	uint16 delta = (_v3DeltaHi << 8) | _v3DeltaLo;
	freq += delta;
	_v3FreqLo = freq & 0xFF;
	_v3FreqHi = (freq >> 8) & 0xFF;

	sidWrite(kSIDV3FreqLo, _v3FreqLo);
	sidWrite(kSIDV3FreqHi, _v3FreqHi);

	_v3Counter--;
}

// Phase state machine for multi-step SFX (#6, #14, #15).
// Handles gate-off timing and chord/phase transitions that the
// original implements via busy-wait loops.
void DrillerC64SFXPlayer::tickPhase() {
	if (_sfxPhase == 0)
		return;

	if (_sfxPhaseTimer > 0) {
		_sfxPhaseTimer--;
		if (_sfxPhaseTimer > 0)
			return;
	}

	// Timer expired - handle phase transition
	switch (_sfxActiveIndex) {
	case 6:
		// SFX #6: gate off after 1 frame
		if (_sfxPhase == 1) {
			sidWrite(kSIDV1Ctrl, 0x10); // Triangle, gate off
			_sfxPhase = 0;
		}
		break;

	case 14:
		// SFX #14: 3-step chord using inline data from $C7AA
		// V1 freqHi: $22, $22, $39
		// V2 freqHi: $26, $30, $9A
		// Timing:    $0C, $08, $1E
		switch (_sfxPhase) {
		case 1: // Step 1 done → Step 2
			sidWrite(kSIDV1FreqHi, 0x22);
			sidWrite(kSIDV2FreqHi, 0x30);
			_sfxPhase = 2;
			_sfxPhaseTimer = 8; // $08 frames
			break;
		case 2: // Step 2 done → Step 3
			sidWrite(kSIDV1FreqHi, 0x39);
			sidWrite(kSIDV2FreqHi, 0x9A);
			_sfxPhase = 3;
			_sfxPhaseTimer = 30; // $1E frames
			break;
		case 3: // Step 3 done → gate off
			sidWrite(kSIDV1Ctrl, 0x46); // Pulse+sync, gate off
			sidWrite(kSIDV2Ctrl, 0x46);
			_sfxPhase = 0;
			break;
		default:
			_sfxPhase = 0;
			break;
		}
		break;

	case 15:
		// SFX #15: two-phase pulse effect
		switch (_sfxPhase) {
		case 1: // Phase 1 slide done → gate off, short wait
			sidWrite(kSIDV1Ctrl, 0x40); // Pulse, gate off
			_sfxPhase = 2;
			_sfxPhaseTimer = 3; // Approximate busy-wait delay
			break;
		case 2: // Wait done → Phase 2: new slide params
			_v1FreqLo = 0x00;
			_v1FreqHi = 0x08;
			_v1DeltaLo = 0x00;
			_v1DeltaHi = 0x00;
			_v1Counter = 6;
			_v1TickCtr = 1;
			_v1TickReload = 8;
			sidWrite(kSIDV1FreqLo, 0x00);
			sidWrite(kSIDV1FreqHi, 0x08);
			sidWrite(kSIDV1Ctrl, 0x41); // Pulse + gate
			_sfxPhase = 0; // V1 slide handler takes over
			break;
		default:
			_sfxPhase = 0;
			break;
		}
		break;

	default:
		_sfxPhase = 0;
		break;
	}
}

// --- SFX dispatch ---

void DrillerC64SFXPlayer::playSfx(int sfxIndex) {
	debugC(1, kFreescapeDebugMedia, "DrillerC64SFX: Playing SFX %d", sfxIndex);

	// Stop any ongoing SFX state before starting new one
	_v1Counter = 0xFF;
	_v3Counter = 0xFF;
	_noiseTimer = 0;
	_sfxPhase = 0;

	// Reset all SID registers to a clean state.
	// Music and SFX are mutually exclusive on C64; during gameplay only
	// SFX play and the game is silent between them. The SID starts from
	// a zeroed state (music stopped at game start), and many SFX routines
	// don't write all registers (e.g. SFX #2 never writes V1 SR). Without
	// this reset, stale values from a previous SFX (e.g. SR=$F8 from #8)
	// would cause subsequent SFX to sustain indefinitely.
	for (int i = 0; i < 0x18; i++)
		sidWrite(i, 0);

	switch (sfxIndex) {
	case 2:
		sfx2();
		break;
	case 3:
		sfx3();
		break;
	case 4:
		sfx4();
		break;
	case 5:
		sfx5();
		break;
	case 6:
		sfx6();
		break;
	case 7:
		sfx7();
		break;
	case 8:
		sfx8();
		break;
	case 9:
		sfx9();
		break;
	case 10:
		sfx10();
		break;
	case 11:
		sfx11();
		break;
	case 14:
		sfx14();
		break;
	case 15:
		sfx15();
		break;
	case 16:
		sfx16();
		break;
	case 17:
		sfx17();
		break;
	case 18:
		sfx18();
		break;
	default:
		debugC(1, kFreescapeDebugMedia, "DrillerC64SFX: Unknown SFX index %d", sfxIndex);
		break;
	}
}

// --- Individual SFX routines ---
// All verified against C64 binary (driller.bin, PRG load $0400)

// SFX #2 ($C55A) - Dual-Voice Noise Sweep Down
void DrillerC64SFXPlayer::sfx2() {
	sidWrite(kSIDVolume, 0x0F);

	// V1: waveform $15 (triangle+ring+gate), AD=$0B
	sidWrite(kSIDV1AD, 0x0B);
	sidWrite(kSIDV1Ctrl, 0x15);
	// V1 slide: start $4800, delta $FF00, 48 frames, no prescaler
	_v1FreqLo = 0x00;
	_v1FreqHi = 0x48;
	_v1DeltaLo = 0x00;
	_v1DeltaHi = 0xFF;
	_v1Counter = 48;
	_v1TickCtr = 0;
	_v1TickReload = 0;
	sidWrite(kSIDV1FreqLo, _v1FreqLo);
	sidWrite(kSIDV1FreqHi, _v1FreqHi);

	// V2: waveform $15, AD=$0B, SR=$09, freq=$0600
	sidWrite(kSIDV2FreqLo, 0x00);
	sidWrite(kSIDV2FreqHi, 0x06);
	sidWrite(kSIDV2AD, 0x0B);
	sidWrite(kSIDV2SR, 0x09);
	sidWrite(kSIDV2Ctrl, 0x15);

	// V3: freq=$0C00, no delta, 48 frames (ring modulation source)
	_v3FreqLo = 0x00;
	_v3FreqHi = 0x0C;
	_v3DeltaLo = 0x00;
	_v3DeltaHi = 0x00;
	_v3Counter = 48;
	sidWrite(kSIDV3FreqLo, _v3FreqLo);
	sidWrite(kSIDV3FreqHi, _v3FreqHi);
}

// SFX #3 ($C5A6) - Noise Pitch Slide
void DrillerC64SFXPlayer::sfx3() {
	sidWrite(kSIDVolume, 0x0F);

	// V1: waveform $15, AD=$0F, no prescaler
	sidWrite(kSIDV1AD, 0x0F);
	sidWrite(kSIDV1Ctrl, 0x15);
	_v1FreqLo = 0x00;
	_v1FreqHi = 0xAF;
	_v1DeltaLo = 0x00;
	_v1DeltaHi = 0xF7;
	_v1Counter = 12;
	_v1TickCtr = 0;
	_v1TickReload = 0;
	sidWrite(kSIDV1FreqLo, _v1FreqLo);
	sidWrite(kSIDV1FreqHi, _v1FreqHi);

	// V3: freq=$2000, no delta, 12 frames
	_v3FreqLo = 0x00;
	_v3FreqHi = 0x20;
	_v3DeltaLo = 0x00;
	_v3DeltaHi = 0x00;
	_v3Counter = 12;
	sidWrite(kSIDV3FreqLo, _v3FreqLo);
	sidWrite(kSIDV3FreqHi, _v3FreqHi);
}

// SFX #4 ($C5E3) - Pulse Slide Down
void DrillerC64SFXPlayer::sfx4() {
	sidWrite(kSIDVolume, 0x0F);

	sidWrite(kSIDV1PwLo, 0xB4);
	sidWrite(kSIDV1PwHi, 0x0E);
	sidWrite(kSIDV1AD, 0x2F);
	sidWrite(kSIDV1SR, 0xF8);
	sidWrite(kSIDV1Ctrl, 0x41);
	_v1FreqLo = 0x00;
	_v1FreqHi = 0x14;
	_v1DeltaLo = 0x00;
	_v1DeltaHi = 0xFD;
	_v1Counter = 2;
	_v1TickCtr = 1;       // Prescaler: tick every 7 frames
	_v1TickReload = 6;
	sidWrite(kSIDV1FreqLo, _v1FreqLo);
	sidWrite(kSIDV1FreqHi, _v1FreqHi);
}

// SFX #5 ($C62A) - Pulse Slide Up
void DrillerC64SFXPlayer::sfx5() {
	sidWrite(kSIDVolume, 0x0F);

	sidWrite(kSIDV1PwLo, 0xB4);
	sidWrite(kSIDV1PwHi, 0x0E);
	sidWrite(kSIDV1AD, 0x2F);
	sidWrite(kSIDV1SR, 0xF8);
	sidWrite(kSIDV1Ctrl, 0x41);
	_v1FreqLo = 0x00;
	_v1FreqHi = 0x10;
	_v1DeltaLo = 0x00;
	_v1DeltaHi = 0x03;
	_v1Counter = 2;
	_v1TickCtr = 1;       // Prescaler: tick every 7 frames
	_v1TickReload = 6;
	sidWrite(kSIDV1FreqLo, _v1FreqLo);
	sidWrite(kSIDV1FreqHi, _v1FreqHi);
}

// SFX #6 ($C681) - Triangle Blip
// Original: gate on, ~250-cycle busy loop (~0.25ms), gate off.
// We use the phase system to gate off after 1 frame.
void DrillerC64SFXPlayer::sfx6() {
	sidWrite(kSIDVolume, 0x0F);

	sidWrite(kSIDV1AD, 0x1F);
	sidWrite(kSIDV1SR, 0xF8);
	sidWrite(kSIDV1FreqLo, 0x00);
	sidWrite(kSIDV1FreqHi, 0x77);
	sidWrite(kSIDV1Ctrl, 0x11); // Triangle + gate on

	_sfxActiveIndex = 6;
	_sfxPhase = 1;
	_sfxPhaseTimer = 1;
}

// SFX #7 ($C6B0) - Dual Noise Burst (one-shot)
void DrillerC64SFXPlayer::sfx7() {
	sidWrite(kSIDVolume, 0x0F);

	sidWrite(kSIDV1FreqLo, 0x00);
	sidWrite(kSIDV1FreqHi, 0x01);
	sidWrite(kSIDV1AD, 0x0A);
	sidWrite(kSIDV1SR, 0x09);
	sidWrite(kSIDV1Ctrl, 0x81);

	sidWrite(kSIDV2FreqLo, 0x00);
	sidWrite(kSIDV2FreqHi, 0x06);
	sidWrite(kSIDV2AD, 0x0A);
	sidWrite(kSIDV2SR, 0x09);
	sidWrite(kSIDV2Ctrl, 0x81);
}

// SFX #8 ($C6DB) - Triangle Slide Up
void DrillerC64SFXPlayer::sfx8() {
	sidWrite(kSIDVolume, 0x0F);

	sidWrite(kSIDV1AD, 0x1F);
	sidWrite(kSIDV1SR, 0xF8);
	sidWrite(kSIDV1Ctrl, 0x11);
	_v1FreqLo = 0x00;
	_v1FreqHi = 0x1E;
	_v1DeltaLo = 0x00;
	_v1DeltaHi = 0x03;
	_v1Counter = 35;
	_v1TickCtr = 0;
	_v1TickReload = 0;
	sidWrite(kSIDV1FreqLo, _v1FreqLo);
	sidWrite(kSIDV1FreqHi, _v1FreqHi);
}

// SFX #9 ($C70F) - Dual Slide (V1+V3)
void DrillerC64SFXPlayer::sfx9() {
	sidWrite(kSIDVolume, 0x0F);

	sidWrite(kSIDV1AD, 0x0F);
	sidWrite(kSIDV1Ctrl, 0x15);
	_v1FreqLo = 0x00;
	_v1FreqHi = 0x60;
	_v1DeltaLo = 0x00;
	_v1DeltaHi = 0xF7;
	_v1Counter = 25;
	_v1TickCtr = 0;
	_v1TickReload = 0;
	sidWrite(kSIDV1FreqLo, _v1FreqLo);
	sidWrite(kSIDV1FreqHi, _v1FreqHi);

	// V3: freq=$5000, delta=$0000 (no slide), 25 frames
	_v3FreqLo = 0x00;
	_v3FreqHi = 0x50;
	_v3DeltaLo = 0x00;
	_v3DeltaHi = 0x00;  // Fixed: was 0x07
	_v3Counter = 25;
	sidWrite(kSIDV3FreqLo, _v3FreqLo);
	sidWrite(kSIDV3FreqHi, _v3FreqHi);
}

// SFX #10 ($C74C) - Programmed Noise Bursts
void DrillerC64SFXPlayer::sfx10() {
	sidWrite(kSIDVolume, 0x0F);

	_noiseTimer = 13;
	_noiseReload = 6;
	_noiseCounter = 6;   // Fixed: was 1 (first burst after 6 frames, not immediately)
	_noiseDec = 5;
}

// SFX #11 ($C766) - Triangle Slide Down Fast
void DrillerC64SFXPlayer::sfx11() {
	sidWrite(kSIDVolume, 0x0F);

	sidWrite(kSIDV1AD, 0x1F);
	sidWrite(kSIDV1SR, 0xF8);
	sidWrite(kSIDV1Ctrl, 0x11);
	_v1FreqLo = 0x00;
	_v1FreqHi = 0x46;
	_v1DeltaLo = 0x00;
	_v1DeltaHi = 0xFE;
	_v1Counter = 30;
	_v1TickCtr = 0;
	_v1TickReload = 0;
	sidWrite(kSIDV1FreqLo, _v1FreqLo);
	sidWrite(kSIDV1FreqHi, _v1FreqHi);
}

// SFX #14 ($C7B3) - 3-Step Chord (V1+V2)
// Inline data at $C7AA: V1 freqHi {$22,$22,$39}, V2 freqHi {$26,$30,$9A},
// timing {$0C,$08,$1E}. Original blocks game loop via busy-wait.
void DrillerC64SFXPlayer::sfx14() {
	sidWrite(kSIDVolume, 0x0F);

	sidWrite(kSIDV1PwLo, 0xB4);
	sidWrite(kSIDV1PwHi, 0x0E);
	sidWrite(kSIDV1AD, 0x09);
	sidWrite(kSIDV1SR, 0x00);

	sidWrite(kSIDV2PwLo, 0xB4);
	sidWrite(kSIDV2PwHi, 0x0E);
	sidWrite(kSIDV2AD, 0x09);
	sidWrite(kSIDV2SR, 0x00);

	// Step 1: V1=$2200, V2=$2600
	sidWrite(kSIDV1FreqLo, 0x00);
	sidWrite(kSIDV1FreqHi, 0x22);
	sidWrite(kSIDV2FreqLo, 0x00);
	sidWrite(kSIDV2FreqHi, 0x26);
	sidWrite(kSIDV1Ctrl, 0x47); // Pulse + sync + gate
	sidWrite(kSIDV2Ctrl, 0x47);

	_sfxActiveIndex = 14;
	_sfxPhase = 1;
	_sfxPhaseTimer = 12; // $0C frames
}

// SFX #15 ($C810) - Two-Phase Pulse Effect
void DrillerC64SFXPlayer::sfx15() {
	sidWrite(kSIDVolume, 0x0F);

	// Phase 1: pulse slide
	sidWrite(kSIDV1PwLo, 0xB4);
	sidWrite(kSIDV1PwHi, 0x32);
	sidWrite(kSIDV1AD, 0x2F);
	sidWrite(kSIDV1SR, 0xF8);
	sidWrite(kSIDV1Ctrl, 0x41);

	_v1FreqLo = 0x00;
	_v1FreqHi = 0x0C;
	_v1DeltaLo = 0x03;   // Fixed: was 0x00 (delta is $0003, not $0300)
	_v1DeltaHi = 0x00;   // Fixed: was 0x03
	_v1Counter = 11;
	_v1TickCtr = 1;       // Fixed: was 0
	_v1TickReload = 1;    // Fixed: was 0 (slide every 2 frames)
	sidWrite(kSIDV1FreqLo, _v1FreqLo);
	sidWrite(kSIDV1FreqHi, _v1FreqHi);

	// Phase system: 11 iterations × 2 frames/iteration = 22 frames
	_sfxActiveIndex = 15;
	_sfxPhase = 1;
	_sfxPhaseTimer = 23;  // Wait for slide to finish + 1 frame margin
}

// SFX #16 ($C896) - Filtered Effect (one-shot, no slide)
void DrillerC64SFXPlayer::sfx16() {
	// V1: waveform $15, freq=$080A, AD=$0A, SR=$09
	sidWrite(kSIDV1FreqLo, 0x0A);
	sidWrite(kSIDV1FreqHi, 0x08);
	sidWrite(kSIDV1AD, 0x0A);
	sidWrite(kSIDV1SR, 0x09);
	sidWrite(kSIDV1Ctrl, 0x15);

	// V3: freq=$010A (ring modulation source)
	sidWrite(kSIDV3FreqLo, 0x0A);
	sidWrite(kSIDV3FreqHi, 0x01);

	// Filter: cutoff=$780, filter V1, LP enable
	sidWrite(kSIDFilterLo, 0x00);
	sidWrite(kSIDFilterHi, 0x78);
	sidWrite(kSIDFilterCtrl, 0x01);
	sidWrite(kSIDVolume, 0x1F);   // Vol=$0F + LP filter enable bit
}

// SFX #17 ($C8C8) - Freq Echo (one-shot, no slide)
void DrillerC64SFXPlayer::sfx17() {
	sidWrite(kSIDVolume, 0x0F);

	// V1: waveform $15, freq=$001D, AD=$DD
	sidWrite(kSIDV1FreqLo, 0x1D);
	sidWrite(kSIDV1FreqHi, 0x00);
	sidWrite(kSIDV1AD, 0xDD);
	sidWrite(kSIDV1Ctrl, 0x15);

	// V3: original reads V1.FreqHi ($D401) → V3.FreqLo, V3.FreqHi=$02
	// Since V1.FreqHi is $00, V3 freq = $0200
	sidWrite(kSIDV3FreqLo, 0x00);
	sidWrite(kSIDV3FreqHi, 0x02);
}

// SFX #18 ($C8EF) - Dual Noise with Modulation (major explosion)
void DrillerC64SFXPlayer::sfx18() {
	sidWrite(kSIDVolume, 0x0F);

	// V1: waveform $15, AD=$0B
	sidWrite(kSIDV1AD, 0x0B);
	sidWrite(kSIDV1Ctrl, 0x15);

	// V2: waveform $15, AD=$0B, SR=$09, freq=$0600
	sidWrite(kSIDV2FreqLo, 0x00);
	sidWrite(kSIDV2FreqHi, 0x06);
	sidWrite(kSIDV2AD, 0x0B);
	sidWrite(kSIDV2SR, 0x09);
	sidWrite(kSIDV2Ctrl, 0x15);

	// V1 slide: start $C8C8, delta $00F0, 10 frames, prescaler=6
	_v1FreqLo = 0xC8;
	_v1FreqHi = 0xC8;
	_v1DeltaLo = 0xF0;   // Fixed: was 0x00 (delta is $00F0, not $F000)
	_v1DeltaHi = 0x00;   // Fixed: was 0xF0
	_v1Counter = 10;
	_v1TickCtr = 1;       // Fixed: was 0
	_v1TickReload = 6;
	sidWrite(kSIDV1FreqLo, _v1FreqLo);
	sidWrite(kSIDV1FreqHi, _v1FreqHi);

	// V3: freq=$07C8 (ring modulation source), no delta, 10 frames
	_v3FreqLo = 0xC8;
	_v3FreqHi = 0x07;    // Fixed: was 0x00
	_v3DeltaLo = 0x00;
	_v3DeltaHi = 0x00;
	_v3Counter = 10;
	sidWrite(kSIDV3FreqLo, _v3FreqLo);
	sidWrite(kSIDV3FreqHi, _v3FreqHi);
}

} // namespace Freescape
