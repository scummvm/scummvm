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

#include "freescape/freescape.h"
#include "freescape/games/driller/driller.h"

namespace Freescape {

void DrillerEngine::playSoundZX(int index, Audio::SoundHandle &handle) {
	debugC(1, kFreescapeDebugMedia, "Playing Driller ZX sound %d", index);
	Common::Array<soundUnitZX> soundUnits;

	auto addTone = [&](uint16 hl, uint16 de, float multiplier) {
		soundUnitZX s;
		s.isRaw = false;
		s.tStates = hl; // HL determines period
		s.freqTimesSeconds = de; // DE determines duration (number of cycles)
		s.multiplier = multiplier;
		soundUnits.push_back(s);
	};

	// Linear Sweep: Period increases -> Pitch decreases
	auto addSweep = [&](uint16 startHl, uint16 endHl, uint16 step, uint16 duration) {
		for (uint16 hl = startHl; hl < endHl; hl += step) {
			addTone(hl, duration, 10.0f);
		}
	};

	// Zap effect: Decreasing Period (E decrements) -> Pitch increases
	auto addZap = [&](uint16 startE, uint16 endE, uint16 duration) {
		for (uint16 e = startE; e > endE; e--) {
			// Map E (delay loops) to HL (tStates)
			// Small E -> Short Period -> High Freq
			uint16 hl = (24 + e) * 4;
			addTone(hl, duration, 10.0f);
		}
	};

	// Sweep Down: Increasing Period (E increments) -> Pitch decreases
	auto addSweepDown = [&](uint16 startE, uint16 endE, uint16 step, uint16 duration, float multiplier) {
		for (uint16 e = startE; e < endE; e += step) {
			uint16 hl = (24 + e) * 4;
			addTone(hl, duration, multiplier);
		}
	};

	switch (index) {
	case 1: // Shoot (FUN_95A1 -> 95AF)
		// Laser: High Pitch -> Low Pitch
		// Adjusted pitch to be even lower (0x200-0x600 is approx 850Hz-280Hz)
		addSweepDown(0x200, 0x600, 20, 1, 2.0f);
		break;
	case 2: // Collide/Bump (FUN_95DE)
		// Low tone sequence
		addTone(0x93c, 0x40, 10.0f); // 64 cycles ~340ms
		addTone(0x7a6, 0x30, 10.0f); // 48 cycles
		break;
	case 3: // Step (FUN_95E5)
		// Short blip
		// Increased duration significantly again (0xC0 = 192 cycles)
		addTone(0x7a6, 0xC0, 10.0f);
		break;
	case 4: // Silence (FUN_95F7)
		break;
	case 5: // Area Change? (FUN_95F8)
		addTone(0x1f0, 0x60, 10.0f); // High pitch, longer
		break;
	case 6: // Menu (Silence?) (FUN_9601)
		break;
	case 7: // Hit? (Sweep FUN_9605)
		// Sweep down (Period increases)
		addSweep(0x200, 0xC00, 64, 2);
		break;
	case 8: // Zap (FUN_961F)
		// Zap: Low -> High
		addZap(0xFF, 0x10, 2);
		break;
	case 9: // Sweep (FUN_9673)
		addSweep(0x100, 0x600, 16, 4);
		break;
	case 10: // Area Change (FUN_9696)
		addSweep(0x100, 0x500, 16, 4);
		break;
	case 11: // Explosion (FUN_96B9)
		{
			soundUnitZX s;
			s.isRaw = true;
			s.rawFreq = 0.0f; // Noise
			s.rawLengthus = 100000; // 100ms noise
			soundUnits.push_back(s);
		}
		break;
	case 12: // Sweep Down (FUN_96E4)
		addSweepDown(0x01, 0xFF, 1, 2, 10.0f);
		break;
	case 13: // Fall? (FUN_96FD)
		addSweep(300, 800, 16, 2);
		break;
	default:
		debugC(1, kFreescapeDebugMedia, "Unknown Driller ZX sound %d", index);
		break;
	}

	FreescapeEngine::playSoundZX(&soundUnits, handle);
}

} // namespace Freescape
