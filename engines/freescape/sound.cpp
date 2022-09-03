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

namespace Freescape {

// If step rate for playSoundSweepIncWL calls needs adjusting,
// can tweak wlStepPerMS parameter by the factor below.
const double kFreescapeSweepTuneFactor = 1.0;

void FreescapeEngine::playSound(int index) {
	if (!_mixer->isSoundHandleActive(_handle))
		_mixer->stopHandle(_handle);

	assert(_usePrerecordedSounds);
	debug("Playing sound %d", index);
	switch (index) {
		case 1: // Done
			if (_usePrerecordedSounds) {
				playWav("fsDOS_laserFire.wav");
				//_system->delayMillis(50);
			} else
				playSoundSweepIncWL(1500, 700, 5.46 * kFreescapeSweepTuneFactor, 1);
		break;
		case 2: // Done
			if (_usePrerecordedSounds) {
				playWav("fsDOS_WallBump.wav");
				//_system->delayMillis(50);
			} else {
				playSoundConst(82, 60);
			}
		break;
		case 3:
			if (_usePrerecordedSounds) {
				playWav("fsDOS_stairDown.wav");
				//_system->delayMillis(50);
			} else {
				playSoundConst(220, 50);
				playSoundConst(185, 50);
			}
		break;
		case 4:
			if (_usePrerecordedSounds) {
				playWav("fsDOS_stairUp.wav");
				//_system->delayMillis(50);
			} else {
				playSoundConst(220, 50);
				playSoundConst(340, 50);
			}
		break;
		case 5:
			if (_usePrerecordedSounds) {
				playWav("fsDOS_roomChange.wav");
				//_system->delayMillis(50);
			} else {
				playSoundConst(82, 60);
			}
		break;
		case 6:
			if (_usePrerecordedSounds) {
				playWav("fsDOS_configMenu.wav");
				//_system->delayMillis(50);
			} else {
				playSoundConst(82, 60);
			}
		break;
		case 7:
			if (_usePrerecordedSounds) {
				playWav("fsDOS_bigHit.wav");
				//_system->delayMillis(50);
			} else {
				playSoundConst(82, 60);
			}
		break;
		case 8:
			if (_usePrerecordedSounds) {
				playWav("fsDOS_teleportActivated.wav");
				//_system->delayMillis(50);
			} else {
				playSoundConst(82, 60);
			}
		break;

		case 9:
			if (_usePrerecordedSounds) {
				playWav("fsDOS_powerUp.wav");
				//_system->delayMillis(50);
			} else {
				playSoundConst(82, 60);
			}
		break;

		case 10:
			if (_usePrerecordedSounds) {
				playWav("fsDOS_energyDrain.wav");
				//_system->delayMillis(50);
			} else {
				playSoundConst(82, 60);
			}
		break;

		case 11: // ???
			if (_usePrerecordedSounds) {
				playWav("???.wav");
				//_system->delayMillis(50);
			} else {
				playSoundConst(82, 60);
			}
		break;

		case 12:
			if (_usePrerecordedSounds) {
				playWav("fsDOS_switchOff.wav");
				//_system->delayMillis(50);
			} else {
				playSoundConst(82, 60);
			}
		break;

		case 13: // Seems to be repeated?
			if (_usePrerecordedSounds) {
				playWav("fsDOS_laserHit.wav");
				//_system->delayMillis(50);
			} else {
				playSoundConst(82, 60);
			}
		break;

		case 14:
			if (_usePrerecordedSounds) {
				playWav("fsDOS_tankFall.wav");
				//_system->delayMillis(50);
			} else {
				playSoundConst(82, 60);
			}
		break;

		case 15:
			if (_usePrerecordedSounds) {
				playWav("fsDOS_successJingle.wav");
				//_system->delayMillis(50);
			} else {
				playSoundConst(82, 60);
			}
		break;

		case 16: // Silence?
			if (_usePrerecordedSounds) {
				//playWav("???.wav");
				//_system->delayMillis(50);
			} else {
				playSoundConst(82, 60);
			}
		break;

		case 17:
			if (_usePrerecordedSounds) {
				playWav("fsDOS_badJingle.wav");
				//_system->delayMillis(50);
			} else {
				playSoundConst(82, 60);
			}
		break;

		case 18: // Silence?
			if (_usePrerecordedSounds) {
				//playWav("???.wav");
				//_system->delayMillis(50);
			} else {
				playSoundConst(82, 60);
			}
		break;

		case 19:
			if (_usePrerecordedSounds) {
				playWav("???.wav");
				//_system->delayMillis(50);
			} else {
				playSoundConst(82, 60);
			}
		break;

		case 20:
			if (_usePrerecordedSounds) {
				playWav("fsDOS_bigHit.wav");
				//_system->delayMillis(50);
			} else {
				playSoundConst(82, 60);
			}
		break;
		default:
		error("Unexpected sound %d", index);
		break;
	}
}

void FreescapeEngine::playWav(const Common::String filename) {
	Common::SeekableReadStream *s = _dataBundle->createReadStreamForMember(filename);
	assert(s);
	Audio::AudioStream *stream = Audio::makeWAVStream(s, DisposeAfterUse::YES);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_handle, stream);
}


