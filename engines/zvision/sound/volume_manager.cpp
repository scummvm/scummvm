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

#include "common/debug.h"
#include "zvision/detection.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/sound/volume_manager.h"

namespace ZVision {

// Power law with exponent 1.5.
static constexpr uint8 powerLaw[256] = {
  0,  0,  0,  0,  1,  1,  1,  1,  1,  2,  2,  2,  3,  3,  3,  4,
  4,  4,  5,  5,  6,  6,  6,  7,  7,  8,  8,  9,  9, 10, 10, 11,
 11, 12, 12, 13, 14, 14, 15, 15, 16, 16, 17, 18, 18, 19, 20, 20,
 21, 21, 22, 23, 23, 24, 25, 26, 26, 27, 28, 28, 29, 30, 31, 31,
 32, 33, 34, 34, 35, 36, 37, 37, 38, 39, 40, 41, 41, 42, 43, 44,
 45, 46, 46, 47, 48, 49, 50, 51, 52, 53, 53, 54, 55, 56, 57, 58,
 59, 60, 61, 62, 63, 64, 65, 65, 66, 67, 68, 69, 70, 71, 72, 73,
 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 88, 89, 90,
 91, 92, 93, 94, 95, 96, 97, 98, 99,100,102,103,104,105,106,107,
108,109,110,112,113,114,115,116,117,119,120,121,122,123,124,126,
127,128,129,130,132,133,134,135,136,138,139,140,141,142,144,145,
146,147,149,150,151,152,154,155,156,158,159,160,161,163,164,165,
167,168,169,171,172,173,174,176,177,178,180,181,182,184,185,187,
188,189,191,192,193,195,196,197,199,200,202,203,204,206,207,209,
210,211,213,214,216,217,218,220,221,223,224,226,227,228,230,231,
233,234,236,237,239,240,242,243,245,246,248,249,251,252,254,255
};

static constexpr uint8 logPower[256] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
  1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,
  3,  4,  4,  4,  4,  5,  5,  5,  5,  6,  6,  6,  7,  7,  7,  8,
  8,  9,  9, 10, 10, 11, 11, 12, 13, 13, 14, 15, 16, 17, 18, 19,
 20, 21, 22, 23, 24, 26, 27, 29, 30, 32, 34, 36, 38, 40, 42, 45,
 47, 50, 52, 55, 58, 62, 65, 69, 73, 77, 81, 86, 90, 96,101,107,
113,119,126,133,140,148,156,165,174,184,194,205,217,229,241,255
};

// */
static constexpr uint8 logAmplitude[256] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  4,  4,  4,  4,  4,  4,  4,  4,  4,  5,  5,  5,  5,  5,
  5,  5,  5,  6,  6,  6,  6,  6,  6,  7,  7,  7,  7,  7,  8,  8,
  8,  8,  8,  9,  9,  9,  9, 10, 10, 10, 10, 11, 11, 11, 12, 12,
  2, 13, 13, 13, 14, 14, 15, 15, 15, 16, 16, 17, 17, 18, 18, 19,
 19, 20, 20, 21, 21, 22, 23, 23, 24, 24, 25, 26, 27, 27, 28, 29,
 30, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 45,
 46, 47, 48, 50, 51, 52, 54, 55, 57, 58, 60, 62, 63, 65, 67, 69,
 71, 73, 75, 77, 79, 81, 83, 86, 88, 90, 93, 96, 98,101,104,107,
