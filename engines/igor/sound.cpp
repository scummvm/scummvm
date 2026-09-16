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
#include "audio/decoders/raw.h"
#include "audio/decoders/voc.h"

#include "igor/igor.h"

namespace Igor {

void IgorEngine::playMusic(int num) {
    debugC(9, kDebugEngine, "playMusic() %d", num);
	if (_game.flags & kFlagFloppy) {
		static const int cmf[] = { 0, 0, CMF_2_1, CMF_3, CMF_4, 0, 0, CMF_7_1, CMF_8, CMF_9, CMF_10, CMF_11, CMF_12 };
		assert(num < ARRAYSIZE(cmf) && cmf[num] != 0);
		_gameState.musicNum = num;
		_gameState.musicSequenceIndex = 1;
		// startMusic(cmf[num]);
	} else {
		// TODO: play CD track
	}
}

void IgorEngine::playSound(int num, int type) {
	debugC(9, kDebugEngine, "playSound() %d", num);
    // --num;
	int soundOffset = -1;
	Audio::Mixer::SoundType soundType;
	Audio::SoundHandle *soundHandle = 0;
	if (type == 1) {
		if (_mixer->isSoundHandleActive(_sfxHandle)) {
			return;
		}
		assert(num >= 0 && num < _soundOffsetsCount);
		soundOffset = _soundOffsets[num];
		soundType = Audio::Mixer::kSFXSoundType;
		soundHandle = &_sfxHandle;
	} else if (type == 0 && (_game.flags & kFlagTalkie) != 0 && num != kNoSpeechSound) {
		if (_mixer->isSoundHandleActive(_speechHandle)) {
			_mixer->stopHandle(_speechHandle);
		}
		num += 100;
		assert(num >= 0 && num < _soundOffsetsCount);
		soundOffset = _soundOffsets[num];
		soundType = Audio::Mixer::kSpeechSoundType;
		soundHandle = &_speechHandle;
	} else {
		return;
	}
	_sndFile.seek(soundOffset);
	Audio::AudioStream *stream = Audio::makeVOCStream(&_sndFile, Audio::FLAG_UNSIGNED);
	if (stream) {
		_mixer->playStream(soundType, soundHandle, stream);
	}
}

void IgorEngine::stopSound() {
	_mixer->stopHandle(_sfxHandle);
}

} // End of namespace Igor