void FreescapeEngine::playSoundConst(double hzFreq, int duration) {
	Audio::PCSpeaker *speaker = new Audio::PCSpeaker();
	speaker->setVolume(50);
	speaker->play(Audio::PCSpeaker::kWaveFormSquare, hzFreq, duration);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_handle, speaker);
	_system->delayMillis(duration);
	_mixer->stopHandle(_handle);
}


void FreescapeEngine::playSoundSweepIncWL(double hzFreq1, double hzFreq2, double wlStepPerMS, int resolution) {
	// Play a PC speaker sweep between sound frequencies, using constant wavelength increment.

	// The wavelength step-per-milliseconds value, or wlStepPerMS, describes how
	// many PIT counter increments occur per millisecond.  This unusual metric is actually
	// quite efficient when programming an 8086 without floating-point hardware, because
	// the increment between counters sent to hardware can be a constant integer.

	// The msResolution describes the time resolution used between frequency change events.
	// If this is only 1 ms, frequency changes very rapidly (every millisecond).
	// If less resolute, like 100 ms, frequency jumps less often, giving a more "racheted" sweep.

	// The PIT hardware calculates frequencies as 1193180.0 / freq.
	// Because the hardware only takes 16-bit integers as input, this can expect
	// to cover nearly all audible frequencies with decent tonal accuracy.

	// Frequencies that sweep using this algorithm will appear to advance slowly
	// at lower frequencies and quickly at higher frequencies.

	// The exact progression works like this:

	// FreqNext = 1 / (1 / FreqOrig + wlStep / MagicNumber)

	// ...where FreqOrig is the original frequency, FreqNext is the stepped-to frequency,
	// wlStep is the PIT counter step value, and MagicNumber is 1193180.0.

	// Option:  can round wlStep to integer to make more 16-bit-counter authentic.
	double wlStep = wlStepPerMS * (double)resolution;

	// Option:  can round inv1 and inv2 to integer to make more 16-bit-counter authentic.
	double inv1 = 1193180.0 / hzFreq1;
	double inv2 = 1193180.0 / hzFreq2;

	// Set step to correct direction
	if (inv1 < inv2 && wlStep < 0)
		wlStep = -wlStep;
	if (inv1 > inv2 && wlStep > 0)
		wlStep = -wlStep;

	// Loop over frequency range
	int hzCounts = (int)((inv2 - inv1) / wlStep);
	while (hzCounts-- >= 0) {
		playSoundConst((1193180.0 / inv1), resolution);
		inv1 += wlStep;
	}
	_mixer->stopHandle(_handle);
}

}