110,113,116,119,122,126,129,133,136,140,144,148,152,156,160,165,
169,174,179,184,189,194,200,205,211,217,222,229,235,241,248,255
};
/*/
// Old system; this is wrong, caused bug #7176; cloister fountain (value 50 in-game, 127/255) inaudible
// Using linear volume served as a temporary fix, but causes other sounds not to play at correct amplitudes (e.g. beehive, door singing in ZGI)
static constexpr uint8 logAmplitude[256] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
  2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,
  4,  4,  4,  5,  5,  5,  5,  5,  6,  6,  6,  6,  7,  7,  7,  7,
  8,  8,  8,  9,  9,  9, 10, 10, 11, 11, 11, 12, 12, 13, 13, 14,
 14, 15, 15, 16, 16, 17, 18, 18, 19, 20, 21, 21, 22, 23, 24, 25,
 26, 27, 28, 29, 30, 31, 32, 33, 34, 36, 37, 38, 40, 41, 43, 45,
 46, 48, 50, 52, 53, 55, 57, 60, 62, 64, 67, 69, 72, 74, 77, 80,
 83, 86, 89, 92, 96, 99,103,107,111,115,119,123,128,133,137,143,
148,153,159,165,171,177,184,191,198,205,212,220,228,237,245,255
};
// */

/*
Estimated relative amplitude of a point sound source as it circles the listener's head from front to rear, due to ear pinna shape.
Maximum attenuation -5dB when fully to rear.  Seems to give a reasonably realistic effect when tested on the Nemesis cloister fountain.
Should be applied AFTER volume profile is applied to script files.
Generating function:
  for 0 < theta < 90, amp = 255;
  for 90 < theta < 180, amp = 255*10^(1-(cos(2*(theta-90))/4))
  where theta is the azimuth, in degrees, of the sound source relative to straight ahead of listener
Source: Own work; crude and naive model that is probably not remotely scientifically accurate, but good enough for a 30-year-old game.
*/
static constexpr uint8 directionalAmplitude[181] = {
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,254,254,253,
252,251,249,248,246,245,243,241,238,236,234,231,228,226,223,220,
217,214,211,208,204,201,198,195,191,188,185,181,178,175,171,168,
165,162,158,155,152,149,146,143,141,138,135,132,130,127,125,122,
120,118,116,113,111,109,108,106,104,102,101, 99, 98, 96, 95, 93,
 92, 91, 90, 89, 88, 87, 86, 85, 85, 84, 83, 83, 82, 82, 82, 81,
 81, 81, 81, 81, 81
};

VolumeManager::VolumeManager(ZVision *engine, volumeScaling mode) :
	_mode(mode) {
}

uint8 VolumeManager::convert(uint8 inputValue) {
	return convert(inputValue, _mode);
}

uint8 VolumeManager::convert(uint8 inputValue, Math::Angle azimuth, uint8 directionality) {
	return convert(inputValue, _mode, azimuth, directionality);
}

uint8 VolumeManager::convert(uint8 inputValue, volumeScaling &mode, Math::Angle azimuth, uint8 directionality) {
	uint8 index = abs(round(azimuth.getDegrees(-180)));
	uint32 output = convert(inputValue, mode);
	uint32 directionalOutput = (output * directionalAmplitude[index]) * directionality;
	directionalOutput /= 0xFF;
	output *= (0xFF - directionality);
	output = (output + directionalOutput) / 0xFF;
	debugC(4, kDebugSound, "Directionally converted output %d", output);
	return output;
}

uint8 VolumeManager::convert(uint8 inputValue, volumeScaling &mode) {
	if (inputValue > _scriptScale)
		inputValue = _scriptScale;
	uint32 scaledInput = inputValue * 0xFF;
	scaledInput /= _scriptScale;
	uint8 output = 0;
	switch (mode) {
	case kVolumeLogPower:
		output = logPower[scaledInput];
		break;
	case kVolumeLogAmplitude:
		output = logAmplitude[scaledInput];
		break;
	case kVolumePowerLaw:
		output = powerLaw[scaledInput];
		break;
	case kVolumeParabolic:
		scaledInput *= scaledInput;
		output = scaledInput / 0xFF;
		break;
	case kVolumeCubic:
		scaledInput *= scaledInput * scaledInput;
		output = scaledInput / 0xFE01;
		break;
	case kVolumeQuartic:
		scaledInput *= scaledInput;
		scaledInput *= scaledInput;
		output = scaledInput / 0xFD02FF;
		break;
	case kVolumeLinear:
	default:
		output = scaledInput;
		break;
	}
	debugC(4, kDebugSound, "Scripted volume %d, scaled volume %d, converted output %d", inputValue, scaledInput, output);
	return output;
}

