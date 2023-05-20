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

#include "common/file.h"
#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

#include "freescape/freescape.h"

namespace Freescape {

// If step rate for playSoundSweepIncWL calls needs adjusting,
// can tweak wlStepPerMS parameter by the factor below.
const double kFreescapeSweepTuneFactor = 10.0;

void FreescapeEngine::playSound(int index, bool sync) {
	debugC(1, kFreescapeDebugMedia, "Playing sound %d with sync: %d", index, sync);
	if (isAmiga() || isAtariST()) {
		playSoundFx(index, sync);
		_syncSound = sync;
		return;
	}
	waitForSounds();
	switch (index) {
	case 1:
		if (_usePrerecordedSounds) {
			playWav("fsDOS_laserFire.wav");
			//_system->delayMillis(50);
		} else
			playSoundSweepIncWL(1500, 700, 5.46 * kFreescapeSweepTuneFactor, 1, sync);
		break;
	case 2: // Done
		if (_usePrerecordedSounds) {
			playWav("fsDOS_WallBump.wav");
			//_system->delayMillis(50);
		} else {
			playSoundConst(82, 60, sync);
		}
		break;
	case 3:
		if (_usePrerecordedSounds) {
			playWav("fsDOS_stairDown.wav");
			//_system->delayMillis(50);
		} else {
			queueSoundConst(220, 50);
			playSoundConst(185, 50, sync);
		}
		break;
	case 4:
		if (_usePrerecordedSounds) {
			playWav("fsDOS_stairUp.wav");
			//_system->delayMillis(50);
		} else {
			queueSoundConst(220, 50);
			playSoundConst(340, 50, sync);
		}
		break;
	case 5:
		if (_usePrerecordedSounds) {
			playWav("fsDOS_roomChange.wav");
			//_system->delayMillis(50);
		} else {
			playSoundSweepIncWL(262, 100, 65.52 * kFreescapeSweepTuneFactor, 1, sync);
		}
		break;
	case 6:
		if (_usePrerecordedSounds) {
			playWav("fsDOS_configMenu.wav");
			//_system->delayMillis(50);
		} else {
			playSoundConst(830, 60, sync);
		}
		break;
	case 7:
		if (_usePrerecordedSounds) {
			playWav("fsDOS_bigHit.wav");
			//_system->delayMillis(50);
		} else {
			playSoundSweepIncWL(3000, 155, 7.28 * kFreescapeSweepTuneFactor, 1, sync);
		}
		break;
	case 8:
		if (_usePrerecordedSounds) {
			playWav("fsDOS_teleporterActivated.wav");
			//_system->delayMillis(50);
		} else {
			playTeleporter(22, sync);
		}
		break;

	case 9:
		if (_usePrerecordedSounds) {
			playWav("fsDOS_powerUp.wav");
			//_system->delayMillis(50);
		} else {
			playSoundSweepIncWL(280, 5000, 9.1 * kFreescapeSweepTuneFactor, 1, sync);
		}
		break;

	case 10:
		if (_usePrerecordedSounds) {
			playWav("fsDOS_energyDrain.wav");
			//_system->delayMillis(50);
		} else {
			playSoundSweepIncWL(240, 255, 1.82 * kFreescapeSweepTuneFactor, 1, sync);
		}
		break;

	case 11: // ???
		debugC(1, kFreescapeDebugMedia, "Playing unknown sound");
		if (_usePrerecordedSounds) {
			// TODO
		} else {
			// TODO
		}
		break;

	case 12:
		if (_usePrerecordedSounds) {
			playWav("fsDOS_switchOff.wav");
			//_system->delayMillis(50);
		} else {
			playSoundSweepIncWL(555, 440, 1.82 * kFreescapeSweepTuneFactor, 1, sync);
		}
		break;

	case 13: // Seems to be repeated?
		if (_usePrerecordedSounds) {
			playWav("fsDOS_laserHit.wav");
			//_system->delayMillis(50);
		} else {
			playSoundSweepIncWL(3000, 420, 14.56 * kFreescapeSweepTuneFactor, 1, sync);
		}
		break;

	case 14:
		if (_usePrerecordedSounds) {
			playWav("fsDOS_tankFall.wav");
			//_system->delayMillis(50);
		} else {
			playSoundSweepIncWL(785, 310, 1.82 * kFreescapeSweepTuneFactor, 1, sync);
		}
		break;

	case 15:
		if (_usePrerecordedSounds) {
			playWav("fsDOS_successJingle.wav");
			//_system->delayMillis(50);
		} else {
			queueSoundConst(587.330, 250);
			queueSoundConst(740, 175);
			playSoundConst(880, 450, sync);
		}
		break;

	case 16: // Silence?
		if (_usePrerecordedSounds) {
			// TODO
		} else {
			playSilence(1, sync);
		}
		break;

	case 17:
		if (_usePrerecordedSounds) {
			playWav("fsDOS_badJingle.wav");
			//_system->delayMillis(50);
		} else {
			queueSoundConst(65, 150);
			playSoundConst(44, 400, sync);
		}
		break;

	case 18: // Silence?
		if (_usePrerecordedSounds) {
			// TODO
		} else {
			// TODO
		}
		break;

	case 19:
		debugC(1, kFreescapeDebugMedia, "Playing unknown sound");
		if (_usePrerecordedSounds) {
			// TODO
		} else {
			// TODO
		}
		break;

	case 20:
		if (_usePrerecordedSounds) {
			playWav("fsDOS_bigHit.wav");
			//_system->delayMillis(50);
		} else {
			playSoundSweepIncWL(3000, 155, 7.28 * kFreescapeSweepTuneFactor, 1, sync);
		}
		break;
	default:
		debugC(1, kFreescapeDebugMedia, "Unexpected sound %d", index);
		break;
	}
	_syncSound = sync;
}
void FreescapeEngine::playWav(const Common::String filename) {

	Common::SeekableReadStream *s = _dataBundle->createReadStreamForMember(filename);
	assert(s);
	Audio::AudioStream *stream = Audio::makeWAVStream(s, DisposeAfterUse::YES);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundFxHandle, stream);
}

