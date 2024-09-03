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

#include "audio/mixer.h"

#include "agi/agi.h"
#include "agi/sound_coco3.h"

namespace Agi {

// SoundGenCoCo3 plays Tandy Color Computer sounds.
//
// CoCo3 AGI sounds are a series of monotonic notes. They sound similar to
// PC speaker versions, but they use a different resource format. As with
// the Apple II, sound playback is a blocking operation.
//
// The sound resource is a stream of four-byte notes. The frequency is a table
// index. The volume is a boolean. The duration changed between interpreters;
// originally the units were 1/10 of a second, then 1/60.
//
// Common::PCSpeaker is used for sound generation. It produces significantly
// louder volumes than the other AGI sound generators, so I've lowered the
// mixer volume for consistency.
//
// Thanks to Guillaume Major for documenting the sound format in their
// conversion program, cc3snd.c.

static const uint16 cocoFrequencies[] = {
	 130,  138,  146,  155,  164,  174,  184,  195,  207,  220,  233,  246,
	 261,  277,  293,  311,  329,  349,  369,  391,  415,  440,  466,  493,
	 523,  554,  587,  622,  659,  698,  739,  783,  830,  880,  932,  987,
	1046, 1108, 1174, 1244, 1318, 1396, 1479, 1567, 1661, 1760, 1864, 1975,
	2093, 2217, 2349, 2489, 2637, 2793, 2959, 3135, 3322, 3520, 3729, 3951
};

#define COCO3_MIXER_VOLUME 20

SoundGenCoCo3::SoundGenCoCo3(AgiBase *vm, Audio::Mixer *pMixer) :
	_isPlaying(false),
	SoundGen(vm, pMixer) {

	_mixer->playStream(Audio::Mixer::kMusicSoundType, _soundHandle, this, -1, COCO3_MIXER_VOLUME, 0, DisposeAfterUse::NO, true);
}

SoundGenCoCo3::~SoundGenCoCo3() {
	_mixer->stopHandle(*_soundHandle);
}

void SoundGenCoCo3::play(int resnum) {
	Common::StackLock lock(_mutex);

	if (_vm->_game.sounds[resnum] == nullptr ||
		_vm->_game.sounds[resnum]->type() != AGI_SOUND_COCO3) {
		error("CoCo3 sound %d not loaded", resnum);
	}

	_speaker.stop();

	// KQ3  (Int. 2.023) stored the duration in 1/10 of a second.
	// LSL1 (Int, 2.072) stored the duration in 1/60 of a second.
	// Fan ports have been made using both interpreters, but our
	// detection table doesn't capture this. For now, treat KQ3
	// as the early interpreter all others as the later one.
	// TODO: create detection heuristic
	bool isEarlySound = (_vm->getGameID() == GID_KQ3);

	// parse and enqueue all notes
	AgiSound *sound = _vm->_game.sounds[resnum];
	byte *data = sound->getData();
	uint32 dataLength = sound->getLength();
	for (uint32 i = 0; i + 4 < dataLength; i += 4) {
		// the third byte is apparently unused, and always zero
		byte freqIndex = data[i];
		byte volume = data[i + 1];
		byte duration = data[i + 3];
		if (freqIndex == 0xff) {
			break;
		}

		// get duration in ticks (1/60 of a second)
		uint32 ticks = duration;
		if (isEarlySound) {
			ticks *= 6;
		}

		// convert ticks to microseconds for PCSpeaker
		uint32 duration_usec = ticks * (1000000.0f / 60.0f);

		// play if volume is non-zero (it's always 0x3f or zero)
		if (volume != 0 && freqIndex < ARRAYSIZE(cocoFrequencies)) {
			int freq = cocoFrequencies[freqIndex];
			_speaker.playQueue(Audio::PCSpeaker::kWaveFormSquare, freq, duration_usec);
		} else {
			_speaker.playQueue(Audio::PCSpeaker::kWaveFormSilence, 0, duration_usec);
		}
	}

	_isPlaying = true;
}

void SoundGenCoCo3::stop() {
	Common::StackLock lock(_mutex);

	_speaker.stop();
	_isPlaying = false;
}

int SoundGenCoCo3::readBuffer(int16 *buffer, const int numSamples) {
	Common::StackLock lock(_mutex);

	// if not playing then there are no samples
	if (!_isPlaying) {
		return 0;
	}

	// fill the buffer with PCSpeaker samples
	int result = _speaker.readBuffer(buffer, numSamples);

	// if PCSpeaker is no longer playing then sound is finished
	if (!_speaker.isPlaying()) {
		_isPlaying = false;
		_vm->_sound->soundIsFinished();
	}

	return result;
}

} // End of namespace Agi