#if defined(USE_MPEG2) && defined(USE_A52)
double VolumeManager::getVobAmplification(Common::String fileName) const {
	// For some reason, we get much lower volume in the hi-res videos than
	// in the low-res ones. So we artificially boost the volume. This is an
	// approximation, but I've tried to match the old volumes reasonably
	// well.
	//
	// Some of these will cause audio clipping. Hopefully not enough to be
	// noticeable.
	double amplification = 0.0;
	if (fileName == "em00d011.vob") {
		// The finale.
		amplification = 10.0;
	} else if (fileName == "em00d021.vob") {
		// Jack's escape and arrival at Flathead Mesa.
		amplification = 9.0;
	} else if (fileName == "em00d032.vob") {
		// The Grand Inquisitor's speech.
		amplification = 11.0;
	} else if (fileName == "em00d122.vob") {
		// Jack orders you to the radio tower.
		amplification = 17.0;
	} else if (fileName == "em3ed012.vob") {
		// The Grand Inquisitor gets the Coconut of Quendor.
		amplification = 12.0;
	} else if (fileName == "g000d101.vob") {
		// Griff gets captured.
		amplification = 11.0;
	} else if (fileName == "g000d111.vob") {
		// Brog gets totemized. The music seems to be mixed much softer
		// in this than in the low-resolution version.
		amplification = 12.0;
	} else if (fileName == "g000d122.vob") {
		// Lucy gets captured.
		amplification = 14.0;
	} else if (fileName == "g000d302.vob") {
		// The Grand Inquisitor visits Jack in his cell.
		amplification = 13.0;
	} else if (fileName == "g000d312.vob") {
		// You get captured.
		amplification = 14.0;
	} else if (fileName == "g000d411.vob") {
		// Propaganda On Parade. No need to make it as loud as the
		// low-resolution version.
		amplification = 11.0;
	} else if (fileName == "pe1ed012.vob") {
		// Jack lets you in with the lantern.
		amplification = 14.0;
	} else if (fileName.hasPrefix("pe1ed")) {
		// Jack answers the door. Several different ways.
		amplification = 17.0;
	} else if (fileName == "pe5ed052.vob") {
		// You get killed by the guards
		amplification = 12.0;
	} else if (fileName == "pe6ed012.vob") {
		// Jack gets captured by the guards
		amplification = 17.0;
	} else if (fileName == "pp1ed022.vob") {
		// Jack examines the lantern
		amplification = 10.0;
	} else if (fileName == "qb1ed012.vob") {
		// Lucy gets invited to the back room
		amplification = 17.0;
	} else if (fileName.hasPrefix("qe1ed")) {
		// Floyd answers the door. Several different ways.
		amplification = 17.0;
	} else if (fileName == "qs1ed011.vob") {
		// Jack explains the rules of the game.
		amplification = 16.0;
	} else if (fileName == "qs1ed021.vob") {
		// Jack loses the game.
		amplification = 14.0;
	} else if (fileName == "uc1gd012.vob") {
		// Y'Gael appears.
		amplification = 12.0;
	} else if (fileName == "ue1ud012.vob") {
		// Jack gets totemized... or what?
		amplification = 12.0;
	} else if (fileName == "ue2qd012.vob") {
		// Jack agrees to totemization.
		amplification = 10.0;
	} else if (fileName == "g000d981.vob") {
		// The Enterprise logo. Has no low-res version. Its volume is
		// louder than the other logo animations.
		amplification = 6.2;
	} else if (fileName.hasPrefix("g000d")) {
		// The Dolby Digital and Activision logos. They have no low-res
		// versions, but I've used the low-resolution Activision logo
		// (slightly different) as reference.
		amplification = 8.5;
	}
	return amplification;
}
#endif

} // End of namespace ZVision