void FreescapeEngine::playMusic(const Common::String filename) {
	Audio::SeekableAudioStream *stream = nullptr;
	stream = Audio::SeekableAudioStream::openStreamFile(filename);
	if (stream) {
		Audio::LoopingAudioStream *loop = new Audio::LoopingAudioStream(stream, 0);
		_mixer->playStream(Audio::Mixer::kMusicSoundType, &_musicHandle, loop);
		_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, Audio::Mixer::kMaxChannelVolume / 10);
	}
}

void FreescapeEngine::playSoundFx(int index, bool sync) {
	if (_soundsFx.size() == 0) {
		debugC(1, kFreescapeDebugMedia, "WARNING: Sounds are not loaded");
		return;
	}

	int size = _soundsFx[index]->size;
	int sampleRate = _soundsFx[index]->sampleRate;
	byte *data = _soundsFx[index]->data;
	int loops = 1;

	if (index == 10)
		loops = 5;
	else if (index == 15)
		loops = 50;

	if (size > 4) {
		Audio::SeekableAudioStream *s = Audio::makeRawStream(data, size, sampleRate, Audio::FLAG_16BITS, DisposeAfterUse::NO);
		Audio::AudioStream *stream = new Audio::LoopingAudioStream(s, loops);
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundFxHandle, stream);
	}
}

void FreescapeEngine::stopAllSounds() {
	_speaker->stop();
	_mixer->stopHandle(_soundFxHandle);
}

void FreescapeEngine::waitForSounds() {
	while (!_speaker->endOfStream())
		g_system->delayMillis(10);
}

bool FreescapeEngine::isPlayingSound() {
	return (!_speaker->endOfStream());
}

void FreescapeEngine::playSilence(int duration, bool sync) {
	_speaker->playQueue(Audio::PCSpeaker::kWaveFormSilence, 0, 1000 * 10 * duration);
	_mixer->stopHandle(_soundFxHandle);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundFxHandle, _speaker, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
}

void FreescapeEngine::queueSoundConst(double hzFreq, int duration) {
	_speaker->playQueue(Audio::PCSpeaker::kWaveFormSquare, hzFreq, 1000 * 10 * duration);
}
void FreescapeEngine::playSoundConst(double hzFreq, int duration, bool sync) {
	queueSoundConst(hzFreq, duration);
	_mixer->stopHandle(_soundFxHandle);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundFxHandle, _speaker, -1, Audio::Mixer::kMaxChannelVolume / 8, 0, DisposeAfterUse::NO);
}

void FreescapeEngine::playSoundSweepIncWL(double hzFreq1, double hzFreq2, double wlStepPerMS, int resolution, bool sync) {
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
		float hzFreq = (1193180.0 / inv1);
		_speaker->playQueue(Audio::PCSpeaker::kWaveFormSquare, hzFreq, 1000 * 10 * resolution);
		inv1 += wlStep;
	}
	_mixer->stopHandle(_soundFxHandle);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundFxHandle, _speaker, -1, Audio::Mixer::kMaxChannelVolume / 8, 0, DisposeAfterUse::NO);
}

void FreescapeEngine::playTeleporter(int totalIters, bool sync) {
	// Play FreeScape DOS teleporter-like effect, which is ascending arpeggio.
	// Length of effect is variable; provide total number of iterations.

	// The general pattern is two iterations upward, one iteration downward.
	// This means one "ascension cycle" lasts three iterations.
	// The result is a simulated echo (a better analogy would be a piano's
	// damper pedal) with gradual ascending frequency.

	// The frequency changes using the same wavelength-shift strategy that is
	// found in playSoundSweepIncWL.

	int i;
	double fBase = 1193180.0 / 244.607;
	double fInc = -600.0;
	int stepCycle = 1;

	// Loop over iterations
	for (i = 0; i < totalIters; i++) {
		float hzFreq = 1193180.0 / fBase;
		_speaker->playQueue(Audio::PCSpeaker::kWaveFormSquare, hzFreq, 1000 * 10 * 21);

		if (stepCycle <= 1) {
			// Ascending first two portions of cycle
			fBase += fInc;
			stepCycle++;
		} else {
			// Descending final portion of cycle
			fBase -= fInc;
			stepCycle = 0;
		}
	}
	_mixer->stopHandle(_soundFxHandle);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundFxHandle, _speaker, -1, Audio::Mixer::kMaxChannelVolume / 8, 0, DisposeAfterUse::NO);
}

void FreescapeEngine::loadSoundsFx(Common::SeekableReadStream *file, int offset, int number) {
	file->seek(offset);
	soundFx *sound = nullptr;
	_soundsFx[0] = sound;
	for (int i = 1; i < number + 1; i++) {
		sound = (soundFx *)malloc(sizeof(soundFx));
		int zero = file->readUint16BE();
		assert(zero == 0);
		int size = file->readUint16BE();
		int sampleRate = file->readUint16BE();
		debugC(1, kFreescapeDebugParser, "Loading sound: %d (size: %d, sample rate: %d)", i, size, sampleRate);
		byte *data = (byte *)malloc(size * sizeof(byte));
		file->read(data, size);
		sound->sampleRate = sampleRate;
		sound->size = size;
		sound->data = (byte *)data;
		_soundsFx[i] = sound;
	}
}

} // namespace Freescape